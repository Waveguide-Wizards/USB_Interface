/*
<<<<<<< HEAD
 * bumpers.h
 *
 *  Created on: Jan 24, 2020
 *      Author: jack
 */

#ifndef BUMPERS_H_
#define BUMPERS_H_

#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

#define X_BUMPER_PORT GPIO_PORTA_BASE
#define X_BUMPER_PIN  GPIO_INT_PIN_6

#define Y_BUMPER_PORT GPIO_PORTA_BASE
#define Y_BUMPER_PIN  GPIO_INT_PIN_3

#define Z_BUMPER_PORT GPIO_PORTA_BASE
#define Z_BUMPER_PIN  GPIO_INT_PIN_4

/*  F U N C T I O N S   */
void init_bumper_gpio(void);

#endif /* BUMPERS_H_ */

