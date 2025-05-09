#include "persistent_storage.h"

const char *HEADER_ID = "Persistent Storage";
const uint16_t HEADER_START = 0;
const uint16_t DICTIONARY_START = EEPROM_24CW640_PAGE_SIZE;
const uint16_t DICTIONARY_SIZE = (63 * EEPROM_24CW640_PAGE_SIZE);
const uint16_t OFFSET_START (64 * EEPROM_24CW640_PAGE_SIZE);
const uint16_t OFFSET_SIZE (32 * EEPROM_24CW640_PAGE_SIZE);
const uint16_t OFFSET_COUNT (OFFSET_SIZE / sizeof(uint16_t));
const uint16_t DATA_START (96 * EEPROM_24CW640_PAGE_SIZE);
const uint16_t DATA_SIZE (160 * EEPROM_24CW640_PAGE_SIZE);
const uint32_t STORAGE_VERSION = 0;

typedef struct StorageHeader  {
    char header_id[20];
    uint8_t pad[8];
    uint32_t storage_version;
} StorageHeader;

bool PersistentStorage::_read_all_pages() {
    bool res = false;
    for(uint16_t i = 0; i < EEPROM_24C640_PAGE_COUNT; i++) {
        uint8_t *p = this->_contents + (i * EEPROM_24CW640_PAGE_SIZE);
        res = this->_eeprom->read_page(i, p);
        if(!res) {
            break;
        }

    }

    return res;
}

bool PersistentStorage::_find_index_by_key(const char *key, uint16_t *index) {
    char *dictionary =  (char *) this->_contents + DICTIONARY_START;
    bool res = true;
    uint16_t i;

    if(!key || !index) {
        this->_result = false;
        return this->_result;
    }

    *index = 0;
    // Strings are placed in the dictionary nul terminated.
    // The end of the dictionary is denoted by seeing 2 nul characters next to each other
    for(i = 0; res;) {
        if(dictionary[i]) {
            if(!strncmp(key, dictionary + i, strlen(key)))
                break; // Found it
            else {
                // Get past string nul terminator to next string in dictionary
                while(dictionary[i]) {
                    i++;
                    // Check to see we don't exceed the size of the dictionary
                    if(i >= DICTIONARY_SIZE) {
                        res = false;
                        break;
                    }
                }
                if(res) {
                    // Move past the last strings nul terminator to the next string
                    i++;
                    (*index)++; // Bump index for each string checked.
                    // Check to see we don't exceed the size of the dictionary
                    if(i >= DICTIONARY_SIZE) {
                        res = false;
                    }
                }
            }
        }
        else {
            res = false; // Last dictionary key reached
        }
    }

    return res;
}

bool PersistentStorage::_get_offset(uint16_t index, uint16_t *offset) {
    uint16_t *offset_table = (uint16_t *) (this->_contents + OFFSET_START);
    uint16_t i;
    
    if(!offset || (index > OFFSET_COUNT)) {
        return false;
    }
    
    *offset = 0;
    // Sum all lengths in offset table until we reach the desired index
    for(i = 0; i < OFFSET_COUNT; i++) {
        if(i == index) {
            break;
        }
        *offset += offset_table[i];
    }
    if(i >= OFFSET_COUNT) {
        return false;
    }

    return true;
}

bool PersistentStorage::begin(Eeprom24C640 *eeprom) {
    this->_eeprom = eeprom;
    this->_viable = false;
    this->_dirty = false;

    if(!eeprom) {
        this->_result = false;
        return this->_result;
    }

    this->_result = this->_eeprom->present();
 
    if(this->_result) {
        this->_result = this->_read_all_pages();
    }
 
    return this->_result;
}

bool PersistentStorage::validate_contents() {
    StorageHeader *sh = (StorageHeader *) this->_contents;

    this->_viable = false;

    if(sh->storage_version != STORAGE_VERSION) {
        this->_result = false;
        return this->_result;
    }

    this->_result = (strncmp((char * ) sh->header_id, HEADER_ID, strlen(HEADER_ID)) == 0);
    if(this->_result) {
        this->_viable = true;
    }

    return this-> _result;
}


void PersistentStorage::format() {
    StorageHeader *sh = (StorageHeader *) this->_contents;

    memset(this->_contents, 0, EEPROM_24CW640_BYTE_COUNT);
    memcpy(sh->header_id, HEADER_ID, strlen(HEADER_ID));
    sh->storage_version = STORAGE_VERSION;

    this->_dirty = true;
    this->_viable = true;
}

bool PersistentStorage::add_key(const char *key, uint16_t storage_size) {
    uint16_t i,index = 0;
    char *dictionary =  (char *) this->_contents + DICTIONARY_START;
    uint16_t *offset_table = (uint16_t *) (this->_contents + OFFSET_START);

    if(!this->_viable) {
        this->_result = false;
        return this->_result;
    }
    this->_result = true;  

    // Look for free space in the dictionary
    for(i = 0; this->_result;) {
        if(dictionary[i]) {
            while(dictionary[i]) {
                i++;
                // Check to see we don't exceed the size of the dictionary
                if(i >= DICTIONARY_SIZE) {
                    // No more space in dictionary
                    this->_result = false;
                    break;
                }
            }
            if(this->_result) {
                index++; // Bump index
                i++; // Move past nul character to next string in the dictionary
            }
        }
        else {
            break;
        }
    }
    // Check for errors which might have occured during the search for dictionary free space
    if(!this->_result) {
        return this->_result;
    }
    
    // Check for an available slot in the offset table
    if(index >= OFFSET_COUNT) {
        this->_result = false;
        return this->_result; 
    }
   

    uint16_t free_space = DICTIONARY_SIZE - i;
    uint16_t key_length = 1 + strlen(key);
    if(free_space > key_length) {
        // Copy the key into the dictionary
        char *d = dictionary + i;
        strncpy(d, key, key_length);
    }
    else {
        // Not enough space to store the key
        this->_result = false;
        return this->_result;
    }

    // Place the storage size of the value into the offset table
    offset_table[index] = storage_size;
    
    // Set dirty flag
    this->_dirty = true;

    return this->_result;
}

