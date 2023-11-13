#ifndef CLIENT_TRANSFER_HPP
#define CLIENT_TRANSFER_HPP


void send_input(struct networking_options& networkingOptions, volatile int& exit_flag);
void read_response(struct networking_options& networkingOptions, volatile int& exit_flag);

#endif //CLIENT_TRANSFER_HPP
