#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <signal.h> 
#include <bits/types.h>
#include <fcntl.h>

#include "tools.h"
#include "configs.h"

void error (char *msg)
{
    perror (msg);
    exit (1);
}

// trim: leading and trailing whitespace of string
void trim (char *dest, char *src)
{
    if (!src || !dest)
       return;

    int len = strlen (src);

    if (!len) {
        *dest = '\0';
        return;
    }
    char *ptr = src + len - 1;

    // remove trailing whitespace
    while (ptr > src) {
        if (!isspace (*ptr))
            break;
        ptr--;
    }

    ptr++;

    char *q;
    // remove leading whitespace
    for (q = src; (q < ptr && isspace (*q)); q++)
        ;

    while (q < ptr)
        *dest++ = *q++;

    *dest = '\0';
}

int get_input (struct message *msg_to_send)
{
    int option=0;
    char inbuf[COMMAND_LENGTH+1];
    char exec_selection[10];
    while (1) {
        printf ("Enter a command for the server [0 to Quit]: ");
        if (fgets (inbuf, sizeof (inbuf),  stdin) == NULL) {
            error ("fgets");
            option = INVALID_OPERATION;
        }
        else{
            strcpy(msg_to_send->requested_operation,inbuf);
            sscanf (inbuf, "%s", exec_selection);
            if(strcmp(exec_selection,MAT_INV_PAR_COMMAND)==0){
                printf("INVERSE Matrix operation selected\n");
                option = GET_MATRIX_INVERSE;
            }
            else if(strcmp(exec_selection,K_MEANS_PAR_COMMAND)==0){
                printf("KMEANS operation selected\n");
                option = GET_KMEANS;
            }
            else if(inbuf[0]=='0'){
                option = QUIT;
            }
            else {
                printf("INVALID operation selected\n");
                option = INVALID_OPERATION;
            }
            msg_to_send->requested_operation_type = htonl(option);
            msg_to_send->message_id = htonl(option);
        }
        return option;
    }
}

