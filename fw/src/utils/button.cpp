#include <Arduino.h>
#include "button.h"
#include "globals.h"
#include "led.h"

void check_button()
{
    // Check if the button is pressed
    if (digitalRead(PIN_IN_BUTTON_PRESS) == HIGH)
    {
        // Button is pressed
        if (!is_button_pressed)
        {
            // Record the button press start time
            time_button_press_start = millis();
            is_button_pressed = true;
        }

        // Check if the button has been held for at least 3 seconds
        if (millis() - time_button_press_start >= 3000 && millis())
        {

            blink_led();
            // Set OUT_ENABLE pin low
            digitalWrite(PIN_OUT_ENABLE, LOW);

            // Enter an infinite loop (you can add your specific behavior here)
            while (true)
            {
            }
        }
    }
    else
    {
        // Button is released early, reset the button press start time
        is_button_pressed = false;
    }
}
