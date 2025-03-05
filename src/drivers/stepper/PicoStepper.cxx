#include "PicoStepper.hxx"
#include <FreeRTOS.h>
#include <PIOStepper.hxx>
#include <task.h>

namespace PowerFeed::Drivers
{
	PicoStepper::PicoStepper(SettingsManager *aSettings, Time *aTime, PIO pio, uint sm)
		: mySettingsManager(aSettings), myTime(aTime), myStoppedAt(0), myDirection(false), myTargetDirection(false), myIsEnabled(false), myTaskHandle(nullptr)
	{
		uint sysclk = clock_get_hz(clk_sys);
		Settings::Driver driver = mySettingsManager->Get()->driver;
		Settings::Mechanical mech = mySettingsManager->Get()->mechanical;

		myPIOStepper = new PIOStepperSpeedController::PIOStepper(
			driver.driverStepPin,
			driver.driverDirPin,
			mech.maxStepsPerSecond,
			mech.acceleration,
			mech.deceleration,
			sysclk,
			125,
			nullptr,
			nullptr,
			nullptr,
			nullptr);

		xTaskCreate(PrivUpdateTask, "Stepper", 2048, this, 15, &myTaskHandle);
	}

	PicoStepper::~PicoStepper()
	{
		delete myPIOStepper;

		if (myTaskHandle != nullptr)
		{
			vTaskDelete(myTaskHandle);
			myTaskHandle = nullptr;
		}
	}

	void PicoStepper::SetSpeed(uint32_t speed)
	{
		myPIOStepper->SetTargetHz(speed);
	}

	void PicoStepper::PrivUpdate()
	{
		// if it returns false, it would have done no step output (ie. stopped or error)
		if (!myPIOStepper->Update())
		{
			// Check if the stepper is stopped and disable the driver if it is
			if (myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPED)
			{

				auto driverDisableTimeout = mySettingsManager->Get()->driver.driverDisableTimeout;

				if (driverDisableTimeout >= 0)
				{

					if (myIsEnabled)
					{
						const uint64_t currentTime = myTime->GetCurrentTimeInMilliseconds();
						if (driverDisableTimeout > 0 && myStoppedAt + driverDisableTimeout < currentTime)
						{
							gpio_put(mySettingsManager->Get()->driver.driverEnPin,
									 mySettingsManager->Get()->driver.driverDisableValue);
							myStoppedAt = 0;
							myIsEnabled = false;
						}
						else
						{
							myStoppedAt = currentTime;
						}
					}
				}
			}
			vTaskDelay(pdMS_TO_TICKS(5));
		}
	}

	void PicoStepper::Stop()
	{
		if (myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPED || myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPING)
		{
			return;
		}
		myPIOStepper->Stop();
	}

	void PicoStepper::Start()
	{
		if (!myIsEnabled)
		{
			gpio_put(mySettingsManager->Get()->driver.driverEnPin,
					 mySettingsManager->Get()->driver.driverEnableValue);
			vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));

			myIsEnabled = true;
		}

		if (myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPED || myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPING)
		{
			myPIOStepper->Start();
		}
	}

	void PicoStepper::PrivUpdateTask(void *pvParameters)
	{
		auto stepper = static_cast<PicoStepper *>(pvParameters);
		while (true)
		{
			stepper->PrivUpdate();
		}
	}

	bool PicoStepper::GetDirection() { return myDirection; }
	bool PicoStepper::GetTargetDirection() { return myTargetDirection; }
	uint32_t PicoStepper::GetTargetSpeed()
	{
		auto stepperState = myPIOStepper->GetState();
		if (stepperState == PIOStepperSpeedController::StepperState::STOPPED || stepperState == PIOStepperSpeedController::StepperState::STOPPING)
		{
			return 0;
		}

		return myPIOStepper->GetTargetFrequency(); // Fixed: Added return statement
	}
	uint32_t PicoStepper::GetCurrentSpeed()
	{
		auto state = myPIOStepper->GetState();
		if (state == PIOStepperSpeedController::StepperState::STOPPED)
		{
			return 0;
		}
		return myPIOStepper->GetCurrentFrequency();
	}
	void PicoStepper::SetDirection(bool direction)
	{
		if (myPIOStepper->GetState() != PIOStepperSpeedController::StepperState::STOPPED)
		{
			BreakPanic("PicoStepper::SetDirection: Stepper is running, cannot change direction\n");
		}
		myTargetDirection = direction;
		gpio_put(mySettingsManager->Get()->driver.driverDirPin, direction);
		myDirection = direction;
		vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));
	}

	bool PicoStepper::IsRunning()
	{
		return myPIOStepper->GetState() != PIOStepperSpeedController::StepperState::STOPPED;
	}
}
