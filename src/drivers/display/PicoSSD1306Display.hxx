#pragma once

#include "Display.hxx"
#include "Settings.hxx"
#include <hardware/i2c.h>
#include <memory>

namespace PowerFeed::Drivers
{
    class PicoSSD1306Display : public Display
    {
    public:
        PicoSSD1306Display(SettingsManager *settings);
        ~PicoSSD1306Display();
        
        void DrawText(const char *text, const unsigned char *font, uint16_t x, uint16_t y) override;
        void DrawImage(const unsigned char *image, uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
        void ClearBuffer() override;
        void WriteBuffer() override;
        void Refresh() override;

    private:
        bool InitDisplay();
        bool CheckDeviceResponsive();
        
        SettingsManager *mySettings;
        i2c_inst_t *myI2CMaster;
        uint8_t myDisplayAddress;
        bool myIsReady = false;
        uint8_t *myFrameBuffer = nullptr;
        int myHeight;
        int myWidth;
        int myBufferLen;
    };

}