#include "PicoSSD1306Display.hxx"
#include "Assert.hxx"
#include "Common.hxx"
#include "Helpers.hxx"
#include "Settings.hxx"
#include "config.h"
#include <cstdint>
#include <cstring>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/timer.h>
#include <memory>
#include <pico/time.h>
#include <stdio.h>

namespace PowerFeed::Drivers
{
    // SSD1306 commands
    #define SSD1306_SET_MEM_MODE        0x20
    #define SSD1306_SET_COL_ADDR        0x21
    #define SSD1306_SET_PAGE_ADDR       0x22
    #define SSD1306_SET_HORIZ_SCROLL    0x26
    #define SSD1306_SET_SCROLL          0x2E

    #define SSD1306_SET_DISP_START_LINE 0x40

    #define SSD1306_SET_CONTRAST        0x81
    #define SSD1306_SET_CHARGE_PUMP     0x8D

    #define SSD1306_SET_SEG_REMAP       0xA0
    #define SSD1306_SET_ENTIRE_ON       0xA4
    #define SSD1306_SET_ALL_ON          0xA5
    #define SSD1306_SET_NORM_DISP       0xA6
    #define SSD1306_SET_INV_DISP        0xA7
    #define SSD1306_SET_MUX_RATIO       0xA8
    #define SSD1306_SET_DISP            0xAE
    #define SSD1306_SET_COM_OUT_DIR     0xC0
    #define SSD1306_SET_COM_OUT_DIR_FLIP 0xC0

    #define SSD1306_SET_DISP_OFFSET     0xD3
    #define SSD1306_SET_DISP_CLK_DIV    0xD5
    #define SSD1306_SET_PRECHARGE       0xD9
    #define SSD1306_SET_COM_PIN_CFG     0xDA
    #define SSD1306_SET_VCOM_DESEL      0xDB

    #define SSD1306_WRITE_MODE         0xFE
    #define SSD1306_READ_MODE          0xFF

    // I2C timeout in microseconds
    #define I2C_TIMEOUT_US 10000
    
    #define SSD1306_HEIGHT 64
    #define SSD1306_WIDTH 128
    #define SSD1306_PAGE_HEIGHT 8
    #define SSD1306_NUM_PAGES (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
    #define SSD1306_BUF_LEN (SSD1306_NUM_PAGES * SSD1306_WIDTH)

    PicoSSD1306Display::PicoSSD1306Display(SettingsManager *aSettings) 
        : Display(aSettings, nullptr), // We'll implement our own font handling
          mySettings(aSettings),
          myWidth(SSD1306_WIDTH),
          myHeight(SSD1306_HEIGHT),
          myBufferLen(SSD1306_BUF_LEN)
    {
        // Initialize the display
        auto settings = aSettings->Get();
        if (settings == nullptr)
        {
            Panic("PicoSSD1306Display: Failed to load settings\n");
            return;
        }

        auto display = settings->display;
        myDisplayAddress = display.ssd1306Address;

        if (display.i2cMasterNum == 0)
        {
            myI2CMaster = i2c0;
        }
        else if (display.i2cMasterNum == 1)
        {
            myI2CMaster = i2c1;
        }
        else
        {
            Panic("PicoSSD1306Display: Invalid i2c master number\n");
            return;
        }

        uint sda = display.i2cMasterSdaIo;
        uint scl = display.i2cMasterSclIo;

        if (sda + 1 != scl)
        {
            Panic("PicoSSD1306Display: SDA and SCL pins must be adjacent\n");
            return;
        }

        // Init i2c controller
        i2c_init(myI2CMaster, 400 * 1000); // 400 kHz
        
        // Set up pins for I2C
        gpio_set_function(sda, GPIO_FUNC_I2C);
        gpio_set_function(scl, GPIO_FUNC_I2C);
        gpio_pull_up(sda);
        gpio_pull_up(scl);

        sleep_ms(1); // Wait for I2C to settle
        
        // Check if display is present on I2C bus using a timeout
        if (!CheckDeviceResponsive()) {
            printf("PicoSSD1306Display: Display not detected on I2C bus, address 0x%02x\n", myDisplayAddress);
            myIsReady = false;
            return;
        }
        
        // Allocate frame buffer
        myFrameBuffer = new uint8_t[myBufferLen];
        if (myFrameBuffer == nullptr) {
            Panic("PicoSSD1306Display: Failed to allocate frame buffer\n");
            return;
        }
        
        // Clear buffer
        memset(myFrameBuffer, 0, myBufferLen);
        
        // Initialize display
        if (!InitDisplay()) {
            printf("PicoSSD1306Display: Failed to initialize display\n");
            myIsReady = false;
            return;
        }
        
        myIsReady = true;
    }

