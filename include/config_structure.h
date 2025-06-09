#pragma once
#include <Arduino.h>
#include "config_default.h"



// Band feature enable/disable flags
#define BAND_FLAG_ACTIVE 1 // Indicate that the band selection is active
#define BAND_FLAG_CHANNELIZED 2 // Use channels by default on this band
#define BAND_FLAG_MODE_USB 4 // Use USB by default on this band
#define BAND_FLAG_NO_MODE_SWITCH 8 // Don't allow the user to select USB or LSB (e.g. for 60 meters)
#define BAND_VFO_STEP_500HZ 0x10 // Step in 500 Hz increments when true, otherwise 1kHz

// Radio info enable/disable mode
#define RADIO_FLAG_BITX_MODE 1 // If set, do not swap LO and BFO on TX

typedef struct Band_Info {
    // Must be 64 bytes
    char name[8];
    uint32_t flags; // Band feature enable/disable flags
    uint32_t lower_limit; // Lower frequency limit for band in Hz
    uint32_t upper_limit; // Upper frequency limit for band in Hz
    int32_t txgain; // TX gain (reserved for future use in multiband radios)
    uint8_t band_control_bits[4]; // Band control bits for I2C expanders (reserved for future use) 
    int32_t freq_offset_display; // The amount to offset the frequency shown on the display (for showing channel center on 60 meters)
    uint8_t pad[32];
} Band_Info;

static_assert(sizeof(Band_Info) == 64, "Band_Info data structure doesn't match page size");

typedef struct Radio_Info {
    uint32_t if_zero_hz_freq; // IF Frequency of the SSB supressed carrier in Hz
    uint32_t ref_clk_freq; // Si5351 Reference Clock Frequency
    uint32_t flags; // Feature enable/disable flags
    uint8_t pad[52];
} Radio_Info;

static_assert(sizeof(Radio_Info) == 64, "Radio_Info Data structure doesn't match page size");

typedef struct Channel {
    uint32_t freq; // Frequency for of the channel
    uint32_t flags; // Channel feature flags
    uint8_t name[CONFIG_CHANNEL_NAME_SIZE]; // Channel Name
    uint8_t pad[8];
} Channel;

static_assert(sizeof(Channel) == 24, "Channel Data structure doesn't match page size");

typedef struct Channel_Info {
    uint8_t pad[4];
    Channel channel[CONFIG_MAX_NUM_CHANNELS];
} Channel_Info;

static_assert(sizeof(Channel_Info) == 772, "Channel_Info Data structure doesn't match page size");