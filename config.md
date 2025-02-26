# Configuration Documentation

## ACCESSING CONFIG
Plug the usb cable into your cable, and a FAT12 drive should appear. On it you will find
two files, a readme containing a link to this documentation, and CONFIG.JSON. If you delete CONFIG.JSON, a new empty one will be generated next time the powerfeed boots. You may edit this config, save it back to the drive, and power cycle the power feed to make it take effect.

## DRIVER SETTINGS

- **DRIVER_DIRECTION_CHANGE_DELAY_MS**: The delay in milliseconds required by the driver to change direction. Default: `5`
- **DRIVER_DIR_PIN**: Direction pin for the stepper driver. Default: `4`
- **DRIVER_EN_PIN**: Enable pin for the stepper driver. Default: `5`
- **DRIVER_STEP_PIN**: Step pin for the stepper driver, aka pulse. Default: `6`
- **DRIVER_ENABLE_VALUE**: If your driver requires a high signal to enable, set this to `true`, or `false` if it requires a low signal to enable. Default: `false`
- **DRIVER_DISABLE_TIMEOUT**: Driver will disable after it has stopped for this amount of time (in milliseconds). Set to `-1` to keep it always enabled. Default: `1000`

## CONTROLS

- **LEFTPIN**: Left switch pin, movement occurs only while this pin is low. Default: `8`
- **RIGHTPIN**: Right switch pin, movement occurs only while this pin is low. Default: `7`
- **RAPIDPIN**: Rapid movement switch pin. If movement is occurring, this selects the rapid speed. The rapid speed can be changed if this pin is low and the encoder is changed, even if there is no movement happening. Default: `9`
- **ENCODER_A_PIN**: IMPORTANT: ENCODER_B_PIN has no effect, but due to the PIO routine, the B pin will always be ENCODER_A_PIN+1. Select this value with that in mind. Default: `10`
- **ENCODER_B_PIN**: For information only, has no effect. Default: `ENCODER_A_PIN + 1`
- **ENCODER_BUTTON_PIN**: GPIO pin for the Units switch button. Default: `12`
- **UNITS_SWITCH_DELAY_MS**: How long to hold the encoder button to switch units. Default: `1000`
- **DEBOUNCE_DELAY_US**: 10ms (in microseconds) debounce for left/right/rapid/encoder button switches. Default: `10000`
- **ENCODER_COUNTS_TO_STEPS_PER_SECOND**: The number of steps per second to change the speed by for each encoder pulse. Common encoders often have 2 or more pulses per detent. If you want more speed per detent, increase this. Default: `10`
- **ENCODER_INVERT**: Set this to `true` if the encoder direction is inverted. Default: `true`

## DISPLAY

- **USE_SSD1306**: Set this to `0` to use the USB Console display and don't start the SSD1306. Default: `1`
- **SSD1306_ADDRESS**: In decimal, not hex. Usually either `60` (aka 0x3C) or `61` (aka 0x3D). Default: `60`
- **I2C_MASTER_SDA_IO**: GPIO number for I2C master data. Default: `16`
- **I2C_MASTER_SCL_IO**: GPIO number for I2C master clock. Default: `17`
- **I2C_MASTER_NUM**: I2C port number (ie. 0 is i2c0 from the rp2040 datasheet). `0` or `1` Default: `0`

## MECHANICAL PARAMETERS

- **MAX_LEADSCREW_RPM**: 200 rpm max output speed, as dictated by your machine (or the stepper). Default: `200`
- **MAX_DRIVER_STEPS_PER_SECOND**: Maximum that the driver can handle. Default: `200000`
- **STEPS_PER_MOTOR_REV**: Including microstepping. For example: 200 steps per rev, 4x microstepping, this value would be set to `800`. Default: `1600`
- **LEADSCREW_GEAR_REDUCTION**: For an Align style power feed with a 18:73 reduction off before the leadscrew, Default: `4.055555556`
- **ACCELERATION**: Steps per second squared. Default: `20000`
- **DECELERATION_MULTIPLIER**: 6x acceleration for deceleration since this mill has lots of friction, and the power feed mechanically disconnects the lead screw when turning it off. Default: `6`
- **ACCELERATION_JERK**: The number of steps per second that the stepper can accelerate from zero to without acceleration being taken into account. MUST BE GREATER THAN 1. Default: `10`
- **MOVE_LEFT_DIRECTION**: If the power feed moves in the wrong direction, change this to `true`. Default: `false`

## SAVED SETTINGS
- **NORMAL_SPEED**: The currently set values for the normal traverse speeds. If you change the speed using the encoder, it will automatically overwrite this value. Default: `1000`
- **RAPID_SPEED**: The default speed for rapid movement in steps per second, automatically overwritten when the rapid speed is changed in use. Default: `15000`
- **INCH_UNITS**: Set this to `true` to use inches as the unit of measurement, or `false` to use millimeters. Overwritten when the units change using the encoder button. Default: `false`