    PicoSSD1306Display::~PicoSSD1306Display()
    {
        if (myFrameBuffer) {
            delete[] myFrameBuffer;
            myFrameBuffer = nullptr;
        }
    }

    bool PicoSSD1306Display::CheckDeviceResponsive()
    {
        // Simple check if device is responsive using a timeout to avoid hanging
        uint8_t rxdata;
        int ret = i2c_read_timeout_us(myI2CMaster, myDisplayAddress, &rxdata, 1, false, I2C_TIMEOUT_US);
        return ret >= 0;
    }

    bool PicoSSD1306Display::InitDisplay()
    {
        // Send command to SSD1306 with timeout
        auto send_cmd = [this](uint8_t cmd) -> bool {
            uint8_t buf[2] = {0x00, cmd}; // First byte is command byte (0x00)
            int ret = i2c_write_timeout_us(myI2CMaster, myDisplayAddress, buf, 2, false, I2C_TIMEOUT_US);
            return ret > 0;
        };

        bool success = true;
        
        // Basic initialization sequence for SSD1306
        uint8_t cmds[] = {
            SSD1306_SET_DISP,               // Set display off
            SSD1306_SET_MEM_MODE,           // Set memory address mode
            0x00,                           // Horizontal addressing mode
            SSD1306_SET_DISP_START_LINE,    // Set display start line
            SSD1306_SET_SEG_REMAP | 0x01,   // Set segment re-map
            SSD1306_SET_MUX_RATIO,          // Set multiplex ratio
            (uint8_t)(myHeight - 1),        // Display height - 1
            SSD1306_SET_COM_OUT_DIR | 0x08, // Set COM output scan direction
            SSD1306_SET_DISP_OFFSET,        // Set display offset
            0x00,                           // No offset
            SSD1306_SET_COM_PIN_CFG,        // Set COM pins hardware configuration
#if (SSD1306_HEIGHT == 32)
            0x02,                           // For 128x32 display
#else
            0x12,                           // For 128x64 display
#endif
            SSD1306_SET_DISP_CLK_DIV,       // Set display clock divide ratio
            0x80,                           // Set divide ratio
            SSD1306_SET_PRECHARGE,          // Set pre-charge period
            0xF1,                           // Pre-charge period (both phases)
            SSD1306_SET_VCOM_DESEL,         // Set VCOMH deselect level
            0x30,                           // 0.83xVCC
            SSD1306_SET_CONTRAST,           // Set contrast control
            0xFF,                           // Max contrast
            SSD1306_SET_ENTIRE_ON,          // Disable entire display on
            SSD1306_SET_NORM_DISP,          // Normal display (not inverted)
            SSD1306_SET_CHARGE_PUMP,        // Enable charge pump regulator
            0x14,                           // Enable charge pump
            SSD1306_SET_SCROLL | 0x00,      // Deactivate horizontal scrolling
            SSD1306_SET_DISP | 0x01,        // Turn display on
        };

        // Send all commands to the display
        for (unsigned int i = 0; i < sizeof(cmds); i++) {
            if (!send_cmd(cmds[i])) {
                success = false;
                printf("PicoSSD1306Display: Command %d failed\n", i);
                break;
            }
        }

        // Set orientation based on settings
        if (success && mySettings->Get()->display.ssd1306Rotate180) {
            success &= send_cmd(SSD1306_SET_SEG_REMAP); // Normal orientation
            success &= send_cmd(SSD1306_SET_COM_OUT_DIR); // Normal orientation
        }

        return success;
    }

