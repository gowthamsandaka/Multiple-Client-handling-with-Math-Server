#pragma once
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#include "configs.h"

struct message {
    int32_t message_id;
    char requested_operation [COMMAND_LENGTH + 1];
    int32_t requested_operation_type;
    int32_t response_status;
    long response_file_size;
    char response_file_name[MAX_FILE_NAME_LENGTH+1];
};

struct argument_flags{
    char IP[1024];                  // will be used by server.c and client.c
    bool DAEMON_FLAG;               // will be used by server.c
    int16_t PORT;                   // will be used by both server.c and client.c
    int OPERATION_MODE;             // will select which module has used in argument parsher
    char INPUT_FILE_NAME[1024];     // will be used by kmeans_par.c
    char OUTPUT_FILE_NAME[1024];    // will be used by kmeans_par.c and matinv_par.c
    int NUMBER_OF_CLUSTERS;         // will be used by kmeans_par.c
    int MATRIX_PROBLEM_SIZE;        // will be used by matinv_par.c
    char MATRIX_INV_INIT_TYPE[32];  // will be used by matinv_par.c
    int MAX_RANDOM_NUMBER;        // will be used by matinv_par.c
    bool DEBUG;                     // will be used by matinv_par.c , kmeans_par.c , client.c and server.c
};

typedef __rlim_t rlim_t;

struct rlimit
{
    /* The current (soft) limit.  */
    rlim_t rlim_cur;
    /* The hard limit.  */
    rlim_t rlim_max;
};

void error (char *msg);

void trim (char *dest, char *src);

int get_input (struct message *msg_to_send);
char *get_unique_file_path(char *current_program_name, unsigned long long client_id);

void daemonize(void);
int count_white_spaces(char *target_str);

void extract_args(char *arguments_as_string, int number_of_expected_arguments, char **destination_args);
void parshe_arguments(int use_mode, struct argument_flags *arg_flag,int total_arguments, char **argument_variables);
