#include <Arduino.h>
#include "serial_util.h"
#include "globals.h"
#include "commands.h"

void serial_handler()
{
    // Check if data is available to read from the serial port
    if (Serial.available())
    {
        // allocate memory for the incoming data
        int c = Serial.read();
        unsigned long start_time = millis();
        while (c >= 0 && c != '\n' && c != '\r' && strlen(incoming_data) < SIZE_BUFFER_MAX - 1 && millis() - start_time < TIME_SERIAL_TIMEOUT)
        {
            incoming_data[strlen(incoming_data)] = c;
            c = Serial.read();
        }

        if (strlen(incoming_data) > 0)
        {
            command_handler(incoming_data);
        }

        // reset the buffer
        for (size_t i = 0; i < SIZE_BUFFER_MAX; i++)
            incoming_data[i] = '\0';
    }
}
