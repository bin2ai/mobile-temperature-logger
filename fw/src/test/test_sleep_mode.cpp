#define TEST
#ifdef TEST
#include <Arduino.h>
#include <LowPower.h>
#include <EEPROM.h>

enum state_t
{
    STATE_IDLE,               // occurs when no telemetry is being collected
    STATE_COLLECTING,         // occurs when telemtry collection is started by user
    STATE_TELM_PARITALY_FULL, // only occurs is telmetry collection started but not finished, set during setup
    STATE_TELM_FULL           // occurs when telmetry is full, set during setup and during collection
};
state_t state = STATE_IDLE;

bool is_usb_connected = false;
int vbus_theshold = 4.5; //~ low bar for usb connection, about ~4.5V (adjusted for on-card voltage divider and tolerance)
uint8_t PINI_CHARGING_L = 2;
uint8_t PINI_STANDBY_L = 0;
uint8_t PINI_BUTTON = 3;
uint8_t PINO_ENABLE = 1;
uint8_t PINO_LED_L = 11;
uint8_t PINA_VBUS = A3;
uint8_t PINA_VBAT = A4;
uint8_t PINA_TEMP = A5;

uint16_t index_temp_telm = 0; // 0-1023
uint8_t interval_collection_seconds = 1;
// telemetry collection start time in seconds
uint32_t time_collection_start = 0;
uint16_t SIZE_TELMETRY = 500;

const uint8_t SIZE_OF_MESSAGE = 100;

uint16_t TIME_LIMIT_BUTTON_LOWER = 3000;
uint16_t TIME_LIMIT_BUTTON_UPPER = 5000;
// last button press time
unsigned long time_button_press = 0;
// button pressed
bool is_button_pressed = false;
bool button_press_init = false;

uint8_t samples_to_average = 64; // 1-64, CANNOT BE less than 1 or greater than 64

bool is_interval_set = false;
uint32_t utc = 0;

// EEPROM offests
uint8_t OFFSET_EEPROM_INDEX = 0;      // size 2 bytes
uint8_t OFFSET_EEPROM_UTC = 2;        // size 4 bytes
uint8_t OFFSET_EEPROM_INTERVAL = 6;   // size 1 byte
uint8_t OFFSET_EEPROM_ID = 7;         // size 1 byte
uint8_t OFFSET_EEPROM_UID = 8;        // size 1 byte
uint8_t OFFSET_EEPROM_TELEMETRY = 10; // size 1000 bytes

uint8_t id = 0;  // user defined id, can be set multiple times
uint8_t uid = 0; // hardware unique id, should only be set once

void death_loop()
{
    while (1)
        ;
}

void pretty_print(const char *fmt, ...)
{
    /*
     * Print a formatted string to the serial port.
     */

    char message[SIZE_OF_MESSAGE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, SIZE_OF_MESSAGE, fmt, args);
    va_end(args);
    Serial.print(message);
    Serial.flush();
}

float get_vbus()
{
    uint16_t sum_of_samples = 0;
    for (uint8_t i = 0; i < samples_to_average; i++)
    {
        sum_of_samples += analogRead(PINA_TEMP);
        delay(5);
    }

    return (float)sum_of_samples / samples_to_average * 3.3 * 2 / 1023.0;
}

float get_vbat()
{
    uint16_t sum_of_samples = 0;
    for (uint8_t i = 0; i < samples_to_average; i++)
    {
        sum_of_samples += analogRead(PINA_VBAT);
        delay(5);
    }
    return (float)sum_of_samples / samples_to_average * 3.3 * 8.5 / 1023.0;
}

float get_temperature(uint16_t bit_value = 0)
{
    /*
    P/N: SDNT2012X104F4250FTF
    R25 = 100k
    B25 = 4250 (@25C-50C)
    K_A = 2000 kelvin to fahrenheit
    K_B = 77 kelvin to fahrenheit
    K_C = 5963 kelvin to fahrenheit
    K_D = 45967 kelvin to fahrenheit
    Rt =  R25 * exp((-K_A*B25*(Temp_F-K_B))/(K_C*100*Temp_F+K_D))
    Temp_F = (K_A*B25*K_B+K_C*100*log(Rt/R25))/(K_C*100-K_A*B25)

    eq: Rt = Vt/(Vt/R1-Vs/R1-Vt/R2)
     * Rt = thermistor resistance
     * Vt = thermistor voltage
     * Vs = supply voltage
     * R1 = 10k
     * R2 = 100k
     *
     */

    float voltage = 0.0;
    if (bit_value == 0)
        voltage = (float)analogRead(PINA_TEMP) * 3.3 / 1023.0;
    else
        voltage = (float)bit_value * 3.3 / 1023.0;

    float resistance = voltage / (voltage / 10000 - 3.3 / 10000 - voltage / 100000);
    double numerator = (2000.0L * 4250.0L * 77.0L - log(resistance / 100000.0L) * 5963.0L * 45967.0L);
    double denominator = (log(resistance / 100000.0L) * 5963.0L * 100.0L + 2000.0L * 4250.0L);
    // Serial Print resistance, numerator, denominator
    Serial.println(voltage);
    Serial.println(resistance);
    Serial.println(numerator);
    Serial.println(denominator);

    return (float)(numerator / denominator);
}

