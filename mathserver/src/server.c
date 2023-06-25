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

#include "configs.h"
#include "tools.h"
#include "strategies.h"


struct argument_flags arg_flags;

int main (int argc, char **argv)
{
    const char * const ident = "math server";
    openlog (ident, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
    syslog (LOG_USER | LOG_INFO, "%s", "Math Server Initiated");

    signal(SIGCHLD, SIG_IGN);

    parshe_arguments(MODE_AS_SERVER, &arg_flags, argc, argv);

    if(arg_flags.DAEMON_FLAG){
        daemonize();
    }
    else{ // default mode to operate the server
        printf("FORK STRATEGY SELECTED\n");
        handle_clients_for_basic_with_fork(&arg_flags);
        printf("EXITING SERVER\n");
    }
    exit(EXIT_SUCCESS);
} // main done