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

// the length passed in includes the null terminator
char *filter_string(const char *input_string, const size_t length, volatile sig_atomic_t *exit_flag, filter_func filter)
{
    char *output_string = (char *)malloc((length + 1) * sizeof(char));
    if(output_string == NULL)
    {
        *exit_flag = -2;
        return NULL;
    }
    for(int i = 0; (size_t)i < length; i++)
    {
        output_string[i] = (char)filter(input_string[i]);
    }
    output_string[length - 1] = '\0';
    return output_string;
}
