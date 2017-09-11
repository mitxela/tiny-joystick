# Mini Pitchbend Joystick

A tiny joystick for bending pitch.

Description of the project and its development: https://mitxela.com/projects/tiny_joystick

Uses an ATtiny85 and the joystick from a PSP console. Uses V-USB with the report descriptors for MIDI streaming, and the oscillator calibration code from EasyLogger. 

The floating point operations need to be calibrated to the limits and dead zone of the joystick, which varies considerably from one joystick to the next. This is done via the javascript tool which uses the Web-MIDI API to talk to the joystick over MIDI SysEx. Currently, Google Chrome is the only browser that supports Web-MIDI, and it only supports SysEx on "secure" websites (https). The calibration tool is uploaded here: https://mitxela.com/other/pitchbend/calibration.htm

For instructions on how to do the calibration, [see here](https://mitxela.com/projects/tiny_joystick#calibration).