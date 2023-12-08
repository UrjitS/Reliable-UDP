#include "reliable-udp.hpp"
#include "networking.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <cstring>
#include <mutex>
#include <iostream>
#include <sys/time.h>

/**
 * @brief Mutex to lock the sent packets vector
 */
std::mutex modifying_global_variables;
/**
 * @brief Vector containing all the sent packets
 */
std::vector<header_field> sent_packets = std::vector<header_field>();
/**
 * @brief Count of the number of packets in the window
 */
int window_size = 0;

/**
 * @brief Pack the header into a string
 * @param header Header struct
 * @return String containing the header
 */
std::string pack_header(struct header_field* header);
/**
 * @brief Increment the sent counter for each packet in the sent packets vector
 * @return void
 */
void increment_sent_counter();
/**
 * @brief Send a packet to the receiver
 * @param networkingOptions Networking options struct
 * @param packet Packet to send
 * @return Number of bytes sent
 */
ssize_t send_packet_over(struct networking_options& networkingOptions, const std::string& packet);
/**
 * @brief Iterate over the sent packets and check if any need to be retransmitted
 * @param networkingOptions Networking options struct
 * @return void
 */
void check_need_for_retransmission(struct networking_options& networkingOptions);
/**
 * @brief Decode the string into a header struct
 * @param packet_raw String containing the packet
 * @return Acknowledgement number
 */
uint32_t decode_string(char * packet_raw);
/**
 * @brief Write the data to the file
 * @param stats_file File to write to
 * @param sequence_number Sequence number of the packet
 * @param time_taken Time taken to receive acknowledgement
 * @return void
 */
void write_data_to_file(FILE * stats_file, uint32_t sequence_number, time_t time_taken);
/**
 * @brief Remove the packet from the list of sent packets
 * @param networkingOptions Networking options struct
 * @param ack_number Acknowledgement number
 * @return void
 */
void remove_packet_from_sent_packets(struct networking_options& networkingOptions, uint32_t ack_number);

std::string pack_header(struct header_field * header) {
    header->data_length = header->data.length() + 3;
    uint32_t seq_number  = htonl(header->sequence_number);
    uint32_t ack_number  = htonl(header->ack_number);
    uint8_t flags        = htons(header->flags);
    uint16_t data_length = htons(header->data_length);

    std::string packet;
    packet.append(reinterpret_cast<const char *>(&seq_number), sizeof(seq_number));
    packet.append(reinterpret_cast<const char *>(&ack_number), sizeof(ack_number));
    packet.append(reinterpret_cast<const char *>(&flags), sizeof(flags));
    packet.append(reinterpret_cast<const char *>(&data_length), sizeof(data_length));
    packet.append(header->data);
    packet.append("\0", 1);        // Append a null character with length 1
    packet.append("\x03\x03", 2);  // Append two ETX characters

    return packet;
}

void increment_sent_counter() {
    // Iterate over the elements up to a maximum of the first five
    for (size_t i = 0; i < std::min(sent_packets.size(), static_cast<size_t>((WINDOW_SIZE + 1))); ++i) {
        auto& sent_packet = sent_packets[i];
        sent_packet.sent_counter++;

    }
}

ssize_t send_packet_over(struct networking_options& networkingOptions, const std::string& packet) {
    ssize_t ret_status;

    if (networkingOptions.ip_family == AF_INET) {
        ret_status = sendto(networkingOptions.socket_fd, packet.c_str(), packet.length(), 0, (struct sockaddr *) &networkingOptions.ipv4_addr, sizeof(networkingOptions.ipv4_addr));
    } else {
        ret_status = sendto(networkingOptions.socket_fd, packet.c_str(), packet.length(), 0, (struct sockaddr *) &networkingOptions.ipv6_addr, sizeof(networkingOptions.ipv6_addr));
    }

    return ret_status;
}

