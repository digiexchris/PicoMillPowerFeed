#pragma once

#include "Display.hpp"
#include <cstdint>
#include <memory>

namespace PowerFeed
{

	struct Settings
	{
		uint32_t normalSpeed;
		uint32_t rapidSpeed;
		uint32_t acceleration;
		Units units;
	};

	class SettingsManager
	{
	public:
		virtual std::shared_ptr<Settings> Load() = 0;
		virtual void Save(std::shared_ptr<Settings> settings) = 0;
	};

} // namespace PowerFeed