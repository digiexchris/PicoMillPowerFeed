#include "ConsoleDisplay.hxx"

#include <cstring>
#include <iostream>
#include <sstream>

#include "../icons.hxx"

namespace PowerFeed::Drivers
{

	ConsoleDisplay::ConsoleDisplay(std::shared_ptr<SettingsManager> aSettings) : Display(aSettings)
	{
		// 33x4 characters approximately equals the ssd1306 display
		// assuming the 12x16 font is used
	}

	void ConsoleDisplay::ClearBuffer()
	{
		myOutputBuffer.clear();
	}

	void ConsoleDisplay::WriteBuffer()
	{
		// Clear screen and move to home position
		std::cout << "\033[2J\033[H" << myOutputBuffer << std::flush;
	}

	void ConsoleDisplay::SetCursor(uint16_t x, uint16_t y)
	{
		std::stringstream pos;
		pos << "\033[" << (y + 1) << ";" << (x + 1) << "H";
		myOutputBuffer += pos.str();
	}

	void ConsoleDisplay::Refresh()
	{
		std::cout << "\033[2J\033[H";

		for (int x = 0; x < 33; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				std::cout << " ";
			}
		}
		std::cout << std::flush;

		WriteBuffer();
	}

	void ConsoleDisplay::DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y)
	{
		// Convert pixel coordinates to character positions (approximate)
		int charX = x / 8;	// Assuming 8 pixels per character width
		int charY = y / 16; // Assuming 16 pixels per character height

		std::stringstream pos;
		pos << "\033[" << (charY + 1) << ";" << (charX + 1) << "H" << text;
		myOutputBuffer += pos.str();
	}

	void ConsoleDisplay::DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		if (image == &moveleft32[0])
		{
			DrawText("<", nullptr, x, y);
		}
		else if (image == &moveright32[0])
		{
			DrawText(">", nullptr, x, y);
		}
		else if (image == &stop32[0])
		{
			DrawText("[]", nullptr, x, y);
		}
		else if (image == &rapidleft32[0])
		{
			DrawText("<<", nullptr, x, y);
		}
		else if (image == &rapidright32[0])
		{
			DrawText(">>", nullptr, x, y);
		}
		else
		{
			DrawText("@", nullptr, x, y);
		}
	}

} // namespace PowerFeed