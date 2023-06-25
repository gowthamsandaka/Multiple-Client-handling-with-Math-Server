#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/epoll.h>


#include "tools.h"
#include "configs.h"
#include "strategies.h"

void handle_clients_for_basic_with_fork(struct argument_flags *server_args){
    printf("Selected Strategy Fork . Which is implemented with fork");
    struct message recv_message, send_message;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    // define server address and port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_args->PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    listen(sfd, 1);

    printf("Listening for clients...\n");

    for (unsigned long long i = 1;; i++)
    {
        int conn = accept(sfd, NULL, NULL);

        int pid = fork();

        if (pid == -1)
        {
            printf("Could not create new process\n");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            while(handle_client_request_response(&conn, &recv_message, &send_message,NULL,NULL));
            close(conn);
            exit(EXIT_SUCCESS);
        }
    }
}

bool handle_client_request_response(int *client_sofcket_fd, struct message *recv_message, struct message *send_message, struct epoll_event *ev,int *efd){
    memset (recv_message, '\0', sizeof (struct message));
    // printf("waiting to receive data from client\n");
    ssize_t numbytes = recv (*client_sofcket_fd, recv_message, sizeof (struct message), 0);

    if (numbytes == -1){
        error ("recv");
        return false;
    }
    else if (numbytes == 0) {
        // connection closed by client
        fprintf (stderr, "Socket %d closed by client\n", (*client_sofcket_fd)-FD_CLIENT_DIFFERENCE);
        if (close (*client_sofcket_fd) == -1)
            error ("close");
        return false;
    }
    else 
    {
        // data from client
        int in_arg_count = count_white_spaces(recv_message->requested_operation) + 1;
        // printf("In_arg_count: %d\n", in_arg_count);
        int total_arg_count = in_arg_count + 3;
        char *args[total_arg_count];
        extract_args(recv_message->requested_operation, in_arg_count, args);
        
        char *out_filepath = NULL;
        char *path;
        printf("Received[%s]\n",recv_message->requested_operation);

        switch (ntohl(recv_message->requested_operation_type)) {
            case GET_MATRIX_INVERSE:
                printf("matrix inverse calculation matched\n");
                send_message->message_id = htonl(COMPUTED_FILE) ;
                send_message->response_status = htonl(COMPUTED_MATRIX_INVERSE);
                send_message->response_file_size = 0;
                path = "./matinvpar";
                out_filepath = get_unique_file_path("matinvpar", (*client_sofcket_fd)-FD_CLIENT_DIFFERENCE);
                break;
            case GET_KMEANS:
                printf("kmeans calculation matched\n");
                send_message->message_id = htonl(COMPUTED_FILE) ;
                send_message->response_status = htonl(COMPUTED_KMEANS);
                send_message->response_file_size = 0;
                path = "./kmeanspar";
                out_filepath = get_unique_file_path("kmeanspar", (*client_sofcket_fd)-FD_CLIENT_DIFFERENCE);
                break;
            case INVALID_OPERATION:
                printf("Invalid operation requested by client\n");
                send_message->message_id = htonl(INVALID_OPERATION) ;
                send_message->response_status = htonl(INVALID_OPERATION);
                send_message->response_file_size = 0;
                strcpy(send_message->response_file_name , "INVALID OPERATION REQUESTED");
                break;
            default:
                // do nothing for now
                printf("Unknown value\n");
                send_message->message_id = htonl(ERROR_IN_INPUT) ;
                send_message->response_status = htonl(INVALID_OPERATION);
                strcpy(send_message->response_file_name , "INVALID OPERATION REQUESTED");
                break;
        }

        args[total_arg_count - 3] = "-o";
        args[total_arg_count - 2] = out_filepath;
        args[total_arg_count - 1] = NULL;
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Could not create new process");
            free(out_filepath);
            return false;
        }
        if (pid == 0)
        {
            execv(path, args);
        }

        int status;
        waitpid(pid, &status, 0);

        FILE *file_fd = fopen(out_filepath, "rwx");
        if (file_fd == NULL)
        {
            perror("Could not open result file");
            send_message->response_file_size = 0;
            fclose(file_fd);
            free(out_filepath);
            return false;
        }

        struct stat file_stat;
        if (fstat(fileno(file_fd), &file_stat) < 0)
        {
            perror("Could not get file stats");
            fclose(file_fd);
            free(out_filepath);
            return false;
        }

        char *out_filename = out_filepath + strlen("../computed_results/");

        send_message->response_file_size = htonl(file_stat.st_size);
        send_message->response_file_name[0] = '\0';
        strncpy(send_message->response_file_name, out_filename, strlen(out_filename) + 1);
        // printf("File size: %ld\n", file_stat.st_size);

        printf("Sending solution: %s\n", out_filename);
        // send filedata
        send(*client_sofcket_fd, send_message , sizeof (struct message),0);

        char FILE_BUF[1024];
        for (;;)
        {
            int bytes_read = read(fileno(file_fd), &FILE_BUF, sizeof(FILE_BUF));
            // printf("bytes_read: %d\n", bytes_read);
            if (bytes_read == 0)
            {
                break;
            }
            if (bytes_read < 0)
            {
                send(*client_sofcket_fd, "\0", sizeof("\0"), 0);
                fclose(file_fd);
                free(out_filepath);
                return false;
                // close(*client_sofcket_fd);
                // exit(EXIT_FAILURE);
            }

            // we use a loop because all of the data may not be written in a single write call.
            char *p = FILE_BUF;
            while (bytes_read > 0)
            {
                // write() is equivalent to send() with a flag of 0
                int bytes_written = write(*client_sofcket_fd, p, bytes_read);
                if (bytes_written <= 0)
                {
                    perror("unable to send data");
                    fclose(file_fd);
                    free(out_filepath);
                    return false;
                }
                bytes_read -= bytes_written;
                p += bytes_written;
            }
        }
        free(out_filepath);
        fclose(file_fd);
    }
    return true;
}