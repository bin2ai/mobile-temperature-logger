#include "LED.h"
#include <Wire.h>
#include <Arduino.h>

WD3153::WD3153(uint8_t address_i2c, Color color, Brightness brightness)
{
    this->_address_i2c = address_i2c;
    this->_color = color;
    this->_brightness = brightness;
    this->_is_setup = false;
    this->_r = false;
    this->_g = false;
    this->_b = false;

    this->_fade_in_out_time = FadeTime::FIOT_SEC_0P26;
    this->_hold_after_fade_in_time = HoldAfterFadeInTime::HAFIT_SEC_0P52;
    this->_hold_after_fade_out_time = HoldAfterFadeOutTime::HAFOT_SEC_0P52;
}

void WD3153::setup()
{
    if (_get_is_setup())
        return;
    uint8_t reg_val = 0;

    _set_reg_rstr(0x55); // reset all registers
    _set_reg_gcr(true);
    _set_reg_isr(false, false, false, false, false, false);
    _set_reg_fctr(false, false, false, false, false);
    _set_reg_lctr(false, false, false);
    _set_reg_lcfg(true, true, true, IMax::I_MAX_5mA, true, true, true);
    _set_reg_pwm((uint8_t)Brightness::BRIGHTNESS_LOW);
    _set_reg_lediT0(FadeTime::FIOT_SEC_0P26, HoldAfterFadeInTime::HAFIT_SEC_0P52, true, true, true);
    _set_reg_lediT1(FadeTime::FIOT_SEC_0P26, HoldAfterFadeOutTime::HAFOT_SEC_0P52, true, true, true);
    _set_reg_lediT2(DelayBeforeBlinkTime::DABT_SEC_0P26, BinkTimes::TIMES_3, true, true, true);
    _set_reg_ledsync(true);

    delay(10);

    Serial.println(reg_val, BIN);

    _set_is_setup(true);

    setBrightness(Brightness::BRIGHTNESS_OFF);
    setColor(Color::OFF);
}

void WD3153::setBrightness(Brightness brightness)
{
    if (this->_get_is_setup() == false)
        return;
    if (this->_brightness == brightness)
        return;

    this->_brightness = brightness;
}

void WD3153::setColor(Color color)
{
    if (this->_get_is_setup() == false)
        return;

    if (this->_color == color)
        return;

    this->_color = color;
    switch (color)
    {
    case Color::OFF:
        _setRGB(0, 0, 0);
        break;
    case Color::BLUE:
        _setRGB(0, 0, 1);
        break;
    case Color::GREEN:
        _setRGB(0, 1, 0);
        break;
    case Color::CYAN:
        _setRGB(0, 1, 1);
        break;
    case Color::RED:
        _setRGB(1, 0, 0);
        break;
    case Color::MAGENTA:
        _setRGB(1, 0, 1);
        break;
    case Color::YELLOW:
        _setRGB(1, 1, 0);
        break;
    case Color::WHITE:
        _setRGB(1, 1, 1);
        break;
    default:
        break;
    }
}

void WD3153::blink(Color color, Brightness brightness, BinkTimes blink_times, DelayBeforeBlinkTime delay_before_blink_time)
{
    if (this->_get_is_setup() == false)
        return;

    // uint8_t reg_val = 0;
    // // delay time enum are bits 7:4, blin time enum are bits 3:0
    // reg_val = (uint8_t)delay_after_blink_time << 4 | (uint8_t)blink_times;
    // _writeControlRegister(reg_val, 0x39);
    // _writeControlRegister(reg_val, 0x3C);
    // _writeControlRegister(reg_val, 0x3F);

    _set_reg_lcfg(true, true, true, IMax::I_MAX_5mA, true, true, true);
    _set_reg_pwm((uint8_t)brightness, true, true, true);
    _set_reg_lediT0(FadeTime::FT_SEC_0P13, HoldAfterFadeInTime::HAFIT_SEC_0P26, true, true, true);
    _set_reg_lediT1(FadeTime::FT_SEC_0P13, HoldAfterFadeOutTime::HAFOT_SEC_0P26, true, true, true);
    _set_reg_lediT2(delay_before_blink_time, blink_times, true, true, true);
    _set_reg_ledsync(true);
    setColor(color);
}

