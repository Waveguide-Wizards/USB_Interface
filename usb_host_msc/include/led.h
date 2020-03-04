/*
 * led.h
 *
 *  Created on: Oct 5, 2019
 *      Author: jack
 */

#ifndef INCLUDE_LED_H_
#define INCLUDE_LED_H_

/*  T A S K S   */
void prvLED_Heartbeat(void *pvParameters);

/*  F U N C T I O N S   */
void init_led_gpio(void);

#endif /* INCLUDE_LED_H_ */
