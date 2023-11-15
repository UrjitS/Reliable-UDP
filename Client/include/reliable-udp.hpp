#ifndef CLIENT_RELIABLE_UDP_HPP
#define CLIENT_RELIABLE_UDP_HPP

#define MAX_WINDOW 5

int send_packet(struct networking_options& networkingOptions);
int receive_acknowledgements(struct networking_options& networkingOptions);

#endif