void WD3153::_setRGB(bool r, bool g, bool b)
{
    if (this->_get_is_setup() == false)
        return;

    this->_r = r;
    this->_g = g;
    this->_b = b;

    _set_reg_lctr(b, g, r);

    /*
    8 colors, 1 off
    r | g | b | color
    -----------------
    0 | 0 | 0 | off
    0 | 0 | 1 | blue
    0 | 1 | 0 | green
    0 | 1 | 1 | cyan
    1 | 0 | 0 | red
    1 | 0 | 1 | magenta
    1 | 1 | 0 | yellow
    1 | 1 | 1 | white
    */

    // _set_reg_lcfg(false, false, false, IMax::I_MAX_0mA, r, g, b);

    // if (r)

    //     _writeControlRegister((uint8_t)_brightness, 0x34);
    // else
    //     _writeControlRegister(0x00, 0x34);
    // if (g)
    //     _writeControlRegister((uint8_t)_brightness, 0x35);
    // else
    //     _writeControlRegister(0x00, 0x35);
    // if (b)
    //     _writeControlRegister((uint8_t)_brightness, 0x36);
    // else
    //     _writeControlRegister(0x00, 0x36);
}

uint8_t WD3153::readRegister(uint8_t address)
{
    if (this->_get_is_setup() == false)
        return 0;
    Wire.beginTransmission(_address_i2c);
    Wire.write(address);
    Wire.endTransmission(false);
    Wire.requestFrom(_address_i2c, (uint8_t)1);

    if (Wire.available())
    {
        uint8_t data = Wire.read();
        Wire.endTransmission();
        return data;
    }
    return 0; // Return a default value if there was an issue
}

void WD3153::_writeControlRegister(uint8_t data, uint8_t reg_address)
{
    Wire.beginTransmission(_address_i2c);
    Wire.write(reg_address);
    Wire.write(data);
    Wire.endTransmission();
}

//     void _set_is_setup(bool is_setup);

void WD3153::_set_is_setup(bool is_setup)
{
    this->_is_setup = is_setup;
}

bool WD3153::_get_is_setup()
{
    return this->_is_setup;
}

void WD3153::_set_reg_rstr(uint8_t rst)
{
    /*
    Chip ID: 0x33
    Reset: 0x55 - reset all registers to default values
    */
    _writeControlRegister(rst, (uint8_t)LEDRegisterMap::REG_RSTR);
}

void WD3153::_set_reg_gcr(bool enable, bool lie0, bool lie1, bool lie2, bool uvlo_ie, bool otp_ie)
{
    /*
    bit 7 - 0
    lie2, lie1, lie0, uvlo_ie, otp_ie, RSVD, RSVD, enable
    */
    uint8_t gcr = 0;
    gcr |= lie2 << 7;
    gcr |= lie1 << 6;
    gcr |= lie0 << 5;
    gcr |= uvlo_ie << 4;
    gcr |= otp_ie << 3;
    gcr |= enable << 0;
    // SET BITS 2-1 TO 0
    gcr &= ~(1 << 2);
    gcr &= ~(1 << 1);

    _writeControlRegister(gcr, (uint8_t)LEDRegisterMap::REG_GCR);
}

void WD3153::_set_reg_isr(bool lis2_is, bool lis1_is, bool lis0_is, bool puis_is, bool uvlo_is, bool otp_is)
{
    /*
    LIS2, LIS1, LIS0, PUIS, UVLO, OTP, RSVD, RSVD
    */
    uint8_t isr = 0;
    isr |= lis2_is << 7;
    isr |= lis1_is << 6;
    isr |= lis0_is << 5;
    isr |= puis_is << 4;
    isr |= uvlo_is << 3;
    isr |= otp_is << 2;
    // SET BITS 1-0 TO 0
    isr &= ~(1 << 1);
    isr &= ~(1 << 0);

    _writeControlRegister(isr, (uint8_t)LEDRegisterMap::REG_ISR);
}

// FCTR
void WD3153::_set_reg_fctr(bool opt_en, bool uvlo_en, bool chrg_en_l, bool pwm_hf, bool log_en)
{
    /*
    bit 7 - 0
    RSVD, RSVD, RSVD, OTP_EN, UVLO_EN, CHRG_EN_L, PWM_HF, LOG_EN
    */
    uint8_t fctr = 0;
    fctr |= opt_en << 4;
    fctr |= uvlo_en << 3;
    fctr |= chrg_en_l << 2;
    fctr |= pwm_hf << 1;
    fctr |= log_en << 0;
    // SET BITS 7-5 TO 0
    fctr &= ~(1 << 7);
    fctr &= ~(1 << 6);
    fctr &= ~(1 << 5);

    _writeControlRegister(fctr, (uint8_t)LEDRegisterMap::REG_FCTR);
}

void WD3153::_set_reg_lctr(bool le2, bool le1, bool le0)
{
    /*
    RSVD, RSVD, RSVD, RSVD, RSVD, RSVD, LE2, LE1, LE0
    */
    uint8_t lctr = 0;
    lctr |= le2 << 2;
    lctr |= le1 << 1;
    lctr |= le0 << 0;

    _writeControlRegister(lctr, (uint8_t)LEDRegisterMap::REG_LCTR);
}

