#include "../include/server.h"
#include "../include/filter.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static volatile sig_atomic_t exit_flag = 0;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

char *get_message_content(const char *msg)
{
    const size_t length      = strlen(msg);
    char        *msg_content = (char *)malloc((length) * sizeof(char));
    if(msg_content == NULL)
    {
        return NULL;
    }
    strncpy(msg_content, msg + 2, length - 2);
    msg_content[length - 1] = '\0';
    return msg_content;
}

char *get_denied_message(void)
{
    const long unsigned int length         = 8;
    char                   *denied_message = (char *)malloc((length) * sizeof(char));
    strlcpy(denied_message, "denied", length);
    denied_message[length - 1] = '\0';
    return denied_message;
}

void *process_client(void *arg)
{
    int                     fd;
    const long unsigned int LIMIT = 100;
    char                   *output;
    char                   *input;
    const thread_data_t    *data     = (thread_data_t *)arg;
    volatile int           *exit_ptr = &exit_flag;
    ssize_t                 n_read;

    fd = open(data->input_fifo, O_RDONLY | O_CLOEXEC);
    if(fd < 0)
    {
        printf("Error opening input fifo");
        exit_flag = -3;
        goto done;
    }

    input = (char *)malloc(LIMIT * sizeof(char));
    if(input == NULL)
    {
        printf("Memory allocation failed\n");
        exit_flag = -2;
        goto done;
    }
    n_read = read(fd, input, LIMIT);
    if(n_read == -1)
    {
        if(errno == EINTR && exit_flag != 0)
        {
            printf("Exiting due to SIGINT\n");
            free(input);
            close(fd);
            goto done;
        }
        printf("writing failed");
        exit_flag = -4;
        free(input);
        close(fd);
        goto done;
    }
    input[n_read] = '\0';
    printf("input: %s\n", input);
    close(fd);

    if(exit_flag == 0)
    {
        char *message_content = get_message_content(input);
        if(message_content == NULL)
        {
            exit_flag = -2;
        }
        else
        {
            printf("strlen(message_content): %d\n", (int)strlen(message_content));
        }

        if(input[0] == 'u')
        {
            output = filter_string(message_content, strlen(message_content), exit_ptr, upper_filter);
        }
        else if(input[0] == 'l')
        {
            output = filter_string(message_content, strlen(message_content), exit_ptr, lower_filter);
        }
        else
        {
            output = filter_string(message_content, strlen(message_content), exit_ptr, null_filter);
        }
        free(message_content);
    }
    else
    {
        output = get_denied_message();
    }

    fd = open(data->output_fifo, O_WRONLY | O_CLOEXEC);
    if(fd < 0)
    {
        printf("Error opening output fifo");
        exit_flag = -2;
    }
    else
    {
        ssize_t n_wrote = write(fd, output, strlen(output));
        printf("output: %s\n", output);
        printf("strlen(output): %d\n", (int)sizeof(output));

        if(n_wrote < 0)
        {
            printf("writing failed\n");
            exit_flag = -4;
        }
        close(fd);
    }
    free(output);
    free(input);
done:
    return NULL;
}

int parse_server_arguments(int argc, char *args[], thread_data_t *data)
{
    int          opt;
    char *const *arguments  = args;
    const int    NUM_ARGS   = 5;
    const char  *arg1       = "-i";
    const char  *arg2       = "-o";
    const bool   HAS_INPUT  = arguments[1] != (void *)0 && strcmp(arguments[1], arg1) == 0;
    const bool   HAS_OUTPUT = arguments[3] != (void *)0 && strcmp(arguments[3], arg2) == 0;
    while((opt = getopt(argc, arguments, "i:o:")) != -1)
    {
        switch(opt)
        {
            case 'i':
                data->input_fifo = optarg;
                break;
            case 'o':
                data->output_fifo = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -i <input fifo path> -o <output fifo path>\n", arguments[0]);
                return -1;
        }
    }
    if(argc < NUM_ARGS || !HAS_INPUT || !HAS_OUTPUT)
    {
        printf("%d args\n", HAS_OUTPUT);
        fprintf(stderr, "Usage: %s -i <input fifo path> -o <output fifo path>\n", arguments[0]);
        return -1;
    }
    return 0;
}

void sigint_handler(int signum)
{
    exit_flag = signum;
}

/*
 * Author: D'Arcy Smith
 */
static void setup_signal_handler(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif
    sa.sa_handler = sigint_handler;

    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void process_client_with_thread(thread_data_t *data)
{
    int       success;
    pthread_t thread_id;
    success = pthread_create(&thread_id, NULL, process_client, (void *)data);
    if(exit_flag || success != 0)
    {
        return;
    }
    pthread_join(thread_id, NULL);
}

// exit flag values
//-1: parse args failed
//-2: memory allocation failed
//-3: error opening a fd
//-4: read/write failed
int main(int argc, char *argv[])
{
    thread_data_t *data = (thread_data_t *)malloc(sizeof(thread_data_t));
    if(data == NULL)
    {
        printf("data memory allocation failed");
        return -2;
    }

    setup_signal_handler();
    exit_flag = parse_server_arguments(argc, argv, data);
    if(exit_flag == -1)
    {
        free(data);
        goto done;
    }

    while(!exit_flag)
    {
        process_client_with_thread(data);
    }
    free(data);

done:
    return exit_flag;
}
