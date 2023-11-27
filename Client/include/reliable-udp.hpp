#ifndef CLIENT_RELIABLE_UDP_HPP
#define CLIENT_RELIABLE_UDP_HPP

#include <cstdint>

#define WINDOW_SIZE 4
#define HEADER_LENGTH 13
#define MAX_PACKET_LENGTH 1010
#define RETRANSMISSION_COUNT 30

/**
 * @brief Send a packet to the receiver
 * @param networkingOptions Networking options struct
 * @return 1 if window size exceeded, -1 if send failed, 0 otherwise
 */
int send_packet(struct networking_options& networkingOptions);
/**
 * @brief Send a packet to the receiver
 * @param networkingOptions Networking options struct
 * @param timeout_seconds Microseconds to wait for select
 * @return Acknowledgement number if successful, 0 otherwise
 */
uint32_t receive_acknowledgements(struct networking_options& networkingOptions, int timeout_seconds);

#endif
