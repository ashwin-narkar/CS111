#include <stdio.h>
#include "mraa.h"
#include <mraa/gpio.h>


void button_handler(){
	printf("Button Pressed!\n");
}




int main() {
	mraa_gpio_context button = mraa_gpio_init(60);
	if (button == NULL) {
		fprintf(stderr,"Button not intialized!\n");
		exit(1);
	}
	mraa_aio_context tempSensor = mraa_aio_init(1);
	if (tempSensor == NULL) {
		fprintf(stderr,"Temp Sensor not intialized\n");
		exit(1);
	}
	printf("Sensor success\n");
	mraa_gpio_dir(button, MRAA_GPIO_IN);
	mraa_gpio_isr(button,MRAA_GPIO_EDGE_FALLING,button_handler,NULL);
	exit(0);
}
