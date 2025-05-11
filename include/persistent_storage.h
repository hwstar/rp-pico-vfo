#pragma once
#include "24cw640.h"


class PersistentStorage {
    public:
    // Function to initalize the persistent storage module. Call during initialization
    bool begin(Eeprom24C640 *eeprom);
    // Validate the contents of the persistent storage 
    bool validate_contents();
    // Format the persistant storage EEPROM
    void format();
    // Print keys, lengths and offsets ( for debugging)
    void print_eeprom_info();

    bool add_key(const char *key, uint16_t storage_size = sizeof(uint32_t));

    // Read functions for individual data types
    bool read(const char *key, uint32_t *value);
    bool read(const char *key, int32_t *value);
   
    // Write functions for individual data types
    bool write(const char *key, uint32_t value);
    bool write(const char *key, int32_t value);
   
    // Return a pointer to a region in the persistent storage for a specific key
    // Useful for strings and data structures.
    void *get_value_pointer(const char *key);

    // Signal that the RAM contents are now different than what is stored on the EEPROM
    void force_dirty() {
        this->_dirty = true;
    }

    // Check to see if the RAM contents have been modified
    bool is_dirty() {
        return this->_dirty;
    }
    // Write contents in RAM to the EEPROM only if there have been changes to the RAM contents
    bool commit();



    protected:
    bool _read_all_pages();
    bool _find_index_by_key(const char *key, uint16_t *index);
    bool _get_offset(uint16_t index, uint16_t *offset);

    bool _result;
    bool _dirty;
    bool _viable;
    Eeprom24C640 *_eeprom;
    uint8_t _contents[EEPROM_24CW640_BYTE_COUNT];
 
};
