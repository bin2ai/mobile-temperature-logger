#include "temperature.h"
#include "globals.h"
#include "led.h"
#include <EEPROM.h>
#include <Arduino.h>

void collect_telmetry()
{
    if (telemetry_index >= TELM_SIZE)
    {
        state = TELM_FULL;
        return;
    }

    // collect sample
    if (samples_to_average == 0)
    {
        telemetry[telemetry_index] = analogRead(ANALOG_TEMP);
        telemetry_index++;
    }
    else
    {
        uint16_t sum_of_samples = 0;
        // collect samples
        for (uint8_t i = 0; i < samples_to_average; i++)
        {
            sum_of_samples += analogRead(ANALOG_TEMP);
            delay(10);
        }

        telemetry[telemetry_index] = sum_of_samples / samples_to_average;

        // write telemetry index to eeprom
        EEPROM.write(0, telemetry_index);
        // divid telemetry by 2 and add to eeprom, 2 bytes into 1 byte
        EEPROM.write(telemetry_index + 1, telemetry[telemetry_index] / 2);
        telemetry_index++;
    }

    // if led on
    if (use_led)
    {
        blink_led();
    }
}
