#include "StepperDriver.h"

void _StepperChassis::init(axis_t left, axis_t right)
{
        _left = left;
        _right = right;
}

void _StepperChassis::write(int16_t left, int16_t right)
{
        StepperDriver.write(_left, left);
        StepperDriver.write(_right, right);
}

void _StepperDriver::stop()
{
        write(0, 0);
}

_StepperChassis StepperChassis;
