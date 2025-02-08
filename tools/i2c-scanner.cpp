// #include "MachineState.hpp"
// #include "drivers/RP2040_HAL.hpp"
// #include <iostream>
// #include <memory>

// extern "C"
// {
// #include "FreeRTOS.h"
// #include "task.h"
// }

// #include "Common.hpp"
// #include "Display.hpp"
// #include "Stepper.hpp"
// #include "StepperState.hpp"
// #include "config.hpp"
// #include "drivers/PIOStepper.hpp"
// #include "drivers/ConsoleDisplay.hpp"

// std::shared_ptr<PowerFeed::IStepper> stepper;
// std::shared_ptr<PowerFeed::Time> iTime;

// std::shared_ptr<PowerFeed::StepperState> stepperState;

// std::shared_ptr<PowerFeed::Machine> machineState;

// std::unique_ptr<PowerFeed::Drivers::RP2040_HAL> hal;

// std::shared_ptr<PowerFeed::Display> display;

// void stepperUpdateTask(void *pvParameters)
// {
// 	while (true)
// 	{
// 		stepperState->Run();
// 		taskYIELD();
// 	}
// }

// void createStepperTask()
// {
// 	xTaskCreate(stepperUpdateTask, "Stepper Task", 2048, NULL, 1, NULL);
// }

// int main()
// {
// 	display = std::make_shared<PowerFeed::Drivers::ConsoleDisplay>();
// 	stepper = std::make_shared<PowerFeed::Drivers::PIOStepper>(PowerFeed::Drivers::PIOStepper(stepPinStepper, dirPinStepper, enablePinStepper, maxStepsPerSecond, ACCELERATION, DECELERATION_MULTIPLIER, pio0, 0, stepsPerMotorRev));
// 	iTime = std::make_shared<PowerFeed::Time>();
// 	stepperState = std::make_shared<PowerFeed::StepperState>(stepper, iTime);
// 	machineState = std::make_shared<PowerFeed::Machine>(display, stepperState);

// 	// todo: load saved units and speed from eeprom

// 	hal = std::make_unique<PowerFeed::Drivers::RP2040_HAL>(machineState);

// 	display->Clear();
// 	display->DrawSpeed(0);

// 	createStepperTask();
// 	vTaskStartScheduler();
// 	return 0;
// }

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define PIN_SDA 16
#define PIN_SCL 17

bool reserved_addr(uint8_t addr)
{
	return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main()
{
	stdio_init_all();

	// This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL) running at 100kHz
	i2c_init(i2c0, 100 * 1000);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SDA);
	gpio_pull_up(PIN_SCL);

	while (true)
	{
		printf("\nI2C Bus Scan for Si7021 and VCNL4000\n");
		printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

		for (int addr = 0; addr < (1 << 7); ++addr)
		{
			if (addr % 16 == 0)
			{
				printf("%02x ", addr);
			}

			int ret;
			uint8_t rxdata;
			if (reserved_addr(addr))
				ret = PICO_ERROR_GENERIC;
			else
				ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

			printf(ret < 0 ? "." : "@");
			printf(addr % 16 == 15 ? "\n" : "  ");
		}

		sleep_ms(2000);
	}
}