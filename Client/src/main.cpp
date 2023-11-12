#include "networking.hpp"
#include <iostream>
#include <p101_env/env.h>
#include <p101_error/error.h>
#include <p101_fsm/fsm.h>
#include <arpa/inet.h>
#include <bits/getopt_core.h>
#include <atomic>

using namespace std;

static int parse_arguments([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg);
static int print_program_usage([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg);
static void check_ip_address(struct networking_options &networkingOptions);
static int display_error([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg);
int read_input([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg);
int send_input([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg);
int clean_resources([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg);
int setup_connection([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg);

enum client_states {
    PARSE_ARGUMENTS = P101_FSM_USER_START,
    SETUP_CONNECTION,
    READ_INPUT,
    SEND_INPUT,
    DISPLAY_ERROR,
    DISPLAY_USAGE,
    CLEAN_RESOURCES
};

int main(int argc, char * argv[]) {
    // Get cmd line arguments
    struct networking_options networkingOptions{};
    struct p101_error * err;
    struct p101_env * env;

    err = p101_error_create(false);
    env = p101_env_create(err, false, nullptr);


    networkingOptions.argc = argc;
    networkingOptions.argv = argv;

    struct p101_fsm_info * fsm_info;
    struct p101_fsm_transition transitions[] = {
            {P101_FSM_INIT,    PARSE_ARGUMENTS, parse_arguments},
            {PARSE_ARGUMENTS,  DISPLAY_USAGE,   display_error},
            {PARSE_ARGUMENTS,  DISPLAY_ERROR,   print_program_usage},
            {PARSE_ARGUMENTS,  SETUP_CONNECTION,setup_connection},
            {SETUP_CONNECTION, READ_INPUT,      read_input},
            {SETUP_CONNECTION, DISPLAY_ERROR,   read_input},
            {READ_INPUT,       SEND_INPUT,      send_input},
            {READ_INPUT,       DISPLAY_ERROR,   display_error},
            {READ_INPUT,       CLEAN_RESOURCES, display_error},
            {SEND_INPUT,       READ_INPUT,      read_input},
            {SEND_INPUT,      DISPLAY_ERROR,   display_error},
            {DISPLAY_ERROR,   CLEAN_RESOURCES, clean_resources},
            {DISPLAY_USAGE,   CLEAN_RESOURCES, clean_resources},
            {CLEAN_RESOURCES, P101_FSM_EXIT,   nullptr},
    };
    struct p101_error * fsm_err;
    struct p101_env * fsm_env;

    fsm_err = p101_error_create(false);
    fsm_env = p101_env_create(err, false, nullptr);
    fsm_info = p101_fsm_info_create(env, err, "client_fsm", fsm_env, fsm_err, nullptr);

    if (p101_error_has_no_error(err)) {
        int from_state;
        int to_state;

        p101_fsm_run(fsm_info, &from_state, &to_state, &networkingOptions, transitions);
        p101_fsm_info_destroy(env, &fsm_info);
        free(fsm_err);
        free(fsm_env);
    }

    free(err);
    free(env);

    return EXIT_SUCCESS;
}

int setup_connection([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg) {
    auto * networkingOptions = (struct networking_options *) arg;

    return CLEAN_RESOURCES;
}

int read_input([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg) {
    auto * networkingOptions = (struct networking_options *) arg;
    // Loop until ctrl + c
    return CLEAN_RESOURCES;
}

int send_input([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg) {
    auto * networkingOptions = (struct networking_options *) arg;

    return CLEAN_RESOURCES;
}

int clean_resources([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg) {
    auto * networkingOptions = (struct networking_options *) arg;

    if (networkingOptions->socket_fd > 0) {
        close(networkingOptions->socket_fd);
    }

    return P101_FSM_EXIT;
}

int parse_arguments([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg) {
    auto * networkingOptions = (struct networking_options *) arg;
    int opt;

    opterr = 0;

    if(networkingOptions->argc < 4)
    {
        networkingOptions->message = "Please give IP address, port, and file(s).";
        return DISPLAY_ERROR;
    }

    while ((opt = getopt(networkingOptions->argc, networkingOptions->argv, "h")) != -1) {
        switch (opt) {
            case 'h':
            {
                networkingOptions->ip_address = optarg;
                break;
            }
            case 'p':
            {
                char * end_ptr;
                long port = std::strtol(optarg, &end_ptr, 10);

                if (*end_ptr != '\0' || errno == ERANGE || port < 0 || port > 65535) {
                    std::cerr << "Invalid port number: " << optarg << std::endl;
                    exit(EXIT_FAILURE);
                }

                networkingOptions->port_number = static_cast<in_port_t>(port);
                break;
            }
            case '?':
            {
                cout << "Unknown option: " << optopt << endl;
                return DISPLAY_ERROR;
            }
            default:
            {
                return DISPLAY_ERROR;
            }
        }
    }

    if(optind >= networkingOptions->argc)
    {
        networkingOptions->message = "The ip address, port, and directory are required";
        return DISPLAY_ERROR;
    }

    if(optind + 2 >= networkingOptions->argc)
    {
        networkingOptions->message = "The port, and directory are required";
        return DISPLAY_ERROR;
    }

    // Handle IP
    printf("Running on ip address: %s \n", networkingOptions->argv[optind]);
    networkingOptions->ip_address = networkingOptions->argv[optind];
    if (!validate_ip_address(networkingOptions->ip_address)) {
        networkingOptions->message = "Invalid IP Address";
        return DISPLAY_ERROR;
    }

    // Handle Port Number
    char * end_ptr;
    long port = std::strtol(networkingOptions->argv[optind+1], &end_ptr, 10);

    if (*end_ptr != '\0' || errno == ERANGE || port < 0 || port > 65535) {
        networkingOptions->message = "Invalid Port Number";
        return DISPLAY_ERROR;
    }

    networkingOptions->port_number = static_cast<in_port_t>(port);

    check_ip_address(*networkingOptions);

    cout << "Given Ip Address: " << networkingOptions->ip_address << endl;
    cout << "Given Port: " << networkingOptions->port_number << endl;

    return READ_INPUT;
}

static void check_ip_address(struct networking_options &networkingOptions) {
    struct sockaddr_storage addr{};

    if(inet_pton(AF_INET, networkingOptions.ip_address.c_str(), &(((struct sockaddr_in *)&addr)->sin_addr)) == 1)
    {
        networkingOptions.ip_family = AF_INET;
    }
    else if(inet_pton(AF_INET6, networkingOptions.ip_address.c_str(), &(((struct sockaddr_in6 *)&addr)->sin6_addr)) == 1)
    {
        networkingOptions.ip_family = AF_INET6;
    }
}

static int print_program_usage([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg) {
    auto * networkingOptions = (struct networking_options *) arg;

    if (!networkingOptions->message.empty()) {
        cerr << networkingOptions->message << endl;
    }

    cerr << "Usage: " << networkingOptions->argv[0] << " <ip address>, <port number>, file name(s)" << endl;

    return CLEAN_RESOURCES;
}

static int display_error([[maybe_unused]] const struct p101_env *env, [[maybe_unused]] struct p101_error *err, void *arg) {
    auto * networkingOptions = (struct networking_options *) arg;

    if (!networkingOptions->message.empty()) {
        cerr << networkingOptions->message << endl;
    }

    return CLEAN_RESOURCES;
}
