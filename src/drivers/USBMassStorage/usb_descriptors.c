#include "tusb.h"
#include "device/usbd.h"

// Device descriptor
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    
    .idVendor           = 0x2E8A,  // Raspberry Pi
    .idProduct          = 0x000A,  // Mass Storage
    .bcdDevice          = 0x0100,
    
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    
    .bNumConfigurations = 0x01
};

// Configuration descriptor
enum
{
    ITF_NUM_MSC = 0,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

uint8_t const desc_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    
    // Interface descriptor + MSC descriptor
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 4, BOARD_TUD_RHPORT, 0x40, 0x01),
};

// String Descriptors
char const* string_desc_arr[] =
{
    (const char[]) { 0x09, 0x04 },  // 0: Supported language is English (0x0409)
    "PowerFeed",                     // 1: Manufacturer
    "Config",                // 2: Product
    "123456",                        // 3: Serial number
    "Config Drive"                   // 4: MSC interface name
};

// USB HID Report Descriptor
uint8_t const desc_hid_report[] =
{
    0x00
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application returns pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &desc_device;
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application returns pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index;
    return desc_configuration;
}

// Invoked when received GET STRING DESCRIPTOR request
// Application returns pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    static uint16_t desc_str[32];
    uint8_t chr_count;

    if (index == 0)
    {
        memcpy(&desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        // Convert ASCII to UTF-16
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
            return NULL;

        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        for (uint8_t i = 0; i < chr_count; i++)
        {
            desc_str[1+i] = str[i];
        }
    }

    // First byte is length (including header), second byte is string type
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return desc_str;
}