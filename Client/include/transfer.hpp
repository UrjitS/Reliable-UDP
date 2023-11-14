#ifndef CLIENT_TRANSFER_HPP
#define CLIENT_TRANSFER_HPP

/**
 * @brief Sends the input to the server
 * @param networkingOptions Networking options struct
 * @param exit_flag Exit flag for when the thread should stop
 */
void send_input(struct networking_options& networkingOptions, volatile int& exit_flag);
/**
 * @brief Reads the response from the server
 * @param networkingOptions Networking options struct
 * @param exit_flag Exit flag for when the thread should stop
 */
void read_response(struct networking_options& networkingOptions, volatile int& exit_flag);

#endif