bool PersistentStorage::read(const char *key, uint32_t *value) {
    if(!this->_viable) {
        this->_result = false;
        return this->_result;
    }
    if(!key || !value) {
        this->_result = false;
        return this->_result;
    }
    this->_result = true;
    // Find the index from the key
    uint16_t index;
    this->_result = this->_find_index_by_key(key, &index);
    if(!this->_result) {
        return this->_result;
    }

    // Check to see that the size stored in the offset table matches the the size of the type
    uint16_t *offset_table = (uint16_t *) (this->_contents + OFFSET_START);

    if(offset_table[index] != sizeof(uint32_t)){
        this->_result = false;

    }
    // Retreive the offset into the block of data
    uint16_t data_offset;
    this->_result = this->_get_offset(index, &data_offset);
    if(!this->_result) {
        return this->_result;
    }
    // Retrieve the value and place it in the memory location supplied by caller
    *value = *(uint32_t *) (this->_contents + DATA_START + data_offset);


    return this-> _result;
}
bool PersistentStorage::read(const char *key, int32_t *value) {  
    if(!this->_viable) {
        this->_result = false;
        return this->_result;
    }
    if(!key || !value) {
        this->_result = false;
        return this->_result;
    }
    this->_result = true;
    // Find the index from the key
    uint16_t index;
    this->_result = this->_find_index_by_key(key, &index);
    if(!this->_result) {
        return this->_result;
    }

    // Check to see that the size stored in the offset table matches the the size of the type
    uint16_t *offset_table = (uint16_t *) (this->_contents + OFFSET_START);

    if(offset_table[index] != sizeof(int32_t)){
        this->_result = false;

    }
    // Retreive the offset into the block of data
    uint16_t data_offset;
    this->_result = this->_get_offset(index, &data_offset);
    if(!this->_result) {
        return this->_result;
    }
    // Retrieve the value and place it in the memory location supplied by caller
    *value = *(int32_t *) (this->_contents + DATA_START + data_offset);


    return this-> _result;
}

bool PersistentStorage::write(const char *key, uint32_t value) {
    if(!this->_viable) {
        this->_result = false;
        return this->_result;
    }
    if(!key) {
        this->_result = false;
        return this->_result;
    }
    this->_result = true;
    // Find the index from the key
    uint16_t index;
    this->_result = this->_find_index_by_key(key, &index);
    if(!this->_result) {
        return this->_result;
    }

    // Check to see that the size stored in the offset table matches the the size of the type
    uint16_t *offset_table = (uint16_t *) (this->_contents + OFFSET_START);

    if(offset_table[index] != sizeof(uint32_t)){
        this->_result = false;

    }
    // Retreive the offset into the block of data
    uint16_t data_offset;
    this->_result = this->_get_offset(index, &data_offset);
    if(!this->_result) {
        return this->_result;
    }
    // Store the value in RAM
    uint8_t *value_region  = (this->_contents + DATA_START);
    *(uint32_t *) (value_region + data_offset) = value;

    // Set the dirty bit
    this->_dirty = true;

    return this->_result;
}


bool PersistentStorage::write(const char *key, int32_t value) {
    if(!this->_viable) {
        this->_result = false;
        return this->_result;
    }
    if(!key) {
        this->_result = false;
        return this->_result;
    }
    this->_result = true;
    // Find the index from the key
    uint16_t index;
    this->_result = this->_find_index_by_key(key, &index);
    if(!this->_result) {
        return this->_result;
    }

    // Check to see that the size stored in the offset table matches the the size of the type
    uint16_t *offset_table = (uint16_t *) (this->_contents + OFFSET_START);

    if(offset_table[index] != sizeof(int32_t)){
        this->_result = false;

    }
    // Retreive the offset into the block of data
    uint16_t data_offset;
    this->_result = this->_get_offset(index, &data_offset);
    if(!this->_result) {
        return this->_result;
    }
    // Store the value in RAM
    uint8_t *value_region  = (this->_contents + DATA_START);
    *(int32_t *) (value_region + data_offset) = value;

    // Set the dirty bit
    this->_dirty = true;

    return this->_result;


}

bool PersistentStorage::commit() {
    if(!this->_viable) {
        this->_result = false;
        return this->_result;
    }

    this->_result = true;

    // Write all pages to eeprom

    if(this->_dirty) {
        Serial1.println();
        Serial1.print("Writing EEProm...");
        for(uint16_t i = 0; i < EEPROM_24C640_PAGE_COUNT; i++) {
            uint8_t *p = this->_contents + (i * EEPROM_24CW640_PAGE_SIZE);
            this->_result = this->_eeprom->write_page(i, p);
            if(!this->_result) {
                break;
            }
        }
        this->_dirty = false;
        Serial1.println("Done");
    }
    return this-> _result;
}

