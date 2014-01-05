#ifndef ARDUINO_STEPPER_2014_H
#define ARDUINO_STEPPER_2014_H

#include <Arduino.h>
#include <stdint.h>

/* Number of parsing axis */
#define NUM_AXIS 3

/* Configuration of chassis; if forward direction set by HIGH
 * level on DIR pin, it is a direct levels, otherwise it is
 * inversed levels.
 *
 * Uncomment if your chassis configuration is inversed
 */
//#define INVERSE_LEVELS

/* Use timer2 for ATmega32U4 and ATmega328P to not
 * crash all things in Arduino environment 
 */

#ifdef INVERSE_LEVELS
typedef enum {
        FORWARD = LOW,
        BACKWARD = HIGH
} _dir_t;
#else
typedef enum {
        FORWARD = HIGH,
        BACKWARD = LOW
} _dir_t;
#endif

typedef uint8_t axis_t;

struct stepper_motor {
        uint8_t step; /* step pin */
        uint8_t dir; /* dir pin */
        uint8_t enable; /* enable pin, 255 is empty value */
        int32_t path;

        int8_t _dir; /* direction coefficient */
        uint8_t _state; /* current state */
        uint16_t _delay; /* current timer value */
        uint16_t _base_delay; /* start timer value */
        uint32_t _rq_path; /* path for engine to rotate on */
};

class _StepperDriver
{
public:
        void init();
        
        axis_t newAxis(uint8_t step, uint8_t dir, uint8_t enable);

        void enable(axis_t axis);
        void disable(axis_t axis);
        void setDir(axis_t axis, uint8_t dir);
        void setDelay(axis_t axis, uint16_t val);
        void setSpeed(axis_t axis, uint16_t val); /* convert speed to delay */
        void write(axis_t axis, int32_t speed); /* continious rotation of engine */
        void write(axis_t axis, int32_t speed, uint32_t path); /* rotate engine to set path */
        void move(axis_t axis, int32_t speed, uint32_t path); /* rotate engine to set path */
        void stop(axis_t axis); /* e-stop */
        uint8_t busy(axis_t axis);
        void wait(axis_t axis);

        int32_t getPath(axis_t axis);
        void resetPath(axis_t axis);
};

class _StepperChassis
{
public:
        void init(axis_t left, axis_t right);
        void write(int16_t left_spd, int16_t right_spd);
        void stop();
private:
        axis_t _left, _right;

};

extern _StepperDriver StepperDriver;
extern _StepperChassis StepperChassis;

#endif