void parshe_arguments(int use_mode, struct argument_flags *arg_flag,int total_arguments, char **argument_variables)
{
  arg_flag->DAEMON_FLAG = 0;
  arg_flag->OPERATION_MODE = MODE_AS_SERVER_FORK;
  strcpy(arg_flag->IP,DEFAULT_SERVER_IP);
  arg_flag->PORT = DEFAULT_SERVER_PORT; 
  arg_flag->DEBUG = DEFAULT_DEBUG_MODE;

  if(use_mode==MODE_AS_KMEANS_PAR){
    strcpy(arg_flag->INPUT_FILE_NAME , DEFAULT_KMEANS_INPUT_FILE);
    strcpy(arg_flag->OUTPUT_FILE_NAME, DEFAULT_KMEANS_OUTPUT_FILE);
    arg_flag->NUMBER_OF_CLUSTERS = DEFAULT_KMEANS_CLUSTERS;
  }
  else if(use_mode==MODE_AS_MATINV_PAR){
      arg_flag->MATRIX_PROBLEM_SIZE = DEFAULT_MATRIX_SIZE;
      strcpy(arg_flag->MATRIX_INV_INIT_TYPE, DEFAULT_MATRIX_INIT_MODE);
      strcpy(arg_flag->OUTPUT_FILE_NAME, DEFAULT_MATINV_OUTPUT_FILE);
      arg_flag->MAX_RANDOM_NUMBER = DEFAULT_RANDOM_NUMBER;
  }
  char *current_executable_file_name;
  current_executable_file_name = *argument_variables;
  while (++argument_variables, --total_arguments > 0){
    if (**argument_variables == '-'){
      switch (*++*argument_variables)
      {
        case 'd':
          if(use_mode==MODE_AS_SERVER){
          arg_flag->DAEMON_FLAG = 1;
          }
          else{
              // pass for now for client
          }
          break;
        case 'D':
          printf("\nDefault:  n         = %d ", arg_flag->MATRIX_PROBLEM_SIZE);
          printf("\n          Init      = %s", arg_flag->MATRIX_INV_INIT_TYPE);
          printf("\n          max rand N= %d ", arg_flag->MAX_RANDOM_NUMBER);
          printf("\n          P         = %d \n\n", arg_flag->DEBUG);
          exit(0);
          break;
        case 'f':
          --total_arguments;
          if(use_mode==MODE_AS_KMEANS_PAR){
            strcpy(arg_flag->INPUT_FILE_NAME, *++argument_variables);
          }
          else{// Will compare for other modes if required
            ++argument_variables;
          }
          break;
        case 'i':
          if(use_mode==MODE_AS_SERVER_FORK){
              // pass for now for server
            ++argument_variables; // just ignore the associated value for now
          }
          else{
              // checking for -ip without refactoring the function.
              if (*++*argument_variables == 'p')
              {
                  --total_arguments;
                  strcpy(arg_flag->IP,*++argument_variables);
                  printf("HOST:%s\n",arg_flag->IP);
              }
          }
          break;
        case 'I':
          --total_arguments;
          if(use_mode==MODE_AS_MATINV_PAR){
            strcpy(arg_flag->MATRIX_INV_INIT_TYPE, *++argument_variables);
          }
          else{ // passing for now . will use if required by other usage mode
            ++argument_variables; // just ignore the associated value for now
          }
          break;
        case 'k':
          --total_arguments;
          if(use_mode==MODE_AS_KMEANS_PAR){
            int temp_num_cluster = atoi(*++argument_variables);
            if (temp_num_cluster > MAX_CLUSTERS)
            {
              printf("k cannot be more than: %u", MAX_CLUSTERS);
              printf("setting number of clusters to %u",MAX_CLUSTERS);
              temp_num_cluster = MAX_CLUSTERS;
            }
            arg_flag->NUMBER_OF_CLUSTERS = temp_num_cluster;
          }
          else{ // passing for now . will use if required by other usage mode
            ++argument_variables; // just ignore the associated value for now
          }
          break;
        case 'n':
          --total_arguments;
          if(use_mode==MODE_AS_MATINV_PAR){
            arg_flag->MATRIX_PROBLEM_SIZE = atoi(*++argument_variables);
          }
          else{ // passing for now . will use if required by other usage mode
            ++argument_variables;// just ignore the associated value for now
          }
          break;
        case 'm':
          --total_arguments;
          if(use_mode==MODE_AS_MATINV_PAR){
            arg_flag->MAX_RANDOM_NUMBER = atoi(*++argument_variables);
          }
          else{// passing for now . will use if required by other usage mode
            ++argument_variables; // just ignore the associated value for now
          }
          break;
        case 'o':
          --total_arguments;
          if(use_mode==MODE_AS_KMEANS_PAR || use_mode==MODE_AS_MATINV_PAR){
            strcpy(arg_flag->OUTPUT_FILE_NAME, *++argument_variables);
          }
          else{ // passing for now . will use if required by other usage mode
            ++argument_variables; // just ignore the associated value for now
          } 
          break;
        case 'p':
          --total_arguments;
          if(use_mode==MODE_AS_CLIENT || use_mode==MODE_AS_SERVER){
            arg_flag->PORT = atoi(*++argument_variables);
            printf("PORT:%d\n",arg_flag->PORT);
          }
          else{ // passing for now . will use if required by other usage mode
            ++argument_variables; // just ignore the associated value for now
          }
          break;
        case 'P':
          --total_arguments;
          if(use_mode==MODE_AS_MATINV_PAR){
            arg_flag->DEBUG = atoi(*++argument_variables);
          }
          else{ // passing for now . will use if required by other usage mode
            ++argument_variables; // just ignore the associated value for now
          }
          break;
        case 's':
          --total_arguments;
          if(use_mode==MODE_AS_SERVER){
              char *in_strat = *++argument_variables;
              printf("Strategy [%s]\n", in_strat);
              if (strcmp(CMD_SERVER_FORK_MODE, in_strat) == 0)
              {
                arg_flag->OPERATION_MODE = MODE_AS_SERVER_FORK;
                printf("FORK matched\n");
              }
              else
              {
                printf("Setting default strategy to fork\n");
                arg_flag->OPERATION_MODE = MODE_AS_SERVER_FORK;
              }
          }
          else {
              printf("Invalid Server Strategy\n");
              exit(3);    // return 3
          }
          break;
        case 'u':
          if(use_mode==MODE_AS_MATINV_PAR){
            printf("\nUsage: matinv [-n problemsize]\n");
            printf("           [-D] show default values \n");
            printf("           [-h] help \n");
            printf("           [-I] init_type fast/rand \n");
            printf("           [-m] max rand num max random no \n");
            printf("           [-P] print_switch 0/1 \n");
            exit(0);
          }
          break;
        case 'h':
          if(use_mode==MODE_AS_SERVER_FORK){
              printf("\nUsage: \nserver [-p port] listen on port (default: 9999)\n");
              printf("       [-s fork|muxbasic|muxscale] specify strategy (default: fork)\n");
              printf("       [-d] run as daemon\n");
              printf("       [-h] help usage\n");
          }
          else if(use_mode==MODE_AS_CLIENT){
              printf("\nUsage: client [-p port] port to connect with the server (default: 9999)\n");
              printf("           [-ip] Server IPv4 adress to connect (default 0.0.0.0 or 127.0.0.1)\n");
              printf("           [-h] help usage\n");
          }
          else if(use_mode==MODE_AS_KMEANS_PAR){
            printf("\nUsage: kmeans [-k] number of clusters\n");
            printf("              [-f] input file name\n");
            printf("              [-o] output file name\n");
            printf("           [-h] help \n");
          }
          else if(use_mode==MODE_AS_MATINV_PAR){
            printf("\nHELP: try matinv -u \n\n");
            exit(0);
          }
          else{
              printf("Invalid User mode\n");
          }
          exit(3); //return 3
          // break;
        default:
          printf("[ TRY WITH %s -h]\n\n", current_executable_file_name);
          printf("%s: options ignored: -%s\n", current_executable_file_name, *argument_variables);
          exit(3); //return 3
          // break;
      }
    }
  }
  // printf("DEBUG %d\n",arg_flag->DEBUG);
}

