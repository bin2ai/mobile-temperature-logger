#include "temperature.h"
#include "globals.h"
#include "led.h"
#include <EEPROM.h>
#include <Arduino.h>

void collect_telmetry()
{
    if (index_temp_telm >= SIZE_TEMP_TELM)
    {
        state = TELM_FULL;
        return;
    }

    // collect sample
    if (samples_to_average == 0)
    {
        temp_telm[index_temp_telm] = analogRead(PIN_ANALOG_TEMP);
        index_temp_telm++;
    }
    else
    {
        uint16_t sum_of_samples = 0;
        // collect samples
        for (uint8_t i = 0; i < samples_to_average; i++)
        {
            sum_of_samples += analogRead(PIN_ANALOG_TEMP);
            delay(10);
        }

        temp_telm[index_temp_telm] = sum_of_samples / samples_to_average;

        // write telemetry index to eeprom
        EEPROM.write(0, index_temp_telm);
        // divid telemetry by 2 and add to eeprom, 2 bytes into 1 byte
        EEPROM.write(index_temp_telm + 1, temp_telm[index_temp_telm] / 2);
        index_temp_telm++;
    }

    // if led on
    if (is_led_used)
    {
        blink_led();
    }
}
