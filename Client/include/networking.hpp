#ifndef FSM_NETWORK_SHARE_CLIENT_NETWORKING_HPP
#define FSM_NETWORK_SHARE_CLIENT_NETWORKING_HPP


#include <netinet/in.h>
#include <string>

#define DEFAULT_PORT 5050

struct networking_options {
    std::string program_name;
    std::string message;
    int socket_fd;
    int ip_family;
    struct sockaddr_storage * receiver_struct;
    std::string device_ip_address;
    std::string receiver_ip_address;
    in_port_t receiver_port;
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
int create_udp_socket(struct networking_options& networkingOptions);
bool bind_udp_socket(struct networking_options& networkingOptions);

#endif
