#include <Arduino.h>
#include "battery.h"
#include "globals.h"

void log_vbat()
{
    // read the battery voltage
    uint16_t vbat_full_bit = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        vbat_full_bit += analogRead(ANALOG_BATT);
        delay(10);
    }

    vbat_full_bit = vbat_full_bit / 10;

    uint8_t vbat_log_size = int(vbat_full_bit / VBAT_LOG_SIZE);

    // if the vbat log is empty, add the first value
    if (vbat_log_index == VBAT_LOG_SIZE - 1)
    {
        if (vbat_full_bit <= vbat_log_size)
        {
            uint32_t current_time = int(millis() / 1000 + utc_timestamp_seconds);
            vbat_log[vbat_log_index] = current_time;
            vbat_log_index++;
        }
    }
    else
    {
        if (vbat_full_bit <= vbat_full_bit - vbat_log_size * vbat_log_index)
        {
            uint32_t current_time = int(millis() / 1000 + utc_timestamp_seconds);
            vbat_log[vbat_log_index] = current_time;
            vbat_log_index++;
        }
    }
}