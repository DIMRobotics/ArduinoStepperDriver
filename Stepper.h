#ifndef ARDUINO_STEPPER_2014_H
#define ARDUINO_STEPPER_2014_H

#include <Arduino.h>
#include <stdint.h>

/* Number of parsing axis */
#define NUM_AXIS 3

/* Use timer2 for ATmega32U4 and ATmega328P to not
 * crash all things in Arduino environment 
 */

typedef enum {
        LEFT,
        RIGHT
} _eng_t;

typedef enum {
        FORWARD = HIGH,
        BACKWARD = LOW
} _dir_t;

struct stepper_motor {
        uint8_t step; /* step pin */
        uint8_t dir; /* dir pin */
        uint8_t enable; /* enable pin, 255 is empty value */
        int32_t path;

        int8_t _dir; /* direction coefficient */
        uint8_t _state; /* current state */
        uint16_t _delay; /* current timer value */
        uint16_t _base_delay; /* start timer value */
};

class _Stepper
{
public:
        void init();
        
        uint8_t newAxis(uint8_t step, uint8_t dir, uint8_t enable);

        void enable(uint8_t axis);
        void disable(uint8_t axis);
        void setDir(uint8_t axis, uint8_t dir);
        void setDelay(uint8_t axis, uint16_t val);
        void setSpeed(uint8_t axis, uint16_t val); /* convert speed to delay */

private:
        uint8_t left, right; /* numbers of axis to drive 2-wheel chassis */
};

extern _Stepper Stepper;

#endif
