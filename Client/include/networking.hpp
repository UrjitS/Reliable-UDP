#ifndef FSM_NETWORK_SHARE_CLIENT_NETWORKING_HPP
#define FSM_NETWORK_SHARE_CLIENT_NETWORKING_HPP


#include <netinet/in.h>
#include <string>
#include <ctime>

#define DEFAULT_PORT 5050

/**
 * @brief Networking options struct
 */
struct networking_options {
    std::string program_name;
    std::string message;
    int socket_fd;
    int ip_family;
    struct sockaddr_in ipv4_addr;
    struct sockaddr_in6 ipv6_addr;
    struct header_field * header;
    std::string device_ip_address;
    std::string receiver_ip_address;
    in_port_t receiver_port;
    bool terminal_input;
    time_t time_started;
    FILE * stats_file;
};

/**
 * @brief Flag Values for header field
 */
enum {
    ACK = 0x01,
    DATA = 0x02,
};

/**
 * @brief Header field struct
 */
struct header_field {
    uint32_t sequence_number;
    uint32_t ack_number;
    uint8_t flags;
    uint16_t data_length;
    std::string data;
    uint64_t sent_counter;
    time_t time_ack;
    time_t time_sent;
};

/**
 * @brief Validates given IP Address
 * @param ip_address IP Address to validate
 * @return True if valid, false otherwise
 */
bool validate_ip_address(const std::string& ip_address);
/**
 * @brief Validates given Port Number
 * @param networkingOptions Networking options struct
 * @return Socket file descriptor otherwise false
 */
int create_udp_socket(struct networking_options& networkingOptions);
/**
 * @brief Gets the device IP Address
 * @param networkingOptions Networking options struct
 * @return True if successful, false otherwise
 */
bool bind_udp_socket(struct networking_options& networkingOptions);

#endif