    // Set a single pixel in the buffer
    static void SetPixel(uint8_t *buf, int x, int y, bool on) {
        if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT)
            return;
            
        int byte_idx = (y / 8) * SSD1306_WIDTH + x;
        uint8_t byte = buf[byte_idx];
        
        if (on)
            byte |= 1 << (y % 8);
        else
            byte &= ~(1 << (y % 8));
            
        buf[byte_idx] = byte;
    }

    // Font data copied from pico-examples
    static uint8_t ssd1306_font[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Nothing
        0x78, 0x14, 0x12, 0x11, 0x12, 0x14, 0x78, 0x00, //A
        0x7f, 0x49, 0x49, 0x49, 0x49, 0x49, 0x7f, 0x00, //B
        0x7e, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x00, //C
        0x7f, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7e, 0x00, //D
        0x7f, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x00, //E
        0x7f, 0x09, 0x09, 0x09, 0x09, 0x01, 0x01, 0x00, //F
        0x7f, 0x41, 0x41, 0x41, 0x51, 0x51, 0x73, 0x00, //G
        0x7f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7f, 0x00, //H
        0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, //I
        0x21, 0x41, 0x41, 0x3f, 0x01, 0x01, 0x01, 0x00, //J
        0x00, 0x7f, 0x08, 0x08, 0x14, 0x22, 0x41, 0x00, //K
        0x7f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, //L
        0x7f, 0x02, 0x04, 0x08, 0x04, 0x02, 0x7f, 0x00, //M
        0x7f, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7f, 0x00, //N
        0x3e, 0x41, 0x41, 0x41, 0x41, 0x41, 0x3e, 0x00, //O
        0x7f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e, 0x00, //P
        0x3e, 0x41, 0x41, 0x49, 0x51, 0x61, 0x7e, 0x00, //Q
        0x7f, 0x11, 0x11, 0x11, 0x31, 0x51, 0x0e, 0x00, //R
        0x46, 0x49, 0x49, 0x49, 0x49, 0x30, 0x00, 0x00, //S
        0x01, 0x01, 0x01, 0x7f, 0x01, 0x01, 0x01, 0x00, //T
        0x3f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3f, 0x00, //U
        0x0f, 0x10, 0x20, 0x40, 0x20, 0x10, 0x0f, 0x00, //V
        0x7f, 0x20, 0x10, 0x08, 0x10, 0x20, 0x7f, 0x00, //W
        0x00, 0x41, 0x22, 0x14, 0x14, 0x22, 0x41, 0x00, //X
        0x01, 0x02, 0x04, 0x78, 0x04, 0x02, 0x01, 0x00, //Y
        0x41, 0x61, 0x59, 0x45, 0x43, 0x41, 0x00, 0x00, //Z
        0x3e, 0x41, 0x41, 0x49, 0x41, 0x41, 0x3e, 0x00, //0
        0x00, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x00, 0x00, //1
        0x30, 0x49, 0x49, 0x49, 0x49, 0x46, 0x00, 0x00, //2
        0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00, //3
        0x3f, 0x20, 0x20, 0x78, 0x20, 0x20, 0x00, 0x00, //4
        0x4f, 0x49, 0x49, 0x49, 0x49, 0x30, 0x00, 0x00, //5
        0x3f, 0x48, 0x48, 0x48, 0x48, 0x48, 0x30, 0x00, //6
        0x01, 0x01, 0x01, 0x61, 0x31, 0x0d, 0x03, 0x00, //7
        0x36, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00, //8
        0x06, 0x09, 0x09, 0x09, 0x09, 0x09, 0x7f, 0x00, //9
    };

    // Get font index from character
    static inline int GetFontIndex(uint8_t ch) {
        if (ch >= 'A' && ch <= 'Z') {
            return ch - 'A' + 1;
        }
        else if (ch >= 'a' && ch <= 'z') {
            return ch - 'a' + 1; // Same as uppercase
        }
        else if (ch >= '0' && ch <= '9') {
            return ch - '0' + 27;
        }
        else return 0; // Not supported, use space
    }

    void PicoSSD1306Display::DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y)
    {
        if (!myIsReady || !myFrameBuffer) {
            return;
        }

        // For simple implementation, force y to nearest page boundary
        y = (y / 8) * 8;

        // Render each character from the text
        while (*text) {
            uint8_t ch = (uint8_t)*text++;
            int idx = GetFontIndex(ch);
            
            // Copy 8 bytes for this character
            for (int i = 0; i < 8; i++) {
                if (x + i < SSD1306_WIDTH) {
                    int fb_idx = (y / 8) * SSD1306_WIDTH + x + i;
                    myFrameBuffer[fb_idx] = ssd1306_font[idx * 8 + i];
                }
            }
            
            x += 8; // Move to next character position
            
            // Break if we're past the edge of the display
            if (x >= SSD1306_WIDTH) {
                break;
            }
        }
    }

    void PicoSSD1306Display::DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        if (!myIsReady || !myFrameBuffer) {
            return;
        }

        // Simple raw image drawing (monochrome)
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                // Check if this bit is set in the source image
                int byte_idx = (j * width + i) / 8;
                int bit_pos = 7 - ((j * width + i) % 8);
                bool pixel = (image[byte_idx] & (1 << bit_pos)) != 0;
                
                SetPixel(myFrameBuffer, x + i, y + j, pixel);
            }
        }
    }

    void PicoSSD1306Display::ClearBuffer()
    {
        if (!myIsReady || !myFrameBuffer) {
            return;
        }

        memset(myFrameBuffer, 0, myBufferLen);
    }

    void PicoSSD1306Display::WriteBuffer()
    {
        if (!myIsReady || !myFrameBuffer) {
            return;
        }

        // First send the commands to set up the display area
        uint8_t cmds[] = {
            SSD1306_SET_COL_ADDR,
            0,                              // Start column
            (uint8_t)(myWidth - 1),         // End column
            SSD1306_SET_PAGE_ADDR,
            0,                              // Start page
            (uint8_t)(SSD1306_NUM_PAGES - 1) // End page
        };

        for (unsigned int i = 0; i < sizeof(cmds); i++) {
            uint8_t buf[2] = {0x00, cmds[i]};
            int ret = i2c_write_timeout_us(myI2CMaster, myDisplayAddress, buf, 2, false, I2C_TIMEOUT_US);
            if (ret < 0) {
                printf("PicoSSD1306Display: Failed to send command %d\n", i);
                return;
            }
        }

        // Now send the frame buffer
        // We need to create a temporary buffer with a control byte at the start
        uint8_t *temp_buf = new uint8_t[myBufferLen + 1];
        if (!temp_buf) {
            printf("PicoSSD1306Display: Failed to allocate temp buffer\n");
            return;
        }

        temp_buf[0] = 0x40; // Control byte: Co=0, D/C=1 (data)
        memcpy(temp_buf + 1, myFrameBuffer, myBufferLen);

        int ret = i2c_write_timeout_us(myI2CMaster, myDisplayAddress, temp_buf, myBufferLen + 1, false, I2C_TIMEOUT_US);
        if (ret < 0) {
            printf("PicoSSD1306Display: Failed to send frame buffer\n");
        }

        delete[] temp_buf;
    }

    void PicoSSD1306Display::Refresh()
    {
        WriteBuffer();
    }
} // namespace PowerFeed::Drivers