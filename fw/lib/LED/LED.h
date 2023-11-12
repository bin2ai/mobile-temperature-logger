#ifndef LED_H
#define LED_H

#include <stdint.h>

enum class LEDRegisterMap
{
    REG_RSTR = 0x00,
    REG_GCR = 0x01,
    REG_ISR = 0x02,
    REG_FCTR = 0x03,
    REG_LCTR = 0x30,
    REG_LCFG0 = 0x31,
    REG_LCFG1 = 0x32,
    REG_LCFG2 = 0x33,
    REG_PWM0 = 0x34,
    REG_PWM1 = 0x35,
    REG_PWM2 = 0x36,
    REG_LED0T0 = 0x37,
    REG_LED0T1 = 0x38,
    REG_LED0T2 = 0x39,
    REG_LED1T0 = 0x3A,
    REG_LED1T1 = 0x3B,
    REG_LED1T2 = 0x3C,
    REG_LED2T0 = 0x3D,
    REG_LED2T1 = 0x3E,
    REG_LED2T2 = 0x3F,
    REG_LEDSYNC = 0x4A
};

enum class FadeTime
{
    // used for T1 and T3
    FT_SEC_0P13 = 0b000,   // 0.13s
    FIOT_SEC_0P26 = 0b001, // 0.26s
    FIOT_SEC_0P52 = 0b010, // 0.52s
    FIOT_SEC_1P04 = 0b011, // 1.04s
    FIOT_SEC_2P08 = 0b100, // 2.08s
    FIOT_SEC_4P16 = 0b101, // 4.16s
    FIOT_SEC_8P32 = 0b110, // 8.32s
    FIOT_SEC_16P64 = 0b111 // 16.64s
};
enum class HoldAfterFadeInTime
{
    // use for T2
    HAFIT_SEC_0P13 = 0b000, // 0.13s
    HAFIT_SEC_0P26 = 0b001, // 0.26s
    HAFIT_SEC_0P52 = 0b010, // 0.52s
    HAFIT_SEC_1P04 = 0b011, // 1.04s
    HAFIT_SEC_2P08 = 0b100, // 2.08s
    HAFIT_SEC_4P16 = 0b101, // 4.16s
                            // all other values are still 4.16s
};

enum class HoldAfterFadeOutTime
{
    HAFOT_SEC_0P13 = 0b000, // 0.13s
    HAFOT_SEC_0P26 = 0b001, // 0.26s
    HAFOT_SEC_0P52 = 0b010, // 0.52s
    HAFOT_SEC_1P04 = 0b011, // 1.04s
    HAFOT_SEC_2P08 = 0b100, // 2.08s
    HAFOT_SEC_4P16 = 0b101, // 4.16s
    HAFOT_SEC_8P32 = 0b110, // 8.32s
    HAFOT_SEC_16P64 = 0b111 // 16.64s
};

enum class DelayBeforeBlinkTime
{
    // used for T4
    DBBT_SEC_0P00 = 0b0000,  // 0.00s
    DBBT_SEC_0P13 = 0b0001,  // 0.13s
    DABT_SEC_0P26 = 0b0010,  // 0.26s
    DABT_SEC_0P52 = 0b0011,  // 0.52s
    DBBT_SEC_1P04 = 0b0100,  // 1.04s
    DBBT_SEC_2P08 = 0b0101,  // 2.08s
    DBBT_SEC_4P16 = 0b0110,  // 4.16s
    DBBT_SEC_8P32 = 0b0111,  // 8.32s
    DBBT_SEC_16P64 = 0b1000, // 16.64s
                             // all other values are still 16.64s

};

enum class BinkTimes
{
    CONTINOUS = 0b0000,
    TIMES_1 = 0b0001,
    TIMES_2 = 0b0010,
    TIMES_3 = 0b0011,
    TIMES_4 = 0b0100,
    TIMES_5 = 0b0101,
    TIMES_6 = 0b0110,
    TIMES_7 = 0b0111,
    TIMES_8 = 0b1000,
    TIMES_9 = 0b1001,
    TIMES_10 = 0b1010,
    TIMES_11 = 0b1011,
    TIMES_12 = 0b1100,
    TIMES_13 = 0b1101,
    TIMES_14 = 0b1110,
    TIMES_15 = 0b1111
};

