#if !defined(COMMANDS_H)
#define COMMANDS_H

#include <Arduino.h>

enum responses
{
    RESPONSE_OK,   // "OK"
    RESPONSE_ERR1, // "ERR1", invalid command
    RESPONSE_ERR2, // "ERR2", invalid arguments
    RESPONSE_ERR3, // "ERR3", invalid state
};

bool starts_with(const char *str, const char *prefix);
void init_substr();
void command_handler(char *at_cmd);
void generate_response(uint8_t resp);

#endif // COMMANDS_H
