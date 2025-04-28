#pragma once
#include "24cw640.h"


class PersistentStorage {
    public:
    bool begin(Eeprom24C640 *eeprom);
    bool validate_contents();
    void format();

    bool add_key(const char *key, uint16_t storage_size = sizeof(uint32_t));

    bool read(const char *key, uint32_t *value);
    bool read(const char *key, int32_t *value);

    bool write(const char *key, uint32_t value);
    bool write(const char *key, int32_t value);

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
