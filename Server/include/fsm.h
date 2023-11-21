#ifndef RELIABLE_UDP_FSM_H
#define RELIABLE_UDP_FSM_H

int entry_state(void *arg);
int parse_args(void *arg);
int set_up(void *arg);
int do_read(void *arg);
int clean_up(void *arg);
int print_error(void *arg);
int end_state(void *arg);

enum state_codes { ENTRY, PARSEARGS, SETUP,
        READ, FATALERROR, CLEANUP, END}; //must be the same order as states

enum ret_codes { ok, error, done};

struct transition {
    enum state_codes src_state;
    enum ret_codes   ret_code;
    enum state_codes dst_state;
};

#endif
