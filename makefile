make: main.c ds18b20.c
	gcc -o main main.c ds18b20.c -l wiringPi -l bcm2835
