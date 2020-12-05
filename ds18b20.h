/*
Reference: https://www.iot-programmer.com/index.php/books/22-raspberry-pi-and-the-iot-in-c/chapters-raspberry-pi-and-the-iot-in-c/36-raspberry-pi-and-the-iot-in-c-one-wire-basics?start=1 
*/
#include <bcm2835.h>

// Define an error code for temperature measurement
#define DS18B20_ERROR 255

uint8_t crc8(uint8_t *data, uint8_t len);
void writeBit(uint8_t pin,int b);
void writeByte(uint8_t pin,int byte);
uint8_t readBit(uint8_t pin);
int readByte(uint8_t pin);
int presence(uint8_t pin);
uint8_t init_ds18b20(uint8_t sensor);
float read_ds18b20(uint8_t sensor);
