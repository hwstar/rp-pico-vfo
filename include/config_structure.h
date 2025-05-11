#pragma once
#include <Arduino.h>
#define MAX_NUM_OF_BANDS 8

#define BAND_FLAG_ACTIVE 1
#define BAND_FLAG_CHANNELIZED_ONLY 2
#define BAND_FLAG_NO_MODE_SWITCH 4

typedef struct Band_Info {
    // Must be 64 bytes
    char name[8];
    uint32_t flags;
    uint32_t lower_limit;
    uint32_t upper_limit;
    int32_t txgain;
    uint8_t band_control_bits[4];
    uint8_t pad[36];
} Band_Info;

static_assert(sizeof(Band_Info) == 64, "Data structure doesn't match page size");

