#pragma once

#include "../Display.hxx"

namespace PowerFeed::Drivers
{

	class ConsoleDisplay : public Display
	{
	public:
		ConsoleDisplay(std::shared_ptr<SettingsManager> aSettings);
		virtual void ClearBuffer() override;
		virtual void WriteBuffer() override;

	protected:
		virtual void DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y) override;
		virtual void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
		void SetCursor(uint16_t x, uint16_t y);
		void Refresh() override;

	private:
		std::string myOutputBuffer;
	};

} // namespace PowerFeed