#include <Arduino.h>
#include "globals.h"
#include "led.h"

void blink_led()
{
    digitalWrite(PIN_OUT_LED_L, LOW);
    delay(50);
    digitalWrite(PIN_OUT_LED_L, HIGH);
    delay(50);
    digitalWrite(PIN_OUT_LED_L, LOW);
    delay(50);
    digitalWrite(PIN_OUT_LED_L, HIGH);
}
