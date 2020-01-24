#include <Wire.h>

#define BBQ10KEYBOARD_DEFAULT_ADDR 0x1f

class BBQ10Keyboard
{
    public:
        enum KeyState
        {
            StateIdle = 0,
            StatePress,
            StateLongPress,
            StateRelease
        };

        struct KeyEvent
        {
            char key;
            KeyState state;
        };

        BBQ10Keyboard();

        void begin(uint8_t addr = BBQ10KEYBOARD_DEFAULT_ADDR, TwoWire *wire = &Wire);

        void reset(void);

        void attachInterrupt(uint8_t pin, void (*func)(void)) const;
        void detachInterrupt(uint8_t pin) const;
        void clearInterruptStatus(void);

        uint8_t status(void) const;
        uint8_t keyCount(void) const;
        KeyEvent keyEvent(void) const;

        float backlight() const;
        void setBacklight(float value);

        uint8_t readRegister8(uint8_t reg) const;
        uint16_t readRegister16(uint8_t reg) const;
        void writeRegister(uint8_t reg, uint8_t value);

    private:
        TwoWire *m_wire;
        uint8_t m_addr;
};
