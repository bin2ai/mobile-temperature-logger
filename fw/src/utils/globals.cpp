#include "globals.h"

// Initialize global variables
bool is_led_on = true;
bool use_led = true;
uint16_t telemetry[TELM_SIZE];
uint16_t telemetry_index = 0;

uint16_t time_between_collection_s = 60;
uint8_t time_collection_delay_s = 0;
uint8_t samples_to_average = 10;
bool state_collecting_telemetry = false;

uint8_t count = 0;

char substr[MAX_SUBSTRING_SIZE];
char response[MAX_RESPONSE_SIZE];

// Define the constants
const char *CMD_PREFIX = "AT";
const char *RES_OK = "+OK";
const char *RES_ERR1 = "+ERR1";
const char *RES_ERR2 = "+ERR2";
const char *RES_ERR3 = "+ERR3";
const char *RES_ERR4 = "+ERR4";
const char *RES_ERR5 = "+ERR5";

states state = IDLE;

unsigned long utc_timestamp_seconds = 0;
uint32_t time_start_collection = 0;
unsigned long local_start_time = 0;
bool is_utc_synced = false;

uint16_t vbat_log[VBAT_LOG_SIZE];
uint8_t vbat_log_index = 0;

char incomingData[MAX_BUFFER_SIZE];

bool buttonPressed = false;
unsigned long buttonPressStartTime = 0;

unsigned long lastBlinkTime = 0;
