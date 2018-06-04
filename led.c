/*
 * led.c
 *
 *  Created on: Dec 11, 2017
 *      Author: markin
 *
 *      LED_BLUE_IO 	(GPIO_NUM_18)
 *      LED_RED_IO 		(GPIO_NUM_19)
 */

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "led.h"

//led




void initialise_led()
{
    gpio_pad_select_gpio(2);
    gpio_set_direction(2,GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(2,GPIO_PULLUP_ONLY);
    gpio_pad_select_gpio(LED_BLUE_IO);
    gpio_set_direction(LED_BLUE_IO,GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LED_BLUE_IO,GPIO_PULLUP_ONLY);
    gpio_pad_select_gpio(LED_RED_IO);
    gpio_set_direction(LED_RED_IO,GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LED_RED_IO,GPIO_PULLUP_ONLY);
}
