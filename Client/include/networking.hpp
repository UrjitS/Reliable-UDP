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

enum {
    ACK = 0x01,
    SYN = 0x02,
    SYN_ACK = 0x03,
};

struct header_field {
    uint32_t sequence_number;
    uint32_t ack_number;
    uint8_t flags;
    uint16_t data_length;
    std::string data;
};

bool validate_ip_address(const std::string& ip_address);

#endif
