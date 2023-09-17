#include <Arduino.h>
#include <EEPROM.h>
#include "LowPower.h"
#include "utils\serial_handler.h"
#include "utils\globals.h"
#include "utils\led.h"
#include "utils\temperature.h"
#include "utils\button.h"
#include "utils\serial_util.h"
void setup()
{
  // initialize the digital pin as an output.
  pinMode(PIN_OUT_LED_L, OUTPUT);
  digitalWrite(PIN_OUT_LED_L, HIGH);
  pinMode(PIN_IN_BUTTON_PRESS, INPUT); // Enable internal pull-up resistor
  pinMode(PIN_OUT_ENABLE, OUTPUT);
  digitalWrite(PIN_OUT_ENABLE, HIGH);
  pinMode(PIN_IN_VBUS, INPUT);

  blink_led(2);

  // setup telemetry, initalize to 0
  for (uint16_t i = 0; i < SIZE_TEMP_TELM; i++)
    temp_telm[i] = 0;

  // read eeprom for telemetry index
  index_temp_telm = EEPROM.read(0);
  // if index is greater than 0, then telemetry is full
  if (index_temp_telm > 0)
    state = STATE_TELM_FULL;
  else
    state = STATE_IDLE;

  // if state is not full
  if (state == STATE_TELM_FULL)
  {
    // read eeprom for telemetry
    for (uint16_t i = 0; i < SIZE_TEMP_TELM; i++)
      temp_telm[i] = EEPROM.read(i + 1);
  }
  else
  {
    // setup vbat telemetry, initalize to 0
    index_vbat_telm = 0;
    for (uint16_t i = 0; i < SIZE_VBAT_TELM; i++)
      vbat_telm[i] = 0;
  }
  Serial.begin(9600);
}

void loop()
{

  // read VBUS to check if USB is connected
  if (digitalRead(PIN_IN_VBUS) == HIGH && !is_usb_connected)
  {
    // attach usb
    USBDevice.attach();
    is_usb_connected = true;
    delay(1000);
    blink_led(2);
  }
  else if (digitalRead(PIN_IN_VBUS) == HIGH && is_usb_connected)
  {
    ; // do nothing
  }
  else if (digitalRead(PIN_IN_VBUS) == LOW && is_usb_connected)
  {
    // detach usb
    USBDevice.detach();
    is_usb_connected = false;
    delay(1000);
    blink_led(2);
  }
  else if (digitalRead(PIN_IN_VBUS) == LOW && !is_usb_connected)
  {
    ; // do nothing
  }

  if (!is_usb_connected)
  {
    // low power mode, sleep for 8 seconds
    //  check if button is pressed
    LowPower.powerStandby(SLEEP_8S, ADC_OFF, BOD_OFF);
  }

  check_button();

  // if state is start waiting
  if (state == STATE_WAITING_TO_START)
  {
    if (millis() >= time_start_collection * 1000)
    {
      state = STATE_COLLECTING;
      // reset the telemetry index
      index_temp_telm = 0;
      collect_telmetry();

      time_utc_timestamp_seconds += (millis() - time_local_start) / 1000;
    }
  }
  else if (state == STATE_COLLECTING)
  {
    if (millis() >= (time_between_collection_s * (index_temp_telm) + time_start_collection) * 1000)
      collect_telmetry();
  }

  // Check if data is available to read from the serial port
  if (Serial.available())
    serial_handler();
}
