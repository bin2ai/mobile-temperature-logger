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
        while (c >= 0 && c != '\n' && c != '\r' && strlen(incomingData) < MAX_BUFFER_SIZE - 1 && millis() - start_time < SERIAL_TIMEOUT)
        {
            incomingData[strlen(incomingData)] = c;
            c = Serial.read();
        }

        if (strlen(incomingData) > 0)
        {
            command_handler(incomingData);
        }

        // reset the buffer
        for (size_t i = 0; i < MAX_BUFFER_SIZE; i++)
            incomingData[i] = '\0';
    }
}