enum class Brightness
{
    BRIGHTNESS_OFF = 0,
    BRIGHTNESS_LOW = 32,
    BRIGHTNESS_MED = 64,
    BRIGHHTNESS_HI = 127
};

enum class Color
{
    OFF,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    YELLOW,
    WHITE
};

enum class IMax
{
    I_MAX_0mA = 0b000,
    I_MAX_5mA = 0b001,
    I_MAX_10mA = 0b010,
    I_MAX_15mA = 0b011,
    I_MAX_25mA = 0b100
    // all other values are still 20mA
};

class WD3153
{
public:
    WD3153(uint8_t address_i2c, Color color = Color::OFF, Brightness brightness = Brightness::BRIGHTNESS_OFF);

    void setup();
    void setBrightness(Brightness brightness);
    void setColor(Color color);
    void blink(Color color, Brightness brightness, BinkTimes blink_times, DelayBeforeBlinkTime delay_after_blink_time);
    uint8_t readRegister(uint8_t address);

private:
    uint8_t _is_setup;
    uint8_t _address_i2c;
    Brightness _brightness;
    Color _color;
    FadeTime _fade_in_out_time;
    HoldAfterFadeInTime _hold_after_fade_in_time;
    HoldAfterFadeOutTime _hold_after_fade_out_time;
    bool _r;
    bool _g;
    bool _b;

    void _writeControlRegister(uint8_t data, uint8_t address);
    void _setRGB(bool r, bool g, bool b);
    void _set_is_setup(bool is_setup);
    bool _get_is_setup();
    // 0x00
    void _set_reg_rstr(uint8_t rst = 0x55);
    // 0x01
    void _set_reg_gcr(bool enable, bool lie0 = false, bool lie1 = false, bool lie2 = false, bool uvlo_ie = false, bool otp_ie = false);
    // 0x02
    void _set_reg_isr(bool lis2_is = false, bool lis1_is = false, bool lis0_is = false, bool puis_is = false, bool uvlo_is = false, bool otp_is = false);
    // 0x03
    void _set_reg_fctr(bool opt_en = false, bool uvlo_en = false, bool chrg_en_l = false, bool pwm_hf = false, bool log_en = false);
    // 0x30
    void _set_reg_lctr(bool le2 = false, bool le1 = false, bool le0 = false);
    // 0x31, 0x32, 0x33
    void _set_reg_lcfg(bool fo = false, bool fi = false, bool md = false, IMax imax = IMax::I_MAX_0mA, bool LED0 = true, bool LED1 = true, bool LED2 = true);
    // 0x34, 0x35, 0x36
    void _set_reg_pwm(uint8_t pwm, bool LED0 = true, bool LED1 = true, bool LED2 = true);
    // 0x37, 0x3A, 0x3D
    void _set_reg_lediT0(FadeTime fade_in_time = FadeTime::FT_SEC_0P13, HoldAfterFadeInTime hold_after_fade_in_time = HoldAfterFadeInTime::HAFIT_SEC_0P26, bool LED0 = true, bool LED1 = true, bool LED2 = true);
    // 0x38, 0x3B, 0x3E
    void _set_reg_lediT1(FadeTime fade_out_time = FadeTime::FT_SEC_0P13, HoldAfterFadeOutTime hold_after_fade_out_time = HoldAfterFadeOutTime::HAFOT_SEC_0P26, bool LED0 = true, bool LED1 = true, bool LED2 = true);
    // 0x39, 0x3C, 0x3F
    void _set_reg_lediT2(DelayBeforeBlinkTime delay_after_blink_time = DelayBeforeBlinkTime::DBBT_SEC_0P00, BinkTimes blink_times = BinkTimes::CONTINOUS, bool LED0 = true, bool LED1 = true, bool LED2 = true);
    // 0x4A
    void _set_reg_ledsync(bool sync_en = false);
    // skip reg iadr (0x77)
};

extern WD3153 LED;

#endif