int send_packet(struct networking_options& networkingOptions) {
    ssize_t ret_status;

    // Make sure the window size is not exceeded
    if (window_size > WINDOW_SIZE) {
        return 1;
    }

    std::string packet = pack_header(networkingOptions.header);

    modifying_global_variables.lock();

    // Add to sent packets
    sent_packets.push_back(*networkingOptions.header);

    // Send the packet
    ret_status = send_packet_over(networkingOptions, packet);

    if (ret_status < 0) {
        perror("Send Failed");
        return -1;
    }

    window_size++;
    increment_sent_counter();

    modifying_global_variables.unlock();

    return 0;
}


uint32_t decode_string(char * packet_raw) {

    // Extract fields from the packet
    uint32_t seq_number;
    uint32_t ack_number;

    std::memcpy(&seq_number, &packet_raw[0], sizeof(seq_number));
    std::memcpy(&ack_number, &packet_raw[4], sizeof(ack_number));

    seq_number = ntohl(seq_number);
    ack_number = ntohl(ack_number);

    std::cout << "----------RECEIVING----------" << std::endl;

    std::cout << "Seq: " << seq_number << std::endl;
    std::cout << "Ack: " << ack_number << std::endl;


    return ack_number;
}


void check_need_for_retransmission(struct networking_options& networkingOptions) {
    // Iterate over the elements up to a maximum of the first five
    for (size_t i = 0; i < std::min(sent_packets.size(), static_cast<size_t>((WINDOW_SIZE + 1))); ++i) {
        auto& sent_packet = sent_packets[i];

        if (sent_packet.sent_counter >= RETRANSMISSION_COUNT) {
            printf("Retransmitting packet with sequence number %d\n", sent_packet.sequence_number);
            // Retransmit packet
            std::string packet = pack_header(&sent_packet);
            ssize_t ret_status = send_packet_over(networkingOptions, packet);

            if (ret_status < 0) {
                perror("Retransmission Failed To Send");
                return;
            }

            sent_packet.sent_counter = 0;
        }
    }
}

void write_data_to_file(FILE * stats_file, uint32_t sequence_number, time_t time_taken) {
    // Write the data to the file
    fprintf(stats_file, "%d, %ld\n", sequence_number, time_taken);

    // Flush the file
    fflush(stats_file);
}

void remove_packet_from_sent_packets(struct networking_options& networkingOptions, uint32_t ack_number) {
    for (auto it = sent_packets.begin(); it != sent_packets.end(); ++it) {
        if (it->sequence_number == ack_number) {
            // Calculate the time taken
            time_t time_taken = time(nullptr) - networkingOptions.time_started;
            write_data_to_file(networkingOptions.stats_file, it->sequence_number, time_taken);

            // Remove the packet from the list of sent packets
            sent_packets.erase(it); // Update iterator after erasing
            window_size--;
            return;
        }
    }
}


uint32_t receive_acknowledgements(struct networking_options& networkingOptions, int timeout_seconds) {
    ssize_t ret_status;

    modifying_global_variables.lock();

    if (window_size > WINDOW_SIZE) {
        increment_sent_counter();
    }

    // Check if any packets need to be retransmitted
    check_need_for_retransmission(networkingOptions);
    modifying_global_variables.unlock();

    // Set up fd_set for select
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(networkingOptions.socket_fd, &read_fds);

    // Set up timeout using timeval struct
    struct timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_seconds;

    // Use select to wait for data or timeout
    ret_status = select(networkingOptions.socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);

    if (ret_status == 0) {
        // Timeout occurred
        modifying_global_variables.lock();
        increment_sent_counter();
        modifying_global_variables.unlock();
        return 1;
    } else if (ret_status < 0) {
        // Error in select
        perror("Select failed");
        return 0;
    }

    // Receive the acknowledgement
    char buffer[HEADER_LENGTH];
    ret_status = recvfrom(networkingOptions.socket_fd, buffer, sizeof(buffer), 0, nullptr, nullptr);

    if (ret_status < 0) {
        perror("Receive Failed");
        return 0;
    }

    modifying_global_variables.lock();

    // Decode the acknowledgement
    uint32_t ack_number = decode_string(buffer);

    // Remove the packet from the list of sent packets
    remove_packet_from_sent_packets(networkingOptions, ack_number);

    modifying_global_variables.unlock();

    // Return the acknowledgement number except for the first packet
    return ack_number == 0 ? 1 : ack_number;
}
