# RP_PICO_VFO
Firmware for Raspberry Pi Pico-based VFO board with a parallel 1602 Display

This is part of the Single Band SSB Radio hardware project.

The VFO code supports both free vfo and memory operation.  

This vfo code is configurable using the serial port at 115200 baud 8 bits No parity. 

Configuration is accomplished by using command keywords. 

## List of valid commands

Here's a list of all the configuration commands:

cal on 

Enable cal mode. Output 10 MHz on LO1 and LO2 outputs

cal off 

Disable cal mode, return to normal operation

cal set *offset* 

Integer: Offset is in parts per billion

cal get 

Return cal offset in parts per billion.

config set band channelized enable *band_number* 

Set channelized operation (memory mode) as the default at power on. Use a band number of 1

config set band channelized disable *band_number* 

Set vfo mode as the default on power up. Use a band number of 1

config set band disable *band_number* 

Disable band for use (Reserved for future use)

Disable the band so that it can't be selected. Band number from 1 to 8

config set band display offset *band_number* *offset_in_hz* 

Adjust the frequency displayed by an offset. (For channel centering on 60 Meters).
Use a band number of 1. Offset in Hz is an integer.

config set band enable *band_number* (Reserved for future use)

config set band lsb *band_number*

When the band is selected, use LSB as the default sideband. Use a band number of 1.

config set band modeselect enable *band_number* 

Enable LSB/USB mode selection in the menu system of the radio for a band. Use a band number of 1

config set band modeselect disable *band_number* 

Disable LSB/USB mode selection in the menu system of the radio for a band. Use a band number of 1

config set band name *band_number* *band_name_str* 

Set the band name using a name up to 4 characters long. Use a band number of 1. 

config set band start *band_number* *start_freq_in_hz* 

Configure the bottom frequency limit for a band. Use a band number of 1. Frequency is an integer expressed in Hz.

config set band stop *band_number* *stop_freq_in_hz* 

Configure the top frequency limit for a band. Use a band number of 1. Frequency is an integer expressed in Hz.

config set band step 500 *band_number* 

Configure the band step size to 500 Hz for the band. Use a band number of 1.

config set band step 1000 *band_number* 

Configure the band step size to 1 kHz for the band. Use a band number of 1.

config set band usb *band_number*

When the band is selected, use USB as the default sideband. Use a band number of 1.

config set channel frequency *channel_number* *frequency_in_hz* 

Set the frequency for a channel in Hz. Channel number is from 1 to 32.
Setting a frequency of 0 Hz disables the channel.

config set channel name *channel_number* *channel_name_string* 

Set the channel name for a channel. Channel name is a string which can contain up to 7 characters.

config set factory defaults 

Restore VFO settings to factory defaults. You will be prompted to confirm this choice.

config set radio if *if_frequency_in_hz* 

Set the radio IF frequency (Supressed Carrier Frequency) in Hz.

config set radio refosc *reference_oscillator_frequency_in_hz* 

Set the frequency for the oscillator used by the SI5351 chip. Frequency is in Hz.

config set radio bitx 

Configure LO1 and LO2 to not swap from RX to TX mode and vice versa. This is used when the radio IF is bi-directional.

config set radio swap 

Configure LO1 and LO2 to swap places when going from RX to TX and TX to RX. This is used when the radio IF is 
unidirectional.

config eeprom save 

Save all the configuration settings in RAM to the onboard EEPROM

help 

Display a condensed version of the command list.

info get band *band_number* 

Get band settings for a particular band. Band number should be 1.

info get channels 

Return a list of active channel names and numbers.

info get eeprom layout 

Return internal layout of data storage in EEPROM. For debugging use.

info get radio config 

Return radio configuration settings.

reboot 

Reboot firmware.


## Building

Requires platformio to build the firmware

See platformio.ini in this repository for the library requirements.
