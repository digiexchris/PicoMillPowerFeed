#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <stdint.h>

/********* DRIVER SETTINGS ********* */

// the delay required by the driver to change direction
#define DRIVER_DIRECTION_CHANGE_DELAY_MS 5

// direction pin for the stepper driver
#define DRIVER_DIR_PIN 4

// enable pin for the stepper driver
#define DRIVER_EN_PIN 5

// step pin for the stepper driver, aka pulse
#define DRIVER_STEP_PIN 6

// If your driver requires a high signal to enable, set this to true
const bool DRIVER_ENABLE_VALUE = false;

// 1 second in moilliseconds. set to -1 to disable
// driver will disable after it has stopped for this amount of time
const int16_t DRIVER_DISABLE_TIMEOUT = 1000;

/************* CONTROLS ************** */

// left switch pin, movement occurs only while this pin is low
#define LEFTPIN 8

// right switch pin, movement occurs only while this pin is low
#define RIGHTPIN 7

// rapid movement switch pin. If movement is occurring, this selects the rapid speed. The rapid speed can be changed if this pin is low and the encoder is changed, even if there is no movement happening.
#define RAPIDPIN 9

// IMPORTANT: ENCODER_B_PIN has no effect, but due to the PIO routine, the B pin will always be ENCODER_A_PIN+1. Select this value with that in mind.
#define ENCODER_A_PIN 10

// for information only, has no effect.
#define ENCODER_B_PIN ENCODER_A_PIN + 1

// hold low for 1s and release to switch units
#define ENCODER_BUTTON_PIN 12

// how long to hold the encoder button to switch units
constexpr uint32_t UNITS_SWITCH_DELAY_MS = 1000;

// 10ms debounce for left/right/rapid/encoder button switches
constexpr uint32_t DEBOUNCE_DELAY_US = 10000;

// The number of steps per second to change the speed by for each encoder pulse.
// Common encoders often have 2 or more pulses per detent. if you want more speed per detent, increase this
#define ENCODER_COUNTS_TO_STEPS_PER_SECOND 10

/************* DISPLAY *************/

// comment this out to use the USB Console display and don't start the SSD1306
#define USE_SSD1306 1

// Uncomment this to rotate the display 180 degrees
//#define SSD1306_ROTATE_180_DEGREES

// usually either 0x3C or 0x3D
#define SSD1306_ADDRESS 0x3C

/*!< gpio number for I2C master data  */
#define I2C_MASTER_SDA_IO 16

/*IMPORTANT: if you change these, see pico datasheet for i2c pinouts, chosen pins must match I2C_MASTER_NUM */
#define I2C_MASTER_SCL_IO 17

/*!< I2C port number for master dev */
#define I2C_MASTER_NUM i2c0

/************* MECHANICAL PARAMETERS ************ */

// 200 rpm max output speed, as dictated by your machine (or the stepper)
constexpr uint16_t MAX_LEADSCREW_RPM = 200;

// maximum that the driver can handle
constexpr float MAX_DRIVER_STEPS_PER_SECOND = 200000;

// inclduding microstepping. eg: 200 steps per rev, 4x microstepping, this value would be set to 800
constexpr uint16_t STEPS_PER_MOTOR_REV = 1600;

// 18:73 reduction off of the leadscrew
const float STEPS_PER_LEADSCREW_REV = STEPS_PER_MOTOR_REV * (73.0 / 18);

// steps/s/s
constexpr uint16_t ACCELERATION = 20000;

// 6x acceleration for deceleration since this mill has lots of friction
constexpr uint16_t DECELERATION_MULTIPLIER = 6;

// The number of steps per second that the stepper can accelerate from zero to without acceleration being taken into account. See PIOStepper::Update.
// MUST BE GREATER THAN ZERO
constexpr uint16_t ACCELERATION_JERK = 10;

// If the power feed moves in the wrong direction, change this to true
const bool MOVE_LEFT_DIRECTION = false;

/******************INTERNAL USE BELOW HERE*************** */
const bool DRIVER_DISABLE_VALUE = !DRIVER_ENABLE_VALUE;
const bool MOVE_RIGHT_DIRECTION = !MOVE_LEFT_DIRECTION;
static_assert(ACCELERATION_JERK > 0, "ACCELERATION_JERK must be greater than zero");
const float maxStepsPerSecond = MAX_LEADSCREW_RPM * STEPS_PER_LEADSCREW_REV / 60;
const float mmPerRev = 0.25 * 25.4; // 4 tpi lead screw
const float stepsPerMm = STEPS_PER_LEADSCREW_REV / mmPerRev;

#define ENCODER_COUNTS_TO_ACCELERATION 100 / 4
#define ACCELERATION_PIN 13 // currently unused
#endif						// SHARED_H