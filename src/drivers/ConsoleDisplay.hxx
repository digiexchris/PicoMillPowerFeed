#pragma once

#include "../Display.hxx"
// #include <memory.h>
#include <graphics/curses.h>
#include <stdint.h>

namespace PowerFeed::Drivers
{

	class ConsoleDisplay : public Display<ConsoleDisplay>
	{
	public:
		ConsoleDisplay();
		void Init();
		void DrawText(const char *text, const NXHANDLE &aFont, uint16_t x, uint16_t y);
		void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
		void ClearBuffer();
		void WriteBuffer();

	private:
		WINDOW *myDefaultWindow;
		// WINDOW *myRapidWindow;
	};

}