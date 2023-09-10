#include <Arduino.h>
#include "globals.h"

void init_substr()
{
    memset(substr, '\0', MAX_SUBSTRING_SIZE);
}

bool starts_with(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

char *substring(const char *str, size_t start, size_t end)
{
    size_t len = end - start;
    memset(substr, '\0', MAX_SUBSTRING_SIZE);
    strncpy(substr, str + start, len);
    return substr;
}

bool equals(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

void init_response()
{
    memset(response, '\0', MAX_RESPONSE_SIZE);
}

void set_response(const char *str)
{
    strncpy(response, str, MAX_RESPONSE_SIZE);
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
            if (index >= 0 && index < TELM_SIZE)
            {
                snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, telemetry[index]);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else if (equals(at_cmd, "+TEMP#"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, telemetry_index);
        }
        // update date time
        else if (starts_with(at_cmd, "+SYNC_UTC="))
        {
            char *utc_str = substring(at_cmd, 10, len);
            utc_timestamp_seconds = atol(utc_str);
            local_start_time = millis();
            is_utc_synced = true;
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%lu", RES_OK, utc_timestamp_seconds);
        }
        else if (equals(at_cmd, "+GET_UTC?"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%lu", RES_OK, utc_timestamp_seconds);
        }
        else if (equals(at_cmd, "+STATE?"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, state);
        }
        else if (equals(at_cmd, "+START"))
        {
            if (state == IDLE && is_utc_synced)
            {
                state = WAITING_TO_START;
                time_start_collection = millis() / 1000 + time_collection_delay_s;
                memset(telemetry, 0, sizeof(telemetry));
                telemetry_index = 0;
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
                memset(telemetry, 0, sizeof(telemetry));
                telemetry_index = 0;
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
            int tempValue = analogRead(ANALOG_TEMP);
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, tempValue);
        }
        else if (equals(at_cmd, "+VBAT"))
        {
            uint16_t vbat_sum = 0;
            for (int i = 0; i < 10; i++)
            {
                vbat_sum += analogRead(ANALOG_BATT);
                delay(10);
            }

            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, vbat_sum / 10);
        }
        else if (equals(at_cmd, "+VUSB"))
        {
            uint16_t vusb_sum = 0;
            for (int i = 0; i < 10; i++)
            {
                vusb_sum += analogRead(ANALOG_VBUS);
                delay(10);
            }
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, vusb_sum / 10);
        }
        else if (equals(at_cmd, "+CHRG"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, digitalRead(IN_CHARGING));
        }
        else if (equals(at_cmd, "+STBY"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, digitalRead(IN_STANDBY));
        }
        else if (starts_with(at_cmd, "+DELAY="))
        {
            int delayValue = atoi(at_cmd + 7);
            if (delayValue >= 0 && delayValue <= 255)
            {
                time_collection_delay_s = delayValue;
                snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, time_collection_delay_s);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else if (equals(at_cmd, "+DELAY?"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, time_collection_delay_s);
        }
        else if (equals(at_cmd, "+SMPLS?"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, samples_to_average);
        }
        else if (starts_with(at_cmd, "+SMPLS="))
        {
            int samples = atoi(at_cmd + 7);
            if (samples >= 1 && samples <= 30)
            {
                samples_to_average = samples;
                snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, samples_to_average);
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
                snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, time_between_collection_s);
            }
            else
            {
                set_response(RES_ERR4);
            }
        }
        else if (equals(at_cmd, "+PERIOD?"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, time_between_collection_s);
        }
        else if (equals(at_cmd, "+LED=ON"))
        {
            use_led = true;
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, use_led);
        }
        else if (equals(at_cmd, "+LED=OFF"))
        {
            use_led = false;
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, use_led);
        }
        else if (equals(at_cmd, "+LED?"))
        {
            snprintf(response, MAX_RESPONSE_SIZE, "%s=%d", RES_OK, use_led);
        }
        else
        {
            set_response(RES_ERR1);
        }
    }

    // Blink LED if enabled and if command is not "AT+TEMP@"...
    if (use_led && !starts_with(at_cmd, "+TEMP@"))
    {
        digitalWrite(OUT_LED_L, LOW);
        delay(50);
        digitalWrite(OUT_LED_L, HIGH);
        delay(50);
    }

    Serial.println(response);
}
