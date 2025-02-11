

# Stepper Power Feed for milling machines (or other machinery where you want a consistent feed rate)

| Supported Targets | 
| -------- |
| Raspberry Pi Pico (RP2040) (reference hardware) |

This is for supporting a stepper motor retrofit onto the align style milling machine powerfeeds.

# Hardware

The standard switch pin configuration is pull-up, and pulling down to gnd when the switch is turned on. I recommend capacitors across the switches for debouncing, though there is some small software debouncing applied. There is no software debouncing on the encoder A and B channels, so .01uf capacitors to ground are recommended there as well.

The reference hardware uses an Align style power feed, with all of the control electronics inside removed. The lower portion that normally has a large gear on it still has the cross pin in it, and there is a slotted shaft coupling it to a Nema 34 stepper motor The slot in the shaft needs to be deep enough to allow the shaft to travel up and down to operate the clutch.

The original left/right/rapid switches remain in place, have one side of the switch tied to GND, and the other side tied to the pins defined in config.md. The original potentiometer is not used and may remain in place if desired. I have the rotary encoder installed to the right of the rapid switch, and the SSD1306 in a little window above it.

It uses an EC11 or equivalent rotary encoder for speed selection and switching between inches and metric.

TODO: I do have photos. Running out to the shop to get them! BRB

# Configuration

See config.json for tuneable parameters and the default pinout. An explanation of all settings is in config.md in this directory. The config can be edited live by plugging in a usb cable, it will emulate a usb flash drive. Edit CONFIG.JSON on that drive, ensuring you do not rename the file, and keep the file name as you find it. Power cycling the MCU after that will apply the settings.

The reference hardware has the driver set to 1600 steps per revolution, with 18:73 teeth bevel gears in the power feed, on a 0.250" per revolution leadscrew. Feel free to adjust the config if your hardware is different. This is only important if you use the LCD to see and set travel rate, though it does have an effect on maximum RPM and a few other parameters. It will probably work just fine without an LCD with the default parameters on most hardware as long as the steps per revolution on the driver are approximately 1600 (or, 800 will travel twice as fast for the same speed set by the encoder. Not a big deal, it'll just be more of a coarse adjustment).

Remeber to feed your pi pico a clean 5v on the 5v pin or the usb jack only, DO NOT feed it the full voltage sent to the stepper driver! I currently use the 3.3v regulator that's built into the pi pico dev board to run the switches. I have 5v going to the LCD VCC for a little more brightness or speed, but they run fine off of 3.3v as well if you want to eliminate the 5v requirement. My stepper driver works ok with 3.3v logic levels, but if you have long wires or are having issues, a level shifter might be a good idea (I currently use a 2n3096 transistor with a 220 ohm resistor on the base).

TODO: At some point I will post a schematic. Bug me if you need it.

# Building

There is a vscode compatible dev container. It should recommend the extensions required to build this project.

I will be providing a precompiled .uf2 file to flash to a pi pico so you do not need to install any software to use this. (soon!)