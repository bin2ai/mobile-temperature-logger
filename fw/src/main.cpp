#include <Arduino.h>
#include <EEPROM.h>
#include "serial_handler.h"
#include "commands.h"
#include "globals.h"
#include "stdint.h"

const size_t MAX_BUFFER_SIZE = 30;
char incomingData[MAX_BUFFER_SIZE];

bool buttonPressed = false;
unsigned long buttonPressStartTime = 0;

unsigned long lastBlinkTime = 0;
void blink_led()
{
  digitalWrite(OUT_LED_L, LOW);
  delay(50);
  digitalWrite(OUT_LED_L, HIGH);
  delay(50);
  digitalWrite(OUT_LED_L, LOW);
  delay(50);
  digitalWrite(OUT_LED_L, HIGH);
}

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

  telemetry_index = 0;

  Serial.begin(9600);
  // set buffer limit to 10 characters long, reject anything longer
  // not a a timeout
}

void collect_telmetry()
{
  if (telemetry_index >= TELM_SIZE)
  {
    state = TELM_FULL;
    return;
  }

  // collect sample
  if (samples_to_average == 0)
  {
    telemetry[telemetry_index] = analogRead(ANALOG_TEMP);
    telemetry_index++;
  }
  else
  {
    // collect samples
    for (uint8_t i = 0; i < samples_to_average; i++)
    {
      sum_of_samples += analogRead(ANALOG_TEMP);
      delay(10);
    }
    telemetry[telemetry_index] = sum_of_samples / samples_to_average;
    telemetry_index++;
    sum_of_samples = 0;
  }

  // if led on
  if (use_led)
  {
    blink_led();
  }
}

void check_button()
{
  // Check if the button is pressed
  if (digitalRead(IN_BUTTON_PRESS) == HIGH)
  {
    // Button is pressed
    if (!buttonPressed)
    {
      // Record the button press start time
      buttonPressStartTime = millis();
      buttonPressed = true;
    }

    // Check if the button has been held for at least 3 seconds
    if (millis() - buttonPressStartTime >= 3000 && millis())
    {

      blink_led();
      // Set OUT_ENABLE pin low
      digitalWrite(OUT_ENABLE, LOW);

      // Enter an infinite loop (you can add your specific behavior here)
      while (true)
      {
        // Your code to run in the infinite loop
      }
    }
  }
  else
  {
    // Button is released, reset the buttonPressed flag
    buttonPressed = false;
  }
}

void serial_handler()
{
  // Check if data is available to read from the serial port
  if (Serial.available())
  {
    // allocate memory for the incoming data
    int c = Serial.read();
    unsigned long start_time = millis();
    while (c >= 0 && c != '\n' && c != '\r' && strlen(incomingData) < MAX_BUFFER_SIZE - 1 && millis() - start_time < SERIAL_TIMEOUT)
    {
      incomingData[strlen(incomingData)] = c;
      c = Serial.read();
    }

    if (strlen(incomingData) > 0)
    {
      command_handler(incomingData);
    }

    // reset the buffer
    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++)
      incomingData[i] = '\0';
  }
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
  {
    serial_handler();
    int c = Serial.read();
    unsigned long start_time = millis();
    while (c >= 0 && c != '\n' && c != '\r' && strlen(incomingData) < MAX_BUFFER_SIZE - 1 && millis() - start_time < SERIAL_TIMEOUT)
    {
      incomingData[strlen(incomingData)] = c;
      c = Serial.read();
    }

    if (strlen(incomingData) > 0)
    {
      command_handler(incomingData);
    }

    // reset the buffer
    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++)
      incomingData[i] = '\0';
  }
}
