#pragma once
#include <Arduino.h>
#define MAX_NUM_OF_BANDS 8

#define BAND_FLAG_ACTIVE 1
#define BAND_FLAG_CHANNELIZED_ONLY 2
#define BAND_FLAG_MODE_USB 4
#define BAND_FLAG_NO_MODE_SWITCH 8

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

static_assert(sizeof(Band_Info) == 64, "Band_Info data structure doesn't match page size");

typedef struct Radio_Info {
    uint32_t if_zero_hz_freq;
    uint32_t ref_clk_freq;
    uint8_t pad[56];
} Radio_Info;

static_assert(sizeof(Radio_Info) == 64, "Radio_Info Data structure doesn't match page size");

typedef struct Channel {
    uint32_t freq;
    uint32_t flags;
    uint8_t pad[8];
} Channel;

typedef struct Channel_Info {
    uint32_t channel_enables;
    Channel channel[32];
} Channel_Info;

static_assert(sizeof(Channel_Info) == 516, "Channel_Info Data structure doesn't match page size");