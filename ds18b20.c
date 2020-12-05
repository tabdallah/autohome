/*
Reference: https://www.iot-programmer.com/index.php/books/22-raspberry-pi-and-the-iot-in-c/chapters-raspberry-pi-and-the-iot-in-c/36-raspberry-pi-and-the-iot-in-c-one-wire-basics?start=1 
*/
#include <stdio.h>
#include <bcm2835.h>
#include "ds18b20.h"

// Define maximum sensor read attempts
#define MAX_READ 1

uint8_t crc8(uint8_t *data, uint8_t len) {
    uint8_t i;
    uint8_t j;
    uint8_t temp;
    uint8_t databyte;
    
    uint8_t crc = 0;
    for (i = 0; i < len; i++) {
        databyte = data[i];
        for (j = 0; j < 8; j++) {
            temp = (crc ^ databyte) & 0x01;
            crc >>= 1;
            if (temp)
                crc ^= 0x8C;
            databyte >>= 1;
        }
    }
    return crc;
}

void writeBit(uint8_t pin,int b)
{
    int delay1, delay2;
    if (b == 1) {
        delay1 = 6;
        delay2 = 64;
    } else {
        delay1 = 80;
        delay2 = 10;
    }
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(pin, LOW);
    bcm2835_delayMicroseconds(delay1);
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
    bcm2835_delayMicroseconds(delay2);
}

void writeByte(uint8_t pin,int byte)
{
    int i;
    for (i = 0; i < 8; i++) {
        if (byte & 1) {
            writeBit(pin,1);
        } else {
            writeBit(pin,0);
        }
        byte = byte >> 1;
    }
}

uint8_t readBit(uint8_t pin)
{
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(pin, LOW);
    bcm2835_delayMicroseconds(6);
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
    bcm2835_delayMicroseconds(8);
    uint8_t b = bcm2835_gpio_lev(pin);
    bcm2835_delayMicroseconds(55);
    return b;
}

int readByte(uint8_t pin)
{
    int byte = 0;
    int i;
    for (i = 0; i < 8; i++) {
        byte = byte | readBit(pin) << i;
    };
    return byte;
}

int presence(uint8_t pin)
{
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(pin, LOW);
    bcm2835_delayMicroseconds(480);
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
    bcm2835_delayMicroseconds(70);
    uint8_t b = bcm2835_gpio_lev(pin);
    bcm2835_delayMicroseconds(410);
    return b;
}

uint8_t init_ds18b20(uint8_t sensor)
{
	if (!bcm2835_init())
	{
		printf("Error initializing GPIO bcm2835\n");
		return DS18B20_ERROR;
	}
	bcm2835_gpio_fsel(sensor, BCM2835_GPIO_FSEL_INPT);
	return 0;
}

float read_ds18b20(uint8_t sensor)
{
	if (presence(sensor) == 0) {
		printf("Temperature sensor detected\n");

		// Attempt to read the sensor until success or max attempts reached
		uint8_t success = 0;
		uint8_t data[9];
		for (int ij = 0; ij < MAX_READ; ij++) {
			// Set device active
			writeByte(sensor, 0xCC);

			// Initialize conversion
			writeByte(sensor, 0x44);

			// Poll for completion
			uint8_t success = 0;
			for (int i = 0; i < 1000; i++) {
				//bcm2835_delayMicroseconds(100000);
				bcm2835_delayMicroseconds(1000);
				if (readBit(sensor) == 1) {
					break;
					success = 1;
				}
			}
			if (success == 0) {
				printf("No response from sensor\n");
				break;
			}

			// Set device active
			presence(sensor);
			writeByte(sensor, 0xCC);

			// Read scratchpad
			writeByte(sensor, 0xBE);
			for (int i = 0; i < 9; i++) {
				data[i] = readByte(sensor);
			}

			// Check CRC is good
			uint8_t crc = crc8(data, 9);
			if (crc != 0) {
				printf("CRC error\n");
			} else {
				printf("Successful temperature read\n");
				success = 1;
			} 
		}
		if (success == 0) {
			return DS18B20_ERROR;
		}
		
		// Convert to degC
		int16_t temp_val = (data[1] << 8 | data[0]);
		printf("temp_val: %i\n", temp_val);
		float temp_degC = (float) temp_val / 16;
		printf("temp_degC: %f\n", temp_degC);
		return temp_degC;
	} else {
		printf("Temperature sensor not detected\n");
		return DS18B20_ERROR;
	}
}