void WD3153::_set_reg_lcfg(bool fo, bool fi, bool md, IMax imax, bool LED0, bool LED1, bool LED2)
{
    /*
    3 different registers for the 3 LEDs (0x31, 0x32, 0x33)
    bit 7 - 0
    RSVD, fo, fi, md, RSVD, imax (3 bits)
    */

    // BIT 7:0
    // RSVD, FO, FI, MD, 0, IMAX[2], IMAX[1], IMAX[0]
    /*
    if imax is 111, and all others are false, then
    */

    uint8_t lcfg = 0;
    lcfg |= (fo << 6);
    lcfg |= (fi << 5);
    lcfg |= (md << 4);
    lcfg |= (uint8_t)imax << 0;
    // set all other bits to 0 (bit 7 and 3 are reserved)
    lcfg &= ~(1 << 7);
    lcfg &= ~(1 << 3);

    if (LED0)
        _writeControlRegister(lcfg, (uint8_t)LEDRegisterMap::REG_LCFG0);
    if (LED1)
        _writeControlRegister(lcfg, (uint8_t)LEDRegisterMap::REG_LCFG1);
    if (LED2)
        _writeControlRegister(lcfg, (uint8_t)LEDRegisterMap::REG_LCFG2);
}

void WD3153::_set_reg_pwm(uint8_t pwm, bool LED0, bool LED1, bool LED2)
{
    if (LED0)
        _writeControlRegister(pwm, (uint8_t)LEDRegisterMap::REG_PWM0);
    if (LED1)
        _writeControlRegister(pwm, (uint8_t)LEDRegisterMap::REG_PWM1);
    if (LED2)
        _writeControlRegister(pwm, (uint8_t)LEDRegisterMap::REG_PWM2);
}

void WD3153::_set_reg_lediT0(FadeTime fade_in_time, HoldAfterFadeInTime hold_after_fade_in_time, bool LED0, bool LED1, bool LED2)
{
    /*
    bit 7 - 0
    RSVD, FIOT (3 bits), RSVD, HAFIT (3 bits)
    */
    uint8_t ledit0 = 0;
    ledit0 |= (uint8_t)fade_in_time << 4;
    ledit0 |= (uint8_t)hold_after_fade_in_time << 0;
    // set all other bits to 0
    ledit0 &= ~(1 << 7);
    ledit0 &= ~(1 << 3);

    if (LED0)
        _writeControlRegister(ledit0, (uint8_t)LEDRegisterMap::REG_LED0T0);
    if (LED1)
        _writeControlRegister(ledit0, (uint8_t)LEDRegisterMap::REG_LED1T0);
    if (LED2)
        _writeControlRegister(ledit0, (uint8_t)LEDRegisterMap::REG_LED2T0);
}

void WD3153::_set_reg_lediT1(FadeTime fade_out_time, HoldAfterFadeOutTime hold_after_fade_out_time, bool LED0, bool LED1, bool LED2)
{
    /*
    bit 7 - 0
    RSVD, FIOT (3 bits), RSVD, HAFIT (3 bits)
    */
    uint8_t ledit1 = 0;
    ledit1 |= (uint8_t)fade_out_time << 4;
    ledit1 |= (uint8_t)hold_after_fade_out_time << 0;
    // set all other bits to 0
    ledit1 &= ~(1 << 7);
    ledit1 &= ~(1 << 3);

    if (LED0)
        _writeControlRegister(ledit1, (uint8_t)LEDRegisterMap::REG_LED0T1);
    if (LED1)
        _writeControlRegister(ledit1, (uint8_t)LEDRegisterMap::REG_LED1T1);
    if (LED2)
        _writeControlRegister(ledit1, (uint8_t)LEDRegisterMap::REG_LED2T1);
}

void WD3153::_set_reg_lediT2(DelayBeforeBlinkTime delay_after_blink_time, BinkTimes blink_times, bool LED0, bool LED1, bool LED2)
{
    uint8_t ledit2 = 0;
    ledit2 |= (uint8_t)delay_after_blink_time << 4;
    ledit2 |= (uint8_t)blink_times << 0;
    if (LED0)
        _writeControlRegister(ledit2, (uint8_t)LEDRegisterMap::REG_LED0T2);
    if (LED1)
        _writeControlRegister(ledit2, (uint8_t)LEDRegisterMap::REG_LED1T2);
    if (LED2)
        _writeControlRegister(ledit2, (uint8_t)LEDRegisterMap::REG_LED2T2);
}

void WD3153::_set_reg_ledsync(bool sync_en)
{
    uint8_t ledsync = 0;
    ledsync |= sync_en << 2;
    _writeControlRegister(ledsync, (uint8_t)LEDRegisterMap::REG_LEDSYNC);
}

WD3153 LED = WD3153(0x45, Color::OFF, Brightness::BRIGHTNESS_OFF);
