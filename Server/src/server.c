//
// Created by Colin Lam on 2023-11-13.
//

#include "server.h"

int entry_state(void *arg)
{
    printf("Starting server...\n");
    return ok;
}

int parse_args(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;

    if(opts->argc != SERVER_ARGS)
    {
        opts->msg = strdup("Invalid number of arguments\n");
        return error;
    }

    opts->host_ip = strdup(opts->argv[IP_INDEX]);
    opts->ip_family = get_ip_family(opts->host_ip);
    if(opts->ip_family == -1)
    {
        opts->msg = strdup("IP address is not a valid IPv4 or IPv6 address\n");
        return error;
    }

    if(parse_in_port_t(opts) == -1)
    {
        return error;
    }

    printf("---------------------------- Server Options ----------------------------\n");
    printf("Server IP Address: %s\n", opts->host_ip);
    printf("Server Domain: %d\n", opts->ip_family);
    printf("Server Port: %hu\n", opts->host_port);
    printf("---------------------------- Server Options ----------------------------\n");

    return ok;

}

int get_ip_family(const char *ip_addr)
{
    int domain;
    domain = -1;

    if(strstr(ip_addr, ":"))
    {
        domain = AF_INET6;
    }
    else if (strstr(ip_addr, "."))
    {
        domain = AF_INET;
    }

    return domain;

}

int parse_in_port_t(struct server_opts *opts)
{
    char *endptr;
    uintmax_t parsed_value;

    parsed_value = strtoumax(opts->argv[PORT_INDEX], &endptr, 10);

    if (errno != 0)
    {
        opts->msg = strdup("Error parsing in_port_t\n");
        return -1;
    }

    // Check if there are any non-numeric characters in the input string
    if (*endptr != '\0')
    {
        opts->msg = strdup("Invalid characters in input.\n");
        return -1;
    }

    // Check if the parsed value is within the valid range for in_port_t
    if (parsed_value > UINT16_MAX)
    {
        opts->msg = strdup("in_port_t value out of range.\n");
        return -1;
    }

    opts->host_port = (in_port_t)parsed_value;
    return 0;
}

int set_up(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;
    printf("In setup\n");
    return ok;
}

int print_error(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;
    printf("%s\n", opts->msg);

    return ok;
}

int clean_up(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;
    if(opts->msg)
    {
        free(opts->msg);
    }

    return ok;
}

int end_state(void *arg)
{
    printf("Server closing...\n");
    return ok;
}
