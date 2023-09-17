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
  pinMode(PIN_OUT_LED_L, OUTPUT);
  digitalWrite(PIN_OUT_LED_L, HIGH);
  pinMode(PIN_IN_BUTTON_PRESS, INPUT); // Enable internal pull-up resistor
  pinMode(PIN_OUT_ENABLE, OUTPUT);
  digitalWrite(PIN_OUT_ENABLE, HIGH);

  blink_led();

  // setup telemetry, initalize to 0
  for (uint16_t i = 0; i < SIZE_TEMP_TELM; i++)
    temp_telm[i] = 0;

  // read eeprom for telemetry index
  index_temp_telm = EEPROM.read(0);

  // vbat log index 0 and loop through set 0
  index_vbat_telm = 0;
  for (uint16_t i = 0; i < SIZE_VBAT_TELM; i++)
    vbat_telm[i] = 0;

  Serial.begin(9600);
  // set buffer limit to 10 characters long, reject anything longer
  // not a a timeout
}

void loop()
{

  // if its been 10 seconds, blink
  if (millis() - time_last_blink >= 10000)
  {
    blink_led();
    time_last_blink = millis();
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
      index_temp_telm = 0;
      collect_telmetry();

      time_utc_timestamp_seconds += (millis() - time_local_start) / 1000;
    }
  }
  if (state == COLLECTING)
  {
    if (millis() >= (time_between_collection_s * (index_temp_telm) + time_start_collection) * 1000)
      collect_telmetry();
  }

  // Check if data is available to read from the serial port
  if (Serial.available())
    serial_handler();
}
