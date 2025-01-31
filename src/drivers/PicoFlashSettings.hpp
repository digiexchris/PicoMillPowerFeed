#pragma once

#include "MachineState.hpp"
#include "Settings.hpp"
#include <cstdint>
#include <memory>

namespace PicoMill
{

	class PicoFlashSettings : public SettingsManager
	{
	public:
		PicoFlashSettings();
		std::shared_ptr<Settings> Load() override;

		void Save(std::shared_ptr<Settings> settings) override;

	private:
		std::shared_ptr<Settings> mySettings;
	};

} // namespace PicoMill
