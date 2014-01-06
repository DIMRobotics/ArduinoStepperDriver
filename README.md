StepperDriver
=============

Library for Arduino to control stepper motors using CNC drivers.

StepperDriver library for Arduino adds a friendful and effective support of CNC stepper drivers which are using STEP/DIR interface.

Features
========

* Independent control of each motor/axis
* Work in background: do other things while motor is in duty
* Path control: limit motor's movement angle
* True 16-bit design
* Control either step delay or rotation speed
* Connection to each pin, no limitaion
* Support ATmega328P/168 and ATmega32U4 Arduino boards (not tested at others)

How it works
============

In fact, library creates a set of software timers (based on one hardware timer) which are counting the delay for step signal. Counting is independent for each channel. Maximal number of channels is limited (default 3, you can easily change this value; see Tuning page on project's wiki). In each timer cycle the state of STEP changes.

Links
=====

All current documentation is available here:
https://github.com/DIMRobotics/ArduinoStepperDriver/wiki - Project's Wiki
