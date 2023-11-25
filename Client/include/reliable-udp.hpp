#ifndef CLIENT_RELIABLE_UDP_HPP
#define CLIENT_RELIABLE_UDP_HPP

#include <cstdint>

#define MAX_WINDOW 4
#define HEADER_LENGTH 13
#define MAX_PACKET_LENGTH 1010

int send_packet(struct networking_options& networkingOptions);
uint32_t receive_acknowledgements(struct networking_options& networkingOptions, int timeout_seconds);

#endif
