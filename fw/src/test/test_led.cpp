#include "Arduino.h"
#include "Wire.h"
#include "LED.h"

uint32_t start = 0;
uint32_t start_blink = 0;

void setup()
{
    Serial.begin(9600);
    Wire.begin();
    // flush wire
    Wire.flush();

    LED.setup();
}

void loop()
{
    // for color in Color, print color and set color
    // for (int i = 0; i < 8; i++)
    // {
    //     ;
    //     // Serial.println(i);
    //     // // LED.setColor((Color)i);
    //     // LED.blink((Color)i, Brightness::BRIGHTNESS_MED, BinkTimes::TIMES_2, DelayAfterBlinkTime::DABT_SEC_0P26);
    //     // delay(1000);
    // }
    // EVERY 5 SECONDS BLINK A NEW COLOR

    if (millis() - 5000 > start_blink)
    {
        start_blink = millis();
        LED.blink((Color)random(0, 8), Brightness::BRIGHTNESS_LOW, BinkTimes::TIMES_5, DelayBeforeBlinkTime::DBBT_SEC_0P00);
    }

    // if its been 10 seconds read all the LED registers
    if (millis() - 10000 > start)
    {
        start = millis();
        // use enum LEDRegisterMap
        uint8_t registerArray[] = {
            0x00, 0x01, 0x02, 0x03, 0x30, 0x31, 0x32, 0x33,
            0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
            0x3C, 0x3D, 0x3E, 0x3F, 0x4A};

        // Print the uint8_t array values
        for (uint8_t addr : registerArray)
        {
            Serial.print(addr, HEX);
            Serial.print(" ");
            Serial.println(LED.readRegister(addr), BIN);
        }
    }
}