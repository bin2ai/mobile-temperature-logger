#include "globals.h"

// Initialize global variables
bool is_led_on = true;
bool is_led_used = true;
uint16_t temp_telm[SIZE_TEMP_TELM];
uint16_t index_temp_telm = 0;

uint16_t time_between_collection_s = 60;
uint8_t time_collection_delay_s = 0;
uint8_t samples_to_average = 10;
bool is_state_collecting_telemetry = false;

uint8_t count = 0;

char substr[SIZE_SUB_CMD_MAX];
char response[SIZE_RESP_MAX];

states state = STATE_IDLE;

unsigned long time_utc_timestamp_seconds = 0;
uint32_t time_start_collection = 0;
unsigned long local_start_time = 0;
bool is_utc_synced = false;

uint16_t vbat_telm[SIZE_VBAT_TELM];
uint8_t index_vbat_telm = 0;

char incoming_data[SIZE_BUFFER_MAX];

bool is_button_pressed = false;
unsigned long time_button_press_start = 0;

unsigned long time_last_blink = 0;
uint32_t time_local_start = 0;

bool is_usb_connected = false;