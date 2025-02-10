#include "ConsoleDisplay.hxx"
// #include "hardware/i2c.h"
// #include "ssd1306.h"
// #include "textRenderer/12x16_font.h"
// #include "textRenderer/TextRenderer.h"
#include <cstdio>
#include <stdint.h>

#include "../Helpers.hxx"
#include "../config.hxx"
#include <graphics/curses.h>
#include <nuttx/config.h>
#include <system/termcurses.h>

namespace PowerFeed::Drivers
{

	ConsoleDisplay::ConsoleDisplay()
	{
	}

	void ConsoleDisplay::Init()
	{

		printf("Creating ConsoleDisplay\n");
		// 		int width;
		// 		int height;
		// #ifdef CONFIG_PDCURSES_MULTITHREAD
		// 		FAR struct pdc_context_s *ctx = PDC_ctx();
		// #endif

		// 		// termcurses_initterm();

		// 		// 		traceon();
		// 		initscr();

		// 		// 		start_color();
		// 		// #if defined(NCURSES_VERSION) || (defined(PDC_BUILD) && PDC_BUILD > 3000)
		// 		// 		use_default_colors();
		// 		// #endif
		// 		cbreak();
		// 		noecho();

		// 		// 		// curs_set(0);
		// 		// 		noecho();

		// 		/* Refresh stdscr so that reading from it will not cause it to overwrite
		// 		 * the other windows that are being created.
		// 		 */

		// 		refresh();

		// 		/* Create a drawing window */

		// 		width = static_cast<uint32_t>(132 / 6.0);
		// 		height = static_cast<uint32_t>(64 / 10.0);

		// 		myDefaultWindow = newwin(height, width, 0, 0);

		// 		assert(myDefaultWindow == nullptr);

		// 		// for (;;)
		// 		// {
		// 		// init_pair(1, COLOR_BLACK, COLOR_WHITE);
		// 		// wbkgd(myDefaultWindow, COLOR_BLACK);
		// 		werase(myDefaultWindow);

		// 		// init_pair(2, COLOR_RED, COLOR_RED);
		// 		// wattrset(myDefaultWindow, COLOR_WHITE);
		// 		// box(myDefaultWindow, ' ', ' ');
		// 		wrefresh(myDefaultWindow);

		// 		wattrset(myDefaultWindow, 0);

		// 		/* Do random output of a character */

		// 		// nodelay(stdscr, true);

		// 		mvwaddstr(myDefaultWindow, 1, 1, "POWER");
		// 		mvwaddstr(myDefaultWindow, 1, 16, "FEED");
		// 		wrefresh(myDefaultWindow);
	}

	void ConsoleDisplay::WriteBuffer()
	{
		wrefresh(myDefaultWindow);
	}

	void ConsoleDisplay::DrawText(const char *text, const NXHANDLE &aFont, uint16_t x, uint16_t y)
	{
		// pico_ssd1306::drawText(mySSD1306, font, text, x, y);
	}

	void ConsoleDisplay::DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t awidth, uint16_t aheight)
	{
		// mySSD1306->addBitmapImage(x, y, width, height, const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(image)));
	}

	void ConsoleDisplay::ClearBuffer()
	{
		werase(myDefaultWindow);
	}

} // namespace PowerFeed