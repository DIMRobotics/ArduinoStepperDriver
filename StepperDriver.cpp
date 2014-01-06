#include "StepperDriver.h"
#include <avr/interrupt.h>
#include <util/atomic.h>

static volatile struct stepper_motor _motors[NUM_AXIS];
static volatile uint8_t _num_motors = 0;

void _StepperDriver::init()
{
        /* Configure timer */
        /* Timer2, prescaler /1, enable overflow interrupt */
        TCCR2A = 0; /* huh, we need to disable all PWMs */
        TCCR2B = (1<<CS20); /* prescaler */
        TIMSK2 = (1<<TOIE2); /* overflow interrupt */
}

axis_t _StepperDriver::newAxis(uint8_t step, uint8_t dir, uint8_t enable)
{
        /* Library is limited in number of axis.
         * If you need to control more axis, change NUM_AXIS
         * in header file, but be careful! You are still limited
         * in memory! */
        if (_num_motors == NUM_AXIS)
                return 255;

        _motors[_num_motors].step = step;
        _motors[_num_motors].dir = dir;
        _motors[_num_motors].enable = enable;
        
        pinMode(step, OUTPUT);
        pinMode(dir, OUTPUT);
        if (enable != 255)
                pinMode(enable, OUTPUT);
        
        return _num_motors++;
}

axis_t _StepperDriver::newAxis(uint8_t step, uint8_t dir)
{
        return newAxis(step, dir, 255); /* auto-ignoring ENABLE pin feature */
}

void _StepperDriver::enable(axis_t axis)
{
        if (axis >= _num_motors)
                return;
        
        if (_motors[axis].enable != 255)
        #ifdef INVERSE_ENABLE_LEVELS
                digitalWrite(_motors[axis].enable, HIGH);
        #else
                digitalWrite(_motors[axis].enable, LOW);
        #endif
}

void _StepperDriver::disable(axis_t axis)
{
        if (axis >= _num_motors)
                return;
        
        if (_motors[axis].enable != 255)
        #ifdef INVERSE_ENABLE_LEVELS
                digitalWrite(_motors[axis].enable, LOW);
        #else
                digitalWrite(_motors[axis].enable, HIGH);
        #endif
}

void _StepperDriver::setDir(axis_t axis, uint8_t dir)
{
        if (axis >= _num_motors)
                return;
        
        digitalWrite(_motors[axis].dir, dir);
        _motors[axis]._dir = dir == FORWARD ? 1 : -1;
}

void _StepperDriver::setDelay(axis_t axis, uint16_t delay)
{
        if (axis >= _num_motors)
                return;
        
        _motors[axis]._base_delay = delay / 16; /* this division is for conversion to microseconds */
        _motors[axis]._rq_path = 0;
}

void _StepperDriver::setSpeed(axis_t axis, uint16_t value)
{
        if (value != 0)
                value = 65535 / value;
        setDelay(axis, value);
}

void _StepperDriver::write(axis_t axis, int32_t value)
{
        if (axis >= _num_motors)
                return;
        
        if (value < 0) {
                value = -value;
                setDir(axis, BACKWARD);
        } else {
                setDir(axis, FORWARD);
        }
        
        setSpeed(axis, (uint16_t) value);
}

void _StepperDriver::write(axis_t axis, int32_t speed, uint32_t path)
{
        if (axis >= _num_motors)
                return;
        
        write(axis, speed);
        _motors[axis]._rq_path = path;
}

void _StepperDriver::stop(axis_t axis)
{
        write(axis, 0);
}

uint8_t _StepperDriver::busy(axis_t axis)
{
        if (axis >= _num_motors)
                return 0;
        
        uint8_t ret;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
                ret = _motors[axis]._base_delay > 0;
        }

        return ret;
}

void _StepperDriver::wait(axis_t axis)
{
        if (axis >= _num_motors || getPath(axis) == 0)
                return;

        while (busy(axis))
                asm volatile ("nop");
}

void _StepperDriver::move(axis_t axis, int32_t speed, uint32_t path)
{
        write(axis, speed, path);
        wait(axis);
}

int32_t _StepperDriver::getPath(axis_t axis)
{
        if (axis >= _num_motors)
                return 0;
        
        int32_t val;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
                val = _motors[axis].path;
        }
        
        return val;
}

void _StepperDriver::resetPath(axis_t axis)
{
        if (axis >= _num_motors)
                return;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
                _motors[axis].path = 0;
        }
}

_StepperDriver StepperDriver;

/*
 * Special interrupt service functions 
 */

static inline void toggle(volatile struct stepper_motor *s)
{
        if (s->_base_delay) {
                if (s->_state)
                        digitalWrite(s->step, HIGH);
                else
                        digitalWrite(s->step, LOW);
                
                s->path += s->_dir;
                s->_state = !(s->_state);

                if (s->_rq_path > 0) { /* path limit detector */
                        s->_rq_path--;
                        if (s->_rq_path == 0)
                                s->_base_delay = 0;
                }
        }
}

static inline void timer_interrupt()
{
        for (uint8_t i=0; i<_num_motors; i++) {
                if (_motors[i]._delay != 0) {
                        _motors[i]._delay--;
                } else {
                        toggle(&_motors[i]);
                        _motors[i]._delay = _motors[i]._base_delay;
                }
        }
}

ISR(TIMER2_OVF_vect)
{
        timer_interrupt();
}
