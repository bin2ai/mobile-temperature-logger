#include <Arduino.h>
#include "globals.h"
#include <EEPROM.h>

void init_substr()
{
    memset(substr, '\0', SIZE_SUB_CMD_MAX);
}

bool starts_with(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

char *substring(const char *str, size_t start, size_t end)
{
    size_t len = end - start;
    memset(substr, '\0', SIZE_SUB_CMD_MAX);
    strncpy(substr, str + start, len);
    return substr;
}

bool equals(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

void init_response()
{
    memset(response, '\0', SIZE_RESP_MAX);
}

void set_response(const char *str)
{
    strncpy(response, str, SIZE_RESP_MAX);
}

void command_handler(char *at_cmd)
{
    int len = strlen(at_cmd);
    init_substr();
    init_response();
    count++;

    if (!starts_with(at_cmd, CMD_PREFIX) && len < 3)
    {
        set_response(RES_ERR1);
    }
    else
    {
        at_cmd += 2; // Skip "AT"
        if (equals(at_cmd, ""))
        {
            set_response(RES_OK);
        }

        else if (starts_with(at_cmd, "+TEMP@"))
        {
            uint16_t index = atoi(at_cmd + 6);
            if (index >= 0 && index < SIZE_TEMP_TELM)
            {
                snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, temp_telm[index]);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else if (equals(at_cmd, "+TEMP#"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, index_temp_telm);
        }
        // update date time
        else if (starts_with(at_cmd, "+SYNC_UTC="))
        {
            char *utc_str = substring(at_cmd, 10, len);
            time_utc_timestamp_seconds = atol(utc_str);
            time_local_start = millis();
            is_utc_synced = true;
            snprintf(response, SIZE_RESP_MAX, "%s=%lu", RES_OK, time_utc_timestamp_seconds);
        }
        else if (equals(at_cmd, "+GET_UTC?"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%lu", RES_OK, time_utc_timestamp_seconds);
        }
        else if (equals(at_cmd, "+STATE?"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, state);
        }
        else if (equals(at_cmd, "+START"))
        {
            if (state == IDLE && is_utc_synced && index_temp_telm == 0)
            {
                state = WAITING_TO_START;
                time_start_collection = millis() / 1000 + time_collection_delay_s;
                memset(temp_telm, 0, sizeof(temp_telm));
                set_response(RES_OK);
            }
            else
            {
                set_response(RES_ERR5);
            }
        }
        else if (equals(at_cmd, "+STOP"))
        {
            if (state == COLLECTING || state == WAITING_TO_START)
            {
                state = IDLE;
                set_response(RES_OK);
            }
            else
            {
                set_response(RES_ERR5);
            }
        }
        else if (equals(at_cmd, "+CLEAR"))
        {
            if (state == IDLE || state == TELM_FULL)
            {

                // clear epprom, first byte is the index
                // 2nd bytes starts the data (16bit per data)
                EEPROM.write(0, 0);
                index_temp_telm = 0;

                set_response(RES_OK);
                state = IDLE;
            }
            else
            {
                set_response(RES_ERR5);
            }
        }
        else if (equals(at_cmd, "+TEMP?"))
        {
            int tempValue = analogRead(PIN_ANALOG_TEMP);
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, tempValue);
        }
        else if (equals(at_cmd, "+VBAT"))
        {
            uint16_t vbat_sum = 0;
            for (int i = 0; i < 10; i++)
            {
                vbat_sum += analogRead(PIN_ANALOG_BATT);
                delay(10);
            }

            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, vbat_sum / 10);
        }
        else if (equals(at_cmd, "+VUSB"))
        {
            uint16_t vusb_sum = 0;
            for (int i = 0; i < 10; i++)
            {
                vusb_sum += analogRead(PIN_ANALOG_VBUS);
                delay(10);
            }
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, vusb_sum / 10);
        }
        else if (equals(at_cmd, "+CHRG"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, digitalRead(PIN_IN_CHARGING));
        }
        else if (equals(at_cmd, "+STBY"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, digitalRead(PIN_IN_STANDBY));
        }
        else if (starts_with(at_cmd, "+DELAY="))
        {
            int delayValue = atoi(at_cmd + 7);
            if (delayValue >= 0 && delayValue <= 255)
            {
                time_collection_delay_s = delayValue;
                snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, time_collection_delay_s);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else if (equals(at_cmd, "+DELAY?"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, time_collection_delay_s);
        }
        else if (equals(at_cmd, "+SMPLS?"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, samples_to_average);
        }
        else if (starts_with(at_cmd, "+SMPLS="))
        {
            int samples = atoi(at_cmd + 7);
            if (samples >= 1 && samples <= 30)
            {
                samples_to_average = samples;
                snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, samples_to_average);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else if (starts_with(at_cmd, "+PERIOD="))
        {
            uint32_t interval = atoi(at_cmd + 8);
            if (interval <= 65535 && interval >= 1)
            {
                time_between_collection_s = interval;
                snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, time_between_collection_s);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else if (equals(at_cmd, "+PERIOD?"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, time_between_collection_s);
        }
        else if (equals(at_cmd, "+LED=ON"))
        {
            is_led_used = true;
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, is_led_used);
        }
        else if (equals(at_cmd, "+LED=OFF"))
        {
            is_led_used = false;
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, is_led_used);
        }
        else if (equals(at_cmd, "+LED?"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, is_led_used);
        }
        // get vbat log index
        else if (equals(at_cmd, "+VBATLOG#"))
        {
            snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, index_vbat_telm);
        }
        // get vbat log per index
        else if (starts_with(at_cmd, "+VBATLOG@"))
        {
            uint8_t index = atoi(at_cmd + 9);
            if (index >= 0 && index <= SIZE_VBAT_TELM - 1)
            {
                snprintf(response, SIZE_RESP_MAX, "%s=%d", RES_OK, vbat_telm[index]);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else
        {
            set_response(RES_ERR1);
        }
    }

    // Blink LED if enabled and if command is not "AT+TEMP@"...
    if (is_led_used && !starts_with(at_cmd, "+TEMP@"))
    {
        digitalWrite(PIN_OUT_LED_L, LOW);
        delay(50);
        digitalWrite(PIN_OUT_LED_L, HIGH);
        delay(50);
    }

    Serial.println(response);
}
