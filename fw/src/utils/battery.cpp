#include <Arduino.h>
#include "battery.h"
#include "globals.h"

void log_vbat()
{
    // read the battery voltage
    uint16_t vbat_full_bit = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        vbat_full_bit += analogRead(PIN_ANALOG_BATT);
        delay(10);
    }

    vbat_full_bit = vbat_full_bit / 10;

    uint8_t vbat_log_size = int(vbat_full_bit / SIZE_VBAT_TELM);

    // if the vbat log is empty, add the first value
    if (index_vbat_telm == SIZE_VBAT_TELM - 1)
    {
        if (vbat_full_bit <= vbat_log_size)
        {
            uint32_t current_time = int(millis() / 1000 + time_utc_timestamp_seconds);
            vbat_telm[index_vbat_telm] = current_time;
            index_vbat_telm++;
        }
    }
    else
    {
        if (vbat_full_bit <= vbat_full_bit - vbat_log_size * index_vbat_telm)
        {
            uint32_t current_time = int(millis() / 1000 + time_utc_timestamp_seconds);
            vbat_telm[index_vbat_telm] = current_time;
            index_vbat_telm++;
        }
    }
}