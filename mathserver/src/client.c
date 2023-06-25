#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include <netdb.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "configs.h"
#include "tools.h"


struct argument_flags arg_flags;
struct message message;

int main (int argc, char **argv)
{
    signal(SIGCHLD, SIG_IGN);

    parshe_arguments(MODE_AS_CLIENT, &arg_flags, argc, argv);

    struct addrinfo hints;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result;
    int s; 
    char server_port[10];
    printf("SERVER %s PORT %d\n",arg_flags.IP, arg_flags.PORT);
    sprintf(server_port,"%d",arg_flags.PORT);
    if ((s = getaddrinfo (arg_flags.IP, server_port, &hints, &result)) != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        exit (EXIT_FAILURE);
    }

    /* Scan through the list of address structures returned by 
       getaddrinfo. Stop when the the socket and connect calls are successful. */

    int sock_fd;
    struct addrinfo *rptr;
    for (rptr = result; rptr != NULL; rptr = rptr -> ai_next) {
        sock_fd = socket (rptr -> ai_family, rptr -> ai_socktype,
                       rptr -> ai_protocol);
        if (sock_fd == -1)
            continue;

        if (connect (sock_fd, rptr -> ai_addr, rptr -> ai_addrlen) == -1) {
            if (close (sock_fd) == -1)
                error ("close");
            continue;
        }
        break;
    }

    if (rptr == NULL) {               // Not successful with any address
        fprintf(stderr, "Not able to connect\n");
        exit (EXIT_FAILURE);
    }

    freeaddrinfo (result);

    int option;

    while(1)
    {
        option = get_input (&message);
        if (option == QUIT)
            break;

        // send request to server MSG_NOSIGNAL
        if (send (sock_fd, &message, sizeof (struct message), MSG_NOSIGNAL) == -1)
            error ("send");

        // receive response from server
        int bytes = recv (sock_fd, &message, sizeof (struct message), 0);
        if (bytes == -1)
            error ("recv");
        if (bytes == 0)
        {
            printf("Connection closed\n");
            exit(EXIT_SUCCESS);
        }
        else if (bytes == -1)
        {
            perror("Could not recieve file data");
            exit(EXIT_FAILURE);
        }
        // process server response 
        switch (ntohl(message.response_status)) {
            case COMPUTED_MATRIX_INVERSE:
                printf("DEBUG_[%s]\n",message.response_file_name);
                break;
            case COMPUTED_KMEANS:
                printf("DEBUG_[%s]\n",message.response_file_name);
                break;
            case ERROR_IN_INPUT:
                printf("DEBUG_[%s]\n",message.response_file_name);
                break;
            default:
                printf ("\nUnrecongnized message from server\n\n");
                break;
        }
        long response_file_size = ntohl(message.response_file_size);

        printf("Received the solution: %s\n", message.response_file_name);
        char prefix[] = "received_";
        char file_path[1024];
        strcpy(file_path,prefix);
        strcat(file_path,message.response_file_name);

        char FILE_BUF[1024];

        long long recieved_data = 0;
        while (recieved_data < response_file_size)
        {
            int msize = recv(sock_fd, &FILE_BUF, sizeof(FILE_BUF), 0);
            if (msize == 0)
            {
                printf("Connection closed\n");
                exit(EXIT_SUCCESS);
            }
            else if (msize == -1)
            {
                perror("Could not recieve file contents");
                exit(EXIT_FAILURE);
            }
            
            recieved_data += msize;
        } 
    }
    exit (EXIT_SUCCESS);
}

