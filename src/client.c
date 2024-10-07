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

char *initialize_input_string(const char *filter_type, const char *message, int *err)
{
    char *input = (char *)malloc(LIMIT * sizeof(char));
    if(input == NULL)
    {
        printf("Memory allocation failed\n");
        *err = -2;
        return 0;
    }
    input[0] = *filter_type;
    input[1] = DELIMITER;
    strcat(input, message);
    return input;
}

// exit values
//-1: parse args failed
//-2: memory allocation failed
//-3: error opening a fifo
//-4: error reading or writing

int main(int argc, char *argv[])
{
    Client_Settings settings;
    const char     *inputFifo  = "/tmp/inputfifo";
    const char     *outputFifo = "/tmp/outputfifo";
    int             fd;
    char           *input;
    int             exit_flag = 0;

    exit_flag = parse_arguments(argc, argv, &settings);
    if(exit_flag == -1)
    {
        goto done;
    }

    input = initialize_input_string(&settings.filter_type, settings.message, &exit_flag);
    if(exit_flag != 0)
    {
        free(input);
        goto done;
    }

    fd = open(inputFifo, O_WRONLY | O_CLOEXEC);
    if(fd < 0)
    {
        printf("Error opening input fifo for writing");
        exit_flag = -3;
        free(input);
        goto done;
    }
    else
    {
        write_string_to_fd(input, &fd, &exit_flag);
        if(exit_flag != 0)
        {
            goto done;
        }
    }

    fd = open(outputFifo, O_RDONLY | O_CLOEXEC);
    if(fd < 0)
    {
        printf("Error opening output fifo for reading");
        exit_flag = -3;
        goto done;
    }
    else
    {
        read_string_from_fd(strlen(settings.message), &fd, &exit_flag);
        if(exit_flag != 0)
        {
            goto done;
        }
    }

done:
    return exit_flag;
}
