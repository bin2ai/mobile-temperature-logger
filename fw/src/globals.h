#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// Pin definitions
const int OUT_LED_L = 11;
const int IN_BUTTON_PRESS = 3;
const int OUT_ENABLE = 1;
const int IN_CHARGING = 2;
const int IN_STANDBY = 0;
const int ANALOG_TEMP = 5;
const int ANALOG_BATT = 4;
const int ANALOG_VBUS = 3;

// Timing
const uint8_t DEBOUNCE_TIME = 100;
const uint16_t LED_BLINK_TIME = 500;
const unsigned long SERIAL_TIMEOUT = 1000;

// 1000 item uint16_t array
const uint16_t TELM_SIZE = 1000; // can be up to 1000

const size_t MAX_SUBSTRING_SIZE = 25;
extern char substr[MAX_SUBSTRING_SIZE];

const size_t MAX_RESPONSE_SIZE = 25;
extern char response[MAX_RESPONSE_SIZE];

// Declare the constants as extern
extern const char *CMD_PREFIX;
extern const char *RES_OK;
extern const char *RES_ERR1;
extern const char *RES_ERR2;
extern const char *RES_ERR3;
extern const char *RES_ERR4;
extern const char *RES_ERR5;

// Global variables
extern bool is_led_on;                     // true = on, false = off, default true
extern bool use_led;                       // true = use led, false = don't use led, default true
extern uint16_t telemetry[TELM_SIZE];      // each telemetry is 10 bits, 0-1023
extern uint16_t telemetry_index;           // 0-999 = index, default 0, >999 = full
extern uint16_t time_between_collection_s; // 0-65535 = seconds, default 60
extern uint8_t time_collection_delay_s;    // 0-255 = seconds, default 0
extern uint8_t samples_to_average;         // 0-255 = samples, default 10
extern bool state_collecting_telemetry;    // true = collecting, false = not collecting, default false

extern uint8_t count;

// enum states; idle, telemetry collection
enum states
{
    IDLE,             // default states upon
    WAITING_TO_START, // if delay is > 0, after start command, this will be the state
    COLLECTING,       // after start command and delay is done, this will be the state
    TELM_FULL         // after temperature collecting is complete, this will be the state
};

// make global state variable
extern states state;

extern bool is_utc_synced;
extern unsigned long utc_timestamp_seconds;
extern uint32_t local_start_time;

extern uint32_t time_start_collection;

extern uint32_t sum_of_samples;

#endif
