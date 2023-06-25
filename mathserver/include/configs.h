#ifndef CONFIGS_H_
#define CONFIGS_H_

#define MAX_EPOLL_EVENTS            100

#define COMMAND_LENGTH              1023
#define MAX_FILE_NAME_LENGTH        1023
#define RLIM_INFINITY               0xffffffffffffffffuLL

#define DEFAULT_SERVER_PORT         9999
#define DEFAULT_SERVER_IP           "0.0.0.0"
#define MAT_INV_PAR_COMMAND         "matinvpar"
#define K_MEANS_PAR_COMMAND         "kmeanspar"
#define DEFAULT_KMEANS_INPUT_FILE   "kmeans-data.txt"
#define DEFAULT_KMEANS_OUTPUT_FILE  "kmeans-results.txt"
#define CMD_SERVER_FORK_MODE        "fork"
#define CMD_SERVER_MUXBASIC_MODE    "muxbasic"
#define CMD_SERVER_MUXSCALE_MODE    "muxscale"   

#define MODE_AS_MATINV_PAR          -7
#define MODE_AS_KMEANS_PAR          -6
#define MODE_AS_SERVER_MUXSCALE     -5
#define MODE_AS_SERVER_MUXBASIC     -4
#define MODE_AS_SERVER_FORK         -3
#define MODE_AS_CLIENT              -2
#define MODE_AS_SERVER              -1
#define QUIT                        0
#define GET_MATRIX_INVERSE          1
#define GET_KMEANS                  2
#define INVALID_OPERATION           3
#define COMPUTED_MATRIX_INVERSE     4
#define COMPUTED_KMEANS             5
#define COMPUTED_FILE               6
#define ERROR_IN_INPUT              7

#define BACKLOG                     1000000
#define NUM_FDS                     1000000

#define MAX_POINTS                  4096
#define MAX_CLUSTERS                32
#define DEFAULT_KMEANS_CLUSTERS     9
#define MAX_NUM_THREADS             4

#define MAX_MATRIX_SIZE             4096

#define DEFAULT_MATRIX_SIZE         5
#define DEFAULT_MATRIX_INIT_MODE    "fast"
#define DEFAULT_RANDOM_NUMBER       15
#define DEFAULT_DEBUG_MODE          1
#define DEFAULT_MATINV_OUTPUT_FILE  "matrix_result.txt"

#define FD_CLIENT_DIFFERENCE        5

#endif