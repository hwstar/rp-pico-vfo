#pragma once

#define CONFIG_DEFAULT_IF_ZERO_HZ_FREQ 12288007 /* 12.288007 MHz */
#define CONFIG_DEFAULT_REF_CLK_FREQ 26000000 /* 26 MHz */
#define CONFIG_DEFAULT_REF_CLK_CAL -1000 /* Reference oscillator calibration */
#define CONFIG_DEFAULT_STEP_SIZE 1000 /* Tuning knob step size  in hz */
#define CONFIG_DEFAULT_INITIAL_BAND_INDEX 0 /* Band table index selected at power up */
#define CONFIG_DEFAULT_STEP_SIZE_INDEX 3 /* initial step size index into step size table */

/* First band table entry */
#define CONFIG_DEFAULT_BAND_NAME_0 "40M" /* 40 Meters */
#define CONFIG_DEFAULT_BAND_LOWER_LIMIT_HZ_0 7128000 /* Lower limit 7.128 MHz */
#define CONFIG_DEFAULT_BAND_UPPER_LIMIT_HZ_0 7297000 /* Upper limit 7.297 MHz */
#define CONFIG_DEFAULT_BAND_INITIAL_FREQUENCY_0 7200000 /* Initial frequency at power on */
#define CONFIG_DEFAULT_BAND_SIDEBAND_0 false /* Lower sideband */
