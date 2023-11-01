#ifndef FSM_NETWORK_SHARE_CLIENT_NETWORKING_HPP
#define FSM_NETWORK_SHARE_CLIENT_NETWORKING_HPP

#include <netinet/in.h>
#include <string>


struct networking_options {
    int argc;
    char ** argv;
    std::string message;
    int socket_fd;
    std::string ip_address;
    int ip_family;
    in_port_t port_number;
};


#endif //FSM_NETWORK_SHARE_CLIENT_NETWORKING_HPP
