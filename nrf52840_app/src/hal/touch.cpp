#include "touch.h"
#include "../../include/config.h"
#include <Arduino.h>
#include <Wire.h>

// CST816S I2C address
#define CST816S_ADDR       0x15

// Register map
#define REG_GESTURE        0x01
#define REG_FINGER_NUM     0x02
#define REG_XPOS_H         0x03
#define REG_XPOS_L         0x04
#define REG_YPOS_H         0x05
#define REG_YPOS_L         0x06
#define REG_CHIP_ID        0xA7
#define REG_MOTION_MASK    0xEC   // enable continuous gesture reporting
#define REG_IRQ_CTL        0xFA   // interrupt control

namespace 
{
    bool            s_touched  = false;
    touch::Point    s_point    = {};
    touch::Point    s_start    = {};
    touch::Gesture  s_gesture  = touch::Gesture::NONE;
    bool            s_was_touched = false;
    static volatile bool s_int_fired = false;

    void write_reg(uint8_t reg, uint8_t val) 
    {
        Wire.beginTransmission(CST816S_ADDR);
        Wire.write(reg);
        Wire.write(val);
        Wire.endTransmission();
    }

    uint8_t read_reg(uint8_t reg) 
    {
        Wire.beginTransmission(CST816S_ADDR);
        Wire.write(reg);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)CST816S_ADDR, (uint8_t)1);
        return Wire.available() ? Wire.read() : 0;
    }
}

namespace touch 
{

    void init() 
    {
        // Hardware reset
        pinMode(PIN_TP_RST, OUTPUT);
        digitalWrite(PIN_TP_RST, LOW);
        delay(10);
        digitalWrite(PIN_TP_RST, HIGH);
        delay(50);

        attachInterrupt(digitalPinToInterrupt(PIN_TP_INT), touch_isr, FALLING); 

        Wire.begin();
        Wire.setClock(400000);  // 400kHz fast mode

        // Enable continuous gesture output so swipes fire reliably
        write_reg(REG_MOTION_MASK, 0x03);   // bit0=EnDClick, bit1=EnConTinuous
        // Interrupt fires on touch/gesture event (EnTouch=bit4) + once per wakeup (OnceWLP=bit0)
        write_reg(REG_IRQ_CTL, 0x11);
    }

    void touch_isr()
    {
        s_int_fired = true;
    }

    void poll() 
    {
        if (!s_int_fired)
        {
            // No new touch event since last poll
            s_touched = false;
            s_gesture = Gesture::NONE;
            return;
        }
        s_int_fired = false;

        Wire.beginTransmission(CST816S_ADDR);
        Wire.write(REG_GESTURE);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)CST816S_ADDR, (uint8_t)6);

        if (Wire.available() < 6) 
        {
            s_touched = false;
            return;
        }

        uint8_t gest     = Wire.read();  // 0x01 gesture
        uint8_t fingers  = Wire.read();  // 0x02 finger count
        uint8_t xh       = Wire.read();  // 0x03 X high
        uint8_t xl       = Wire.read();  // 0x04 X low
        uint8_t yh       = Wire.read();  // 0x05 Y high
        uint8_t yl       = Wire.read();  // 0x06 Y low

        s_touched = (fingers > 0);
        s_gesture = (Gesture)gest;
        s_point.x = (int16_t)(((xh & 0x0F) << 8) | xl);
        s_point.y = (int16_t)(((yh & 0x0F) << 8) | yl);

        if (!s_was_touched && s_touched)
        {
            s_start = s_point;
            s_was_touched = true;
        }
        else if (s_was_touched && !s_touched)
        {
            s_was_touched = false;
        }
    }

    bool     touched() { return s_touched; }
    Point    point()   { return s_point;   }
    Point start_point() { return s_start; }
    Gesture  gesture() { return s_gesture; }
}