void blink_led(uint8_t times = 2)
{
    for (uint8_t i = 0; i < times; i++)
    {
        digitalWrite(PINO_LED_L, LOW);
        delay(50);
        digitalWrite(PINO_LED_L, HIGH);
        delay(50);
    }
}

void check_usb()
{

    if (get_vbus() > vbus_theshold)
    {
        if (!is_usb_connected)
        {
            // attach USB
            USBDevice.attach();
            blink_led(3);
        }
        is_usb_connected = true;
    }
    else
    {
        if (is_usb_connected)
        {
            // detach USB
            USBDevice.detach();
            blink_led(3);
        }
        is_usb_connected = false;
    }
}

// check button press, func
void check_button_press()
{
    if (digitalRead(PINI_BUTTON) == HIGH)
    {
        if (button_press_init == false)
        {
            is_button_pressed = true;
            button_press_init = true;
            time_button_press = millis();
        }
    }
    // check if button was pressed and released, or just low
    else
    {
        if (is_button_pressed == true)
        {

            uint32_t button_press_time = millis() - time_button_press;
            pretty_print("Button pressed for %d ms, shutting down\n", button_press_time);
            if (button_press_time > TIME_LIMIT_BUTTON_LOWER && button_press_time < TIME_LIMIT_BUTTON_UPPER)
            {
                // ignore if USB is connected
                if (!is_usb_connected)
                {
                    blink_led(3);
                    delay(1000);
                    USBDevice.detach();
                    digitalWrite(PINO_ENABLE, LOW);
                    death_loop(); // just in case PINO_ENABLE fails
                }
                else
                {
                    // ignore
                    ;
                }
            }
        }
        button_press_init = false;
        is_button_pressed = false;
    }
}

uint16_t get_temp()
{
    uint16_t sum_of_samples = 0;
    for (uint8_t i = 0; i < samples_to_average; i++)
    {
        sum_of_samples += analogRead(PINA_TEMP);
        delay(5);
    }
    uint16_t average = sum_of_samples / samples_to_average;
    return average;
}

void setup()
{
    // Initialize serial communication
    Serial.begin(9600);
    Serial.setTimeout(1000);

    // SET AS OUTPUT HIGH Z
    pinMode(PINO_ENABLE, OUTPUT);
    digitalWrite(PINO_ENABLE, HIGH);
    pinMode(PINI_CHARGING_L, INPUT_PULLUP);
    pinMode(PINI_STANDBY_L, INPUT_PULLUP);
    pinMode(PINO_LED_L, OUTPUT);
    digitalWrite(PINO_LED_L, HIGH);

    // read first 2 bytes of eeprom for index
    index_temp_telm = EEPROM.read(OFFSET_EEPROM_INDEX + 0) << 8 | EEPROM.read(OFFSET_EEPROM_INDEX + 1);

    // if index is greater than 499, then telemetry is full
    if (index_temp_telm > SIZE_TELMETRY - 1)
        state = STATE_TELM_FULL;
    else if (index_temp_telm > 0)
        state = STATE_TELM_PARITALY_FULL;
    else
        state = STATE_IDLE;

    check_usb();
    delay(1000);
}

