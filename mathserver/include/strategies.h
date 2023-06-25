#ifndef STRATEGIES_H_
#define STRATEGIES_H_
#include <stdbool.h>
#include <sys/epoll.h>

#include "configs.h"
#include "tools.h"

void handle_clients_for_basic_with_fork(struct argument_flags *server_args);
bool handle_client_request_response(int *client_sofcket_fd, struct message *recv_message, struct message *send_message, struct epoll_event *ev,int *efd);

#endif