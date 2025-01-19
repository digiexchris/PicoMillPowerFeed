

# ESP32-S2 Stepper Power Feed for milling machines (or other machinery where you want a consistent feed rate)

| Supported Targets | 
| -------- |
| ESP32-S2 (reference hardware) |
| ESP32-S3 |

This is for supporting a stepper motor retrofit onto the align style milling machine powerfeeds.

# Hardware

Why the S2? I had several Wemos S2 Mini's on hand, and because they don't have the jtag pins broken out, debugging is painful. They're close enough to my S3 devkit C1 that my bench setup is the S3, and the final working hardware on the mill is the S2-mini. It would probably run just fine on the ESP32-Wroom-32 as well, with maybe some minor changes to the FastAccelStepper init. Let me know if it works for you!

The standard switch pin configuration is pull-down, and applying 3.3v when the switch is turned on. I recommend capacitors across the switches for debouncing, though there is some small software debouncing applied.

The reference hardware uses an Align style power feed, with all of the control electronics inside removed. The lower portion that normally has a large gear on it still has the cross pin in it, and there is a slotted shaft coupling it to a Nema 34 stepper motor The slot in the shaft needs to be deep enough to allow the shaft to travel up and down to operate the clutch.

The original left/right/rapid switches remain in place, have one side of the switch tied to +3.3v, and the other side tied to the below pins. The original potentiometer is not used and may remain in place if desired. I have the rotary encoder installed to the right of the rapid switch, and the SSD1306 in a little window above it.

It uses an EC11 or equivalent rotary encoder for speed selection and switching between inches and metric.

|Function| GPIO |
|Move Left| 38 |
|Move Right| 35 |
|Rapid| 37|
|Encoder A| 8|
|Encoder B| 18|
|Encoder Button| 17|

TODO: I do have photos. Running out to the shop to get them! BRB

# Configuration

See config.h for tuneable parameters and the default pinout. If you do not want to use the lcd screen, comment out the line '#define ENABLE_SSD1306 1' in config.h and it should run headless just fine, but there has not been much testing with it disabled.

The reference hardware has the driver set to 1600 steps per revolution, with 18:73 teeth bevel gears in the power feed, on a 0.250" per revolution leadscrew. Feel free to adjust 'stepsPerRev' and 'mm per rev' if your hardware is different. This is only important if you use the LCD to see and set travel rate, though it does have an effect on maximum RPM and a few other parameters. It will probably work just fine without an LCD with the default parameters on most hardware as long as the steps per revolution on the driver are approximately 1600 (or, 800 will travel twice as fast for the same speed set by the encoder. Not a big deal, it'll just be more of a coarse adjustment).

Remeber to feed your esp32 a clean 5v on the 5v pin or the usb jack only, DO NOT feed it the full voltage sent to the stepper driver! I currently use the 3.3v regulator that's built into the Wemos S2 Mini board to run the switches. I have 5v going to the LCD VCC for a little more brightness or speed, but they run fine off of 3.3v as well if you want to eliminate the 5v requirement. My stepper driver works ok with 3.3v logic levels, but if you have long wires or are having issues, a level shifter might be a good idea (I currently use a 2n3096 transistor with a 220 ohm resistor on the base).

TODO: At some point I will post a schematic. Bug me if you need it.

# Building

Setup ESP-IDF version 4.4.7 or higher, but less than 5.0. Copy sdkconfig.esp32s2 or sdkconfig.esp32s3 to sdkconfig. Build and flash as usual. 

If you have troubles building this let me know and I'll add a .elf or .bin that can be flashed with the esp flash tool. I was planning on adding an spi sd card reader to this for configuration but it's currently satisfying my requirements, if you feel like doing it feel free!