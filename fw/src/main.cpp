#include <Arduino.h>
#include <EEPROM.h>
#include "utils\serial_handler.h"
#include "utils\globals.h"
#include "utils\led.h"
#include "utils\temperature.h"
#include "utils\button.h"
#include "utils\serial_util.h"

void setup()
{
  // initialize enum states (from globals.h) to IDLE
  pinMode(OUT_LED_L, OUTPUT);
  digitalWrite(OUT_LED_L, HIGH);
  pinMode(IN_BUTTON_PRESS, INPUT); // Enable internal pull-up resistor
  pinMode(OUT_ENABLE, OUTPUT);
  digitalWrite(OUT_ENABLE, HIGH);

  blink_led();

  // setup telemetry, initalize to 0
  for (uint16_t i = 0; i < TELM_SIZE; i++)
    telemetry[i] = 0;

  // read eeprom for telemetry index
  telemetry_index = EEPROM.read(0);

  // vbat log index 0 and loop through set 0
  vbat_log_index = 0;
  for (uint16_t i = 0; i < VBAT_LOG_SIZE; i++)
    vbat_log[i] = 0;

  Serial.begin(9600);
  // set buffer limit to 10 characters long, reject anything longer
  // not a a timeout
}

void loop()
{

  // if its been 10 seconds, blink
  if (millis() - lastBlinkTime >= 10000)
  {
    blink_led();
    lastBlinkTime = millis();
  }

  // check if button is pressed
  check_button();

  // if state is start waiting
  if (state == WAITING_TO_START)
  {
    if (millis() >= time_start_collection * 1000)
    {
      state = COLLECTING;
      // reset the telemetry index
      telemetry_index = 0;
      collect_telmetry();

      utc_timestamp_seconds += (millis() - local_start_time) / 1000;
    }
  }
  if (state == COLLECTING)
  {
    if (millis() >= (time_between_collection_s * (telemetry_index) + time_start_collection) * 1000)
      collect_telmetry();
  }

  // Check if data is available to read from the serial port
  if (Serial.available())
    serial_handler();
}
