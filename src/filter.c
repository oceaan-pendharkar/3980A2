#include "../include/filter.h"
#include <ctype.h>
#include <stdlib.h>

int upper_filter(char character)
{
    return toupper(character);
}

int lower_filter(char character)
{
    return tolower(character);
}

int null_filter(char character)
{
    return (int)character;
}

char *filter_string(const char *input_string, const size_t length, volatile int *exit_flag, filter_func filter)
{
    char *output_string = (char *)malloc((length * sizeof(char)) + 1);
    if(output_string == NULL)
    {
        *exit_flag = -2;
        return NULL;
    }
    for(int i = 0; (size_t)i < length; i++)
    {
        output_string[i] = (char)filter(input_string[i]);
    }
    output_string[length] = '\0';
    return output_string;
}
