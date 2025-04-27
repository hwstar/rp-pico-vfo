#pragma once

#include <Wire.h>

#define EEPROM_24CW640_PAGE_SIZE 32
#define EEPROM_24CW640_BYTE_COUNT 8192
#define EEPROM_24C640_PAGE_COUNT  (EEPROM_24CW640_BYTE_COUNT/EEPROM_24CW640_PAGE_SIZE)


class Eeprom24C640
{
    public:
    // Initialization
    void begin(MbedI2C *i2c_bus, int i2c_addr);

    // Test presence
    bool present();

    // Return result code from previous transaction
    int get_result();
   
    // Read a page of data (32 bytes)
    bool read_page(uint16_t page_num, void *buffer);

    // Write a page of data (32 bytes)
    bool write_page(uint16_t page_num, void *buffer);

    protected:
   
    uint8_t _slave_addr;
    uint8_t _result;

    //const uint8_t EEPROM_WRTE = 0x40;
    const uint32_t EEPROM_TIMEOUT = 10;

    MbedI2C *_i2c_bus;

};

