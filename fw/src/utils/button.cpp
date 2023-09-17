#include <Arduino.h>
#include "button.h"
#include "globals.h"
#include "led.h"

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
