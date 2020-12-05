#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <wiringPi.h>
#include <bcm2835.h>
#include <sched.h>
#include <sys/mman.h>
#include "ds18b20.h"

// Delay time
#define TI_DELAY_MS (1 * 1000)

// Relay output definitions
#define RELAY_1 1
#define ON LOW
#define OFF HIGH

// Temperature sensor input definitions
#define TEMPERATURE_1 RPI_V2_GPIO_P1_31

// Shutdown routine
void sigint(int a)
{
	digitalWrite(RELAY_1, OFF); // Shut off relay 1
	printf("\nGoodbye\n");
	exit(EXIT_SUCCESS);
}

int main(void)
{

	// Configure shutdown routine
	signal(SIGINT, sigint);

	// Configure relays
	wiringPiSetup() ;
	pinMode(RELAY_1, OUTPUT) ;

	// Configure temperature sensor
	const struct sched_param priority = {1};
	sched_setscheduler(0, SCHED_FIFO, &priority);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	if (init_ds18b20(TEMPERATURE_1) == DS18B20_ERROR) {
		printf("Error initializing Temperature Sensor 1\n");
	}

	float temperature_1_degC = 0;
	for (;;)
	{	
		// Read temperature sensor 1
		temperature_1_degC = read_ds18b20(TEMPERATURE_1);
		if (temperature_1_degC == DS18B20_ERROR) {
			printf("Error reading temperature sensor 1\n");
		} else {
			printf("Temperature 1: %0.2f degC\n", temperature_1_degC);
		}

		// Alternate relay 1 every 5 seconds 
		printf("Relay On\n");
		digitalWrite(RELAY_1, ON);
		delay(TI_DELAY_MS);
		printf("Relay Off\n");
		digitalWrite(RELAY_1, OFF);
		delay(TI_DELAY_MS);
	}
	return 0;
}
