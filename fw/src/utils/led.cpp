#include <Arduino.h>
#include "globals.h"
#include "led.h"

void blink_led(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        digitalWrite(PIN_OUT_LED_L, LOW);
        delay(50);
        digitalWrite(PIN_OUT_LED_L, HIGH);
        delay(50);
    }
}
