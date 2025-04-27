
#include <Arduino.h>
#include <Wire.h>
#include <24cw640.h>

  
void Eeprom24C640::begin(MbedI2C *i2c_bus, int i2c_addr) 
{
    
    this->_i2c_bus = i2c_bus;
    this->_slave_addr = i2c_addr;

}


bool Eeprom24C640::present()
{
    uint8_t addr[2] = {0x80, 0x00};

    this->_i2c_bus->beginTransmission(this->_slave_addr);
    this->_i2c_bus->write(addr, 2);  // Send configuration register address
    if((this->_result = this->_i2c_bus->endTransmission(false)) != 0) // Restart for read
        return false;
    return true;
}

int Eeprom24C640::get_result()
{
    return this->_result;
}


bool Eeprom24C640::read_page(uint16_t page_num, void *buffer)
{ 
    uint8_t *b = (uint8_t *) buffer;  
    uint8_t addr[2];
    uint8_t index;
    uint16_t page_base_addr = (page_num * EEPROM_24CW640_PAGE_SIZE) & (EEPROM_24CW640_BYTE_COUNT - 1);

    // Calculate the address bytes
    addr[1] = (uint8_t) (page_base_addr & 0xFF);
    addr[0] = (uint8_t) (page_base_addr >> 8);

    // Send the page address
    this->_i2c_bus->beginTransmission(this->_slave_addr); // Send page base address
    this->_i2c_bus->write(addr, 2);
    if((this->_result = this->_i2c_bus->endTransmission(false)) != 0) // Restart for read
        return false;

    // Request the page
    this->_i2c_bus->requestFrom(this->_slave_addr, EEPROM_24CW640_PAGE_SIZE);
   
    // Capture the read bytes as they become available
    for(index = 0; index < EEPROM_24CW640_PAGE_SIZE; index++){
        while(!this->_i2c_bus->available());
        b[index] = this->_i2c_bus->read();
    }
    return true;
}

bool Eeprom24C640::write_page(uint16_t page_num, void *buffer)
{
    uint8_t *b = (uint8_t *) buffer;  
    uint8_t addr[2];
    uint16_t page_base_addr = (page_num * EEPROM_24CW640_PAGE_SIZE) & (EEPROM_24CW640_BYTE_COUNT - 1);
    // Calculate the address bytes
    addr[1] = (uint8_t) (page_base_addr & 0xFF);
    addr[0] = (uint8_t) (page_base_addr >> 8);


    // Perform write transaction
    this->_i2c_bus->beginTransmission(this->_slave_addr); // Send page base address
    this->_i2c_bus->write(addr, 2);
    this->_i2c_bus->write(b, EEPROM_24CW640_PAGE_SIZE);
    if((this->_result = this->_i2c_bus->endTransmission()) != 0) 
        return false;
    // Wait for write to finish
    delay(10);

    return true;

}

