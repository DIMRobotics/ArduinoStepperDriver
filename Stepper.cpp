#include "Stepper.h"
#include <avr/interrupt.h>

static volatile struct stepper_motor _motors[NUM_AXIS];
static volatile uint8_t _num_motors = 0;

void _Stepper::init()
{
        /* Configure timer */
        /* Timer2, prescaler /1, enable overflow interrupt */
        TCCR2A = 0; /* huh, we need to disable all PWMs */
        TCCR2B = (1<<CS20); /* prescaler */
        TIMSK2 = (1<<TOIE2); /* overflow interrupt */
}

uint8_t _Stepper::newAxis(uint8_t step, uint8_t dir, uint8_t enable)
{
        _motors[_num_motors].step = step;
        _motors[_num_motors].dir = dir;
        _motors[_num_motors].enable = enable;
        
        pinMode(step, OUTPUT);
        pinMode(dir, OUTPUT);
        if (enable != 255)
                pinMode(enable, OUTPUT);
        
        return _num_motors++;
}

void _Stepper::enable(uint8_t axis)
{
        if (axis >= _num_motors)
                return;
        
        digitalWrite(_motors[axis].enable, LOW);
}

void _Stepper::disable(uint8_t axis)
{
        if (axis >= _num_motors)
                return;
        
        digitalWrite(_motors[axis].enable, HIGH);
}

void _Stepper::setDir(uint8_t axis, uint8_t dir)
{
        if (axis >= _num_motors)
                return;
        
        digitalWrite(_motors[axis].dir, dir);
        _motors[axis]._dir = dir == FORWARD ? 1 : -1;
}

void _Stepper::setDelay(uint8_t axis, uint16_t delay)
{
        if (axis >= _num_motors)
                return;
        
        _motors[axis]._base_delay = delay;
}

void _Stepper::setSpeed(uint8_t axis, uint16_t value)
{
        if (value != 0)
                value = 65535 / value;
        setDelay(axis, value);
}

_Stepper Stepper;

/* Special interrupt service functions */

static inline void toggle(volatile struct stepper_motor *s)
{
        if (s->_base_delay) {
                if (s->_state)
                        digitalWrite(s->step, HIGH);
                else
                        digitalWrite(s->step, LOW);
                
                s->path += s->_dir;
                s->_state = !(s->_state);
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
