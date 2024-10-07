#include "../include/client.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define LIMIT 100
#define DELIMITER '|'

int parse_arguments(int argc, char *args[], Client_Settings *settings)
{
    int          opt;
    char *const *arguments = args;
    settings->filter_type  = 'n';
    settings->message      = NULL;
    while((opt = getopt(argc, arguments, "s:lu")) != -1)
    {
        switch(opt)
        {
            case 'l':
            case 'u':
                settings->filter_type = (char)opt;
                break;
            case 's':
                settings->message = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -s [<the string>] [-u or -l (optional)]\n", arguments[0]);
                return -1;
        }
    }
    if(!settings->filter_type || !settings->message)
    {
        fprintf(stderr, "Usage: %s -s [<the string>] [-u or -l (optional)]\n", arguments[0]);
        return -1;
    }
    return 0;
}

void write_string_to_fd(char *input, const int *fd, int *err)
{
    ssize_t n_wrote = write(*fd, input, strlen(input) + 1);
    if(n_wrote < 0)
    {
        *err = -4;
    }
    close(*fd);
    free(input);
}

void read_string_from_fd(const unsigned long length, const int *fd, int *err)
{
    char   *output = (char *)malloc(length * sizeof(char));
    ssize_t n_read = read(*fd, output, length + 1);
    if(n_read < 0)
    {
        *err = -4;
    }
    close(*fd);
    printf("response from server: %s\n", output);
    free(output);
}

char *initialize_input_string(const Client_Settings *settings, int *err)
{
    char *input = (char *)malloc(LIMIT * sizeof(char));
    if(input == NULL)
    {
        printf("Memory allocatio_ failed\n");
        *err = -2;
        return NULL;
    }
    input[0] = settings->filter_type;
    input[1] = DELIMITER;
    strcat(input, settings->message);
    return input;
}

void initialize_fifos(Client_Settings *settings)
{
    settings->inputFifo  = "/tmp/inputfifo";
    settings->outputFifo = "/tmp/outputfifo";
}

void cleanup(Client_Settings *settings)
{
    free(settings->server_input);
}

void send_server_request(Client_Settings *settings)
{
    settings->fd = open(settings->inputFifo, O_WRONLY | O_CLOEXEC);
    if(settings->fd < 0)
    {
        printf("Error opening input fifo for writing");
        settings->exit_flag = -3;
        free(settings->server_input);
        return;
    }

    write_string_to_fd(settings->server_input, &settings->fd, &settings->exit_flag);
    if(settings->exit_flag != 0)
    {
        return;
    }
}

void receive_server_response(Client_Settings *settings)
{
    settings->fd = open(settings->outputFifo, O_RDONLY | O_CLOEXEC);
    if(settings->fd < 0)
    {
        printf("Error opening output fifo for reading");
        settings->exit_flag = -3;
        return;
    }
    read_string_from_fd(strlen(settings->message), &settings->fd, &settings->exit_flag);
    if(settings->exit_flag != 0)
    {
        return;
    }
}

// exit values
//-1: parse args failed
//-2: memory allocation failed
//-3: error opening a fifo
//-4: error reading or writing

int main(int argc, char *argv[])
{
    Client_Settings settings;
    initialize_fifos(&settings);

    settings.exit_flag = parse_arguments(argc, argv, &settings);
    if(settings.exit_flag == -1)
    {
        goto done;
    }

    settings.server_input = initialize_input_string(&settings, &settings.exit_flag);
    if(settings.exit_flag != 0)
    {
        cleanup(&settings);
        goto done;
    }

    send_server_request(&settings);
    if(settings.exit_flag != 0)
    {
        goto done;
    }

    receive_server_response(&settings);

done:
    return settings.exit_flag;
}
