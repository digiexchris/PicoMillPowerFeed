#pragma once

#include <cassert>
#include <cstdint>
#include <string>

namespace PowerFeed
{

	// First, define the concept that all stepper implementations must satisfy
	template <typename T>
	concept StepperImpl = requires(T &stepper) {
		{ stepper.SetDirection(bool{}) };
		{ stepper.GetDirection() } -> std::convertible_to<bool>;
		{ stepper.GetTargetDirection() } -> std::convertible_to<bool>;
		{ stepper.GetTargetSpeed() } -> std::convertible_to<uint32_t>;
		{ stepper.GetCurrentSpeed() } -> std::convertible_to<uint32_t>;
		{ stepper.SetSpeed(uint32_t{}) };
		{ stepper.Start() };
		{ stepper.Stop() };
		{ stepper.IsRunning() } -> std::convertible_to<bool>;
		{ stepper.IsStopping() } -> std::convertible_to<bool>;
	};

	// Forward declaration with concept constraint
	template <typename Derived>
		requires StepperImpl<Derived>
	class Stepper;

	// Base class without concept constraint - used for implementation
	template <typename Derived>
	class StepperBase
	{
	public:
		bool GetDirection()
		{
			return static_cast<Derived *>(this)->GetDirection();
		}

		bool GetTargetDirection()
		{
			return static_cast<Derived *>(this)->GetTargetDirection();
		}

		void SetDirection(bool aDirection)
		{
			if (IsRunning())
			{
				assert(!IsRunning() && "StepperBase: Cannot change direction while running");
			}
			static_cast<Derived *>(this)->SetDirection(aDirection);
		}

		uint32_t GetTargetSpeed()
		{
			return static_cast<Derived *>(this)->GetTargetSpeed();
		}

		void Stop()
		{
			static_cast<Derived *>(this)->Stop();
		}

		void Start()
		{
			static_cast<Derived *>(this)->Start();
		}

		void SetSpeed(uint32_t speed)
		{
			static_cast<Derived *>(this)->SetSpeed(speed);
		}

		uint32_t GetCurrentSpeed()
		{
			return static_cast<Derived *>(this)->GetCurrentSpeed();
		}

		bool IsRunning()
		{
			return static_cast<Derived *>(this)->IsRunning();
		}

		bool IsStopping()
		{
			return static_cast<Derived *>(this)->IsStopping();
		}

		virtual ~StepperBase() = default;
	};

	// Concept-constrained class that inherits from the base
	template <typename Derived>
		requires StepperImpl<Derived>
	class Stepper : public StepperBase<Derived>
	{
	public:
		// This class exists only to enforce the concept constraint
	};

	// Helper function template to validate a type against the concept
	template <typename T>
	constexpr bool ValidateStepper()
	{
		static_assert(StepperImpl<T>, "Type must implement StepperImpl requirements");
		return true;
	}

} // namespace PowerFeed