char *get_unique_file_path(char *current_program_name, unsigned long long client_id)
{
  char *generated_path = malloc(100 * sizeof(char));
  int index = 1;

  snprintf(generated_path, 100, "../computed_results/%s_client%lld_soln%d.txt", current_program_name, client_id, index);

  while (access(generated_path, F_OK) == 0)
  {
    index++;
    snprintf(generated_path, 100, "../computed_results/%s_client%lld_soln%d.txt", current_program_name, client_id, index);
  }

  return generated_path;
}

// code provided in lecture 10 in our course
void daemonize()
{
  pid_t child_pid = fork();
  if(child_pid==0){
      pid_t sid = setsid();
      printf("Daemonized Process ID %d\n",sid);
      /*setsid() creates a new session if the calling process is not a
       process group leader.  The calling process is the leader of the
       new session (i.e., its session ID is made the same as its process
       ID).  The calling process also becomes the process group leader
       of a new process group in the session (i.e., its process group ID
       is made the same as its process ID).
      */
  }
  else exit(EXIT_SUCCESS); // close parent process/ corrupted process
}

void extract_args(char *arguments_as_string, int number_of_expected_arguments, char **destination_args)
{
  destination_args[0] = strtok(arguments_as_string, " ");
  for (int i = 1; i < number_of_expected_arguments; i++)
  {
    destination_args[i] = strtok(NULL, " ");
  }
  if(destination_args[number_of_expected_arguments-1][strlen(destination_args[number_of_expected_arguments-1])-1]=='\n'){
    destination_args[number_of_expected_arguments-1][strlen(destination_args[number_of_expected_arguments-1])-1] = '\0';
  }
}

int count_white_spaces(char *target_str)
{
  int number_of_spaces = 0;
  for (int i = 0; i < strlen(target_str); i++)
  {
    if (target_str[i] == ' ')
    {
      number_of_spaces += 1;
    }
  }
  return number_of_spaces;
}