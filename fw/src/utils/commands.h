#if !defined(COMMANDS_H)
#define COMMANDS_H

#include <Arduino.h>

bool starts_with(const char *str, const char *prefix);
void set_response(const char *str);
void init_substr();
void command_handler(char *at_cmd);

#endif // COMMANDS_H