void loop()
{

    check_usb();
    check_button_press();
    // if is not connected go to sleep for (1 or 8) seconds
    if (!is_usb_connected && state == STATE_COLLECTING)
    {
        // #blink 5 times
        blink_led(5);
        if (interval_collection_seconds < 8)
            LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
        else
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }

    // check serial for string "something\r\n"
    if (Serial.available() > 0)
    {
        blink_led(2);
        String input = Serial.readStringUntil('\n');
        if (input.startsWith("STATUS?"))
        {
            // STATUS?STATE
            String suffix = input.substring(7);
            if (suffix == "STATE")
                Serial.println(state);
            else if (suffix == "VBUS")
                Serial.println(get_vbus());
            else if (suffix == "VBAT")
                Serial.println(get_vbat());
            else if (suffix == "TEMP")
                Serial.println(get_temp());
            else if (suffix == "CHARGING_L")
                Serial.println(digitalRead(PINI_CHARGING_L));
            else if (suffix == "STANDBY_L")
                Serial.println(digitalRead(PINI_STANDBY_L));
            else if (suffix == "ENABLE")
                Serial.println(digitalRead(PINO_ENABLE));
            else if (suffix == "INDEX")
                Serial.println(index_temp_telm);
            else if (suffix == "INTERVAL")
                Serial.println(interval_collection_seconds);
            else if (suffix == "UTC")
                Serial.println(utc);
            else
                Serial.println("ERROR");
        }
        else if (input == "CLEAR")
        {
            // is collecting, send error
            if (state == STATE_COLLECTING or state == STATE_IDLE)
            {
                Serial.println("ERROR");
                return;
            }
            // clear utc variable and eeprom
            utc = 0;
            EEPROM.write(OFFSET_EEPROM_UTC + 0, 0);
            EEPROM.write(OFFSET_EEPROM_UTC + 1, 0);
            EEPROM.write(OFFSET_EEPROM_UTC + 2, 0);
            EEPROM.write(OFFSET_EEPROM_UTC + 3, 0);
            // clear index variable and eeprom
            index_temp_telm = 0;
            EEPROM.write(OFFSET_EEPROM_INDEX + 0, 0);
            EEPROM.write(OFFSET_EEPROM_INDEX + 1, 0);
            // clear telemetry and eeprom
            for (uint16_t i = 0; i < SIZE_TELMETRY; i += 2)
            {
                EEPROM.write(OFFSET_EEPROM_TELEMETRY + i, 0);
                EEPROM.write(OFFSET_EEPROM_TELEMETRY + i + 1, 0);
            }
            // interval
            interval_collection_seconds = 0;
            EEPROM.write(OFFSET_EEPROM_INTERVAL, 0);

            Serial.println("OK");
            // reset index
            index_temp_telm = 0;
            // reset state
            state = STATE_IDLE;
        }
        else if (input == "ID?")
        {
            id = EEPROM.read(OFFSET_EEPROM_ID);
            Serial.println(id);
        }
        else if (input.startsWith("ID="))
        {
            // if state is not idle, send error
            if (state != STATE_IDLE)
            {
                Serial.println("ERROR");
                return;
            }
            // set id
            uint8_t startPos = input.indexOf("=") + 1;
            uint8_t endPos = input.indexOf("\r\n");
            id = input.substring(startPos, endPos).toInt();
            if (id < 0 or id > UINT8_MAX)
            {
                Serial.println("ERROR");
                return;
            }
            // write to eeprom
            EEPROM.write(OFFSET_EEPROM_ID, id);
            Serial.println("OK");
        }
        else if (input == "UID?")
        {
            uid = EEPROM.read(OFFSET_EEPROM_UID);
            Serial.println(uid);
        }
        else if (input.startsWith("UID="))
        {
            // if state is not idle, send error
            if (state != STATE_IDLE)
            {
                Serial.println("ERROR");
                return;
            }
            // set id
            uint8_t startPos = input.indexOf("=") + 1;
            uint8_t endPos = input.indexOf("\r\n");
            uid = input.substring(startPos, endPos).toInt();
            if (uid < 0 or uid > UINT8_MAX)
            {
                Serial.println("ERROR");
                return;
            }
            // write to eeprom
            EEPROM.write(OFFSET_EEPROM_UID, uid);
            Serial.println("OK");
        }
        // update UTC
        else if (input.startsWith("UTC="))
        {
            // if state is not idle, send error
            if (state != STATE_IDLE)
            {
                Serial.println("ERROR");
                return;
            }
            // set UTC
            uint8_t startPos = input.indexOf("=") + 1;
            uint8_t endPos = input.indexOf("\r\n");
            utc = input.substring(startPos, endPos).toInt();
            if (utc < 0 or utc > UINT32_MAX - 1)
            {
                Serial.println("ERROR");
                return;
            }
            // write to eeprom
            EEPROM.write(OFFSET_EEPROM_UTC + 0, (utc >> 24) & 0xFF);
            EEPROM.write(OFFSET_EEPROM_UTC + 1, (utc >> 16) & 0xFF);
            EEPROM.write(OFFSET_EEPROM_UTC + 2, (utc >> 8) & 0xFF);
            EEPROM.write(OFFSET_EEPROM_UTC + 3, utc & 0xFF);
            Serial.println("OK");
        }
        // get UTC
        else if (input == "UTC?")
        {
            utc = 0;
            utc = static_cast<uint32_t>(EEPROM.read(OFFSET_EEPROM_UTC + 0)) << 24;
            utc |= static_cast<uint32_t>(EEPROM.read(OFFSET_EEPROM_UTC + 1)) << 16;
            utc |= static_cast<uint32_t>(EEPROM.read(OFFSET_EEPROM_UTC + 2)) << 8;
            utc |= static_cast<uint32_t>(EEPROM.read(OFFSET_EEPROM_UTC + 3));
            Serial.println(utc);
        }
        // set interval
        else if (input.startsWith("INTERVAL="))
        {
            // if state is not idle, send error
            if (state != STATE_IDLE)
            {
                Serial.println("ERROR");
                return;
            }
            // set interval
            uint8_t startPos = input.indexOf("=") + 1;
            uint8_t endPos = input.indexOf("\r\n");
            interval_collection_seconds = input.substring(startPos, endPos).toInt();

            // interval_seconds x SIZE_TELMETRY <= 10 hours (i.e. 90x500/60/60 = 12.5)
            if (interval_collection_seconds < 1 or interval_collection_seconds > 90)
            {
                Serial.println("ERROR");
                return;
            }
            interval_collection_seconds = interval_collection_seconds;
            EEPROM.write(OFFSET_EEPROM_INTERVAL, interval_collection_seconds);
            is_interval_set = true;
            Serial.println("OK");
        }
        else if (input == "INTERVAL?")
        {
            interval_collection_seconds = EEPROM.read(OFFSET_EEPROM_INTERVAL);
            Serial.println(interval_collection_seconds);
        }
        else if (input == "START")
        {
            // if index not 0, then telemetry is might be full send error
            if (index_temp_telm > 0 or !is_interval_set or state != STATE_IDLE or (digitalRead(PINI_STANDBY_L) == HIGH and get_vbat() < 4.0))
            {
                Serial.println("ERROR");
                return;
            }
            // start telemetry collection
            Serial.println("OK");
            state = STATE_COLLECTING;
            time_collection_start = millis();
        }
        else if (input == "STOP")
        {
            // if state is not collecting, send error
            if (state != STATE_COLLECTING)
            {
                Serial.println("ERROR");
                return;
            }
            // stop telemetry collection
            Serial.println("STOP");
            state = STATE_TELM_PARITALY_FULL;
        }
        // check if string starts with DUMP
        else if (input.startsWith("DUMP"))
        {
            // dump can only occur in state idle
            if (state != STATE_TELM_PARITALY_FULL and state != STATE_TELM_FULL)
            {
                Serial.println("ERROR");
                return;
            }
            uint8_t startPos = input.indexOf("DUMP");
            uint8_t endPos = input.indexOf("\n");
            // get length of numbered string
            uint8_t length = endPos - startPos;
            // get index number, i.e. DUMP100, index = 100, or DUMP24, index = 24
            uint16_t index = input.substring(startPos + 4, length).toInt();
            // check if index number follows DUMP
            if (index < SIZE_TELMETRY)
            {
                index = index * 2;
                // assume each telemetry is 2 bytes, concat before printing
                uint16_t telemetry = (EEPROM.read(index + OFFSET_EEPROM_TELEMETRY) << 8) | EEPROM.read(index + OFFSET_EEPROM_TELEMETRY + 1);
                Serial.println(telemetry);
            }
            else
            {
                // index is greater than SIZE_TELMETRY
                Serial.println("ERROR");
            }
        }
        // flush
        else
            Serial.println("ERROR");
        Serial.flush();
    }
    // if state collecting, collect temperature telemetry, i
    if (state == STATE_COLLECTING)
    {
        // if telemetry is full, go into full state
        if (index_temp_telm > SIZE_TELMETRY - 1)
        {
            state = STATE_TELM_FULL;
            return;
        }

        // if time is up, collect telemetry
        if (millis() - time_collection_start >= interval_collection_seconds * 1000)
        {
            uint16_t temp = get_temp();
            // write to EEPROM
            EEPROM.write(index_temp_telm + OFFSET_EEPROM_TELEMETRY, temp >> 8);
            EEPROM.write(index_temp_telm + OFFSET_EEPROM_TELEMETRY + 1, temp & 0xFF);

            // increment index
            index_temp_telm += 2;
            time_collection_start = millis();
            blink_led(1);
        }
    }
}
#endif // TEST