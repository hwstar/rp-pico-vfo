#pragma once
#include <Arduino.h>


/* INPUTS */

#define GPIO_PTT_BUTTON p2
#define GPIO_TUNE_BUTTON p3
#define GPIO_ENCODER_SWITCH p8
#define GPIO_BAND_BUTTON p9
#define GPIO_ENCODER_I p14
#define GPIO_ENCODER_Q p15

/* OUTPUTS */

#define GPIO_PTT_OUT p4
#define GPIO_TUNE_OUT p5
#define GPIO_MUTE_OUT p6
#define GPIO_AGC_DISABLE p7
#define GPIO_LCD_RS p16
#define GPIO_LCD_E p17
#define GPIO_LCD_DB4 p18
#define GPIO_LCD_DB5 p19
#define GPIO_LCD_DB6 p20
#define GPIO_LCD_DB7 p21
#define GPIO_LCD_BACKLIGHT p22

/* ADC */
#define GPIO_ANALOG_VMON p28
#define GPIO_ANALOG_SMETER p26

/* I2C */

#define GPIO_I2C_SDA1 p10
#define GPIO_I2C_SCL1 p11
#define GPIO_I2C_SDA0 p12
#define GPIO_I2C_SCL0 p13

/* UART */

#define GPIO_UART_TX p0
#define GPIO_UART_RX p1


