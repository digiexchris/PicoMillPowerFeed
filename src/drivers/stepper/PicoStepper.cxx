#include "PicoStepper.hxx"
#include "Assert.hxx"
#include <FreeRTOS.h>
#include <PIOStepper.hxx>
#include <task.h>
#include <hardware/gpio.h>

namespace PowerFeed::Drivers
{
	PicoStepper::PicoStepper(SettingsManager *aSettings, Time *aTime, PIO pio, uint sm)
		: mySettingsManager(aSettings), myTime(aTime), myStoppedAt(0), myDirection(false), myTargetDirection(false), myIsEnabled(false), myTaskHandle(nullptr)
	{

		uint sysclk = clock_get_hz(clk_sys);
		Settings::Driver driver = mySettingsManager->Get()->driver;
		Settings::Mechanical mech = mySettingsManager->Get()->mechanical;

		gpio_init(driver.driverDirPin);
		gpio_set_dir(driver.driverDirPin, GPIO_OUT);
		
		gpio_init(driver.driverEnPin);
		gpio_set_dir(driver.driverEnPin, GPIO_OUT);

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

		xTaskCreate(PrivUpdateTask, "Stepper", 4 * 2048, this, 15, &myTaskHandle);

		vTaskCoreAffinitySet(myTaskHandle, (1 << 0));

		PrivDisable();
	}

	PicoStepper::~PicoStepper()
	{
		if (myTaskHandle != nullptr)
		{
			vTaskDelete(myTaskHandle);
			myTaskHandle = nullptr;
		}

		delete myPIOStepper;
	}

	void PicoStepper::SetSpeed(uint32_t speed)
	{
		LockGuard<Mutex> lock(myMutex);
		myPIOStepper->SetTargetHz(speed);
	}

	void PicoStepper::PrivUpdate()
	{
		LockGuard<Mutex> lock(myMutex);
		taskENTER_CRITICAL();

		myPIOStepper->Update();

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
						PrivDisable();
						myStoppedAt = 0;
					}
					else
					{
						myStoppedAt = currentTime;
					}
				}
			}
		}
		taskEXIT_CRITICAL();
	}

	void PicoStepper::Stop()
	{
		LockGuard<Mutex> lock(myMutex);
		if (myPIOStepper->GetState() != PIOStepperSpeedController::StepperState::STOPPED &&
			myPIOStepper->GetState() != PIOStepperSpeedController::StepperState::STOPPING)
		{
			myPIOStepper->Stop();
		}
	}

	void PicoStepper::Start()
	{
		if (!myIsEnabled)
		{
			PrivEnable();

			// TODO figure out a better way to handle this, maybe enabling/disabling should be the layer above? should not be in a section protected by a mutex.
			// vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));
		}

		if (myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPED ||
			myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPING)
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
			portYIELD();
		}
	}

	bool PicoStepper::GetDirection() { return myDirection; }
	bool PicoStepper::GetTargetDirection() { return myTargetDirection; }
	uint32_t PicoStepper::GetTargetSpeed()
	{
		uint32_t speed = 0;
		LockGuard<Mutex> lock(myMutex);
		auto stepperState = myPIOStepper->GetState();
		if (stepperState != PIOStepperSpeedController::StepperState::STOPPED &&
			stepperState != PIOStepperSpeedController::StepperState::STOPPING)
		{
			speed = myPIOStepper->GetTargetFrequency();
		}
		return speed;
	}

	uint32_t PicoStepper::GetCurrentSpeed()
	{
		uint32_t speed = 0;
		LockGuard<Mutex> lock(myMutex);
		auto state = myPIOStepper->GetState();
		if (state != PIOStepperSpeedController::StepperState::STOPPED)
		{
			speed = myPIOStepper->GetCurrentFrequency();
		}
		return speed;
	}

	void PicoStepper::SetDirection(bool direction)
	{
		LockGuard<Mutex> lock(myMutex);
		if (myPIOStepper->GetState() != PIOStepperSpeedController::StepperState::STOPPED)
		{
			Panic("PicoStepper::SetDirection: Stepper is running, cannot change direction\n");
			return;
		}

		myTargetDirection = direction;
		gpio_put(mySettingsManager->Get()->driver.driverDirPin, direction);
		myDirection = direction;
		// TODO implement this at the layer above just before Start occurs
		//  vTaskDelay(pdMS_TO_TICKS(mySettingsManager->Get()->driver.driverDirectionChangeDelayMs));
	}

	bool PicoStepper::IsRunning()
	{
		bool running = false;
		LockGuard<Mutex> lock(myMutex);
		running = myPIOStepper->GetState() != PIOStepperSpeedController::StepperState::STOPPED;
		return running;
	}

	bool PicoStepper::IsStopping()
	{
		bool stopping = false;
		LockGuard<Mutex> lock(myMutex);
		stopping = myPIOStepper->GetState() == PIOStepperSpeedController::StepperState::STOPPING;
		return stopping;
	}

	void PicoStepper::PrivEnable()
	{
		gpio_put(mySettingsManager->Get()->driver.driverEnPin,
				 mySettingsManager->Get()->driver.driverEnableValue);
		myIsEnabled = true;
	}

	void PicoStepper::PrivDisable()
	{
		gpio_put(mySettingsManager->Get()->driver.driverEnPin,
				 !mySettingsManager->Get()->driver.driverDisableValue);
		myIsEnabled = false;
	}
}