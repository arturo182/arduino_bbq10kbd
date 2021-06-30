#include <Arduino.h>

#include "BBQ10Keyboard.h"

#define _REG_VER 0x01 // fw version
#define _REG_CFG 0x02 // config
#define _REG_INT 0x03 // interrupt status
#define _REG_KEY 0x04 // key status
#define _REG_BKL 0x05 // backlight
#define _REG_DEB 0x06 // debounce cfg
#define _REG_FRQ 0x07 // poll freq cfg
#define _REG_RST 0x08 // reset
#define _REG_FIF 0x09 // fifo
#define _REG_BK2 0x0A // backlight 2
#define _REG_DIR 0x0B // gpio direction
#define _REG_PUE 0x0C // gpio input pull enable
#define _REG_PUD 0x0D // gpio input pull direction
#define _REG_GIO 0x0E // gpio value
#define _REG_GIC 0x0F // gpio interrupt config
#define _REG_GIN 0x10 // gpio interrupt status

#define _WRITE_MASK (1 << 7)

#define CFG_OVERFLOW_ON  (1 << 0)
#define CFG_OVERFLOW_INT (1 << 1)
#define CFG_CAPSLOCK_INT (1 << 2)
#define CFG_NUMLOCK_INT  (1 << 3)
#define CFG_KEY_INT      (1 << 4)
#define CFG_PANIC_INT    (1 << 5)

#define INT_OVERFLOW     (1 << 0)
#define INT_CAPSLOCK     (1 << 1)
#define INT_NUMLOCK      (1 << 2)
#define INT_KEY          (1 << 3)
#define INT_PANIC        (1 << 4)

#define KEY_CAPSLOCK     (1 << 5)
#define KEY_NUMLOCK      (1 << 6)
#define KEY_COUNT_MASK   (0x1F)

#define DIR_OUTPUT       0
#define DIR_INPUT        1

#define PUD_DOWN         0
#define PUD_UP           1

BBQ10Keyboard::BBQ10Keyboard()
    : m_wire(nullptr)
{

}

void BBQ10Keyboard::begin(uint8_t addr, TwoWire *wire)
{
    m_addr = addr;
    m_wire = wire;

    m_wire->begin();

    reset();
}

void BBQ10Keyboard::reset()
{
    m_wire->beginTransmission(m_addr);
    m_wire->write(_REG_RST);
    m_wire->endTransmission();

    delay(100);
}

void BBQ10Keyboard::attachInterrupt(uint8_t pin, void (*func)()) const
{
    ::pinMode(pin, INPUT_PULLUP);
    ::attachInterrupt(digitalPinToInterrupt(pin), func, RISING);
}

void BBQ10Keyboard::detachInterrupt(uint8_t pin) const
{
    ::detachInterrupt(pin);
}

void BBQ10Keyboard::clearInterruptStatus()
{
    writeRegister(_REG_INT, 0x00);
}

uint8_t BBQ10Keyboard::status() const
{
    return readRegister8(_REG_KEY);
}

uint8_t BBQ10Keyboard::keyCount() const
{
    return status() & KEY_COUNT_MASK;
}

BBQ10Keyboard::KeyEvent BBQ10Keyboard::keyEvent() const
{
    KeyEvent event = { .key = '\0', .state = StateIdle };

    if (keyCount() == 0)
        return event;

    const uint16_t buf = readRegister16(_REG_FIF);
    event.key = buf >> 8;
    event.state = KeyState(buf & 0xFF);

    return event;
}

float BBQ10Keyboard::backlight() const
{
    return readRegister8(_REG_BKL) / 255.0f;
}

void BBQ10Keyboard::setBacklight(float value)
{
    writeRegister(_REG_BKL, value * 255);
}

float BBQ10Keyboard::backlight2() const
{
    return readRegister8(_REG_BK2) / 255.0f;
}

void BBQ10Keyboard::setBacklight2(float value)
{
    writeRegister(_REG_BK2, value * 255);
}

void BBQ10Keyboard::pinMode(uint8_t pin, uint8_t mode)
{
    if (pin > 7)
        return;

    if (mode == INPUT || mode == INPUT_PULLUP) {
        updateRegisterBit(_REG_DIR, pin, DIR_INPUT);
    } else if (mode == OUTPUT) {
        updateRegisterBit(_REG_DIR, pin, DIR_OUTPUT);
    }
}

void BBQ10Keyboard::digitalWrite(uint8_t pin, uint8_t val)
{
    if (pin > 7)
        return;

    if (readRegisterBit(_REG_DIR, pin) == DIR_INPUT) {
        updateRegisterBit(_REG_PUD, pin, val == LOW ? PUD_DOWN : PUD_UP);
    } else {
        updateRegisterBit(_REG_GIO, pin, val == HIGH);
    }
}

int BBQ10Keyboard::digitalRead(uint8_t pin)
{
    if (pin > 7)
        return LOW;

    if (readRegisterBit(_REG_GIO, pin))
        return HIGH;

    return LOW;
}

uint8_t BBQ10Keyboard::readRegister8(uint8_t reg) const
{
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg);
    m_wire->endTransmission();

    m_wire->requestFrom(m_addr, 1);
    if (m_wire->available() < 1)
        return 0;

    return m_wire->read();
}

uint16_t BBQ10Keyboard::readRegister16(uint8_t reg) const
{
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg);
    m_wire->endTransmission();

    m_wire->requestFrom(m_addr, 2);
    if (m_wire->available() < 2)
        return 0;

    const uint8_t low = m_wire->read();
    const uint8_t high = m_wire->read();

    return (high << 8) | low;
}

uint8_t BBQ10Keyboard::readRegisterBit(uint8_t reg, uint8_t bit)
{
    return bitRead(readRegister8(reg), bit);
}

void BBQ10Keyboard::writeRegister(uint8_t reg, uint8_t value)
{
    uint8_t data[2];
    data[0] = reg | _WRITE_MASK;
    data[1] = value;

    m_wire->beginTransmission(m_addr);
    m_wire->write(data, sizeof(uint8_t) * 2);
    m_wire->endTransmission();
}

void BBQ10Keyboard::updateRegisterBit(uint8_t reg, uint8_t bit, uint8_t value)
{
    uint8_t oldValue = readRegister8(reg);
    uint8_t newValue = oldValue;

    bitWrite(newValue, bit, value);

    if (newValue != oldValue)
        writeRegister(reg, newValue);
}
