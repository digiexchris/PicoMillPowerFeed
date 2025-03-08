#pragma once

#include "Assert.hxx"
#include "config.h" //generated from cmake
#include <memory>

class IMutex
{
public:
	virtual ~IMutex() = default;
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

// template <typename MutexType>
// class Mutex : public IMutex
// {
// public:
// 	Mutex() : mutex_(std::make_unique<MutexType>()) {}
// 	void lock() override
// 	{
// 		mutex_->lock();
// 	}
// 	void unlock() override
// 	{
// 		mutex_->unlock();
// 	}

// private:
// 	std::unique_ptr<MutexType> mutex_;
// };

#if USE_FREERTOS
#include <FreeRTOS.h>
#include <semphr.h>
class FreeRTOSMutex : public IMutex
{
public:
	FreeRTOSMutex() : mutex_(xSemaphoreCreateMutex())
	{
		if (mutex_ == NULL)
		{
			Panic("Failed to create mutex");
		}
	}
	~FreeRTOSMutex()
	{
		if (mutex_ != NULL)
		{
			vSemaphoreDelete(mutex_);
			mutex_ = NULL;
		}
	}
	void lock() override
	{
		xSemaphoreTake(mutex_, portMAX_DELAY);
	}
	void unlock() override
	{
		xSemaphoreGive(mutex_);
	}

private:
	SemaphoreHandle_t mutex_;
};

using Mutex = FreeRTOSMutex;
#else
using Mutex = IMutex<std::mutex>;
#endif

template <typename MutexType>
class LockGuard
{
public:
	explicit LockGuard(MutexType &mutex) : mutex_(mutex)
	{
		mutex_.lock();
	}

	~LockGuard()
	{
		mutex_.unlock();
	}

	// Delete copy constructor and copy assignment operator
	LockGuard(const LockGuard &) = delete;
	LockGuard &operator=(const LockGuard &) = delete;

private:
	MutexType &mutex_;
};