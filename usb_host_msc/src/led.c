/*
 * led.c
 *
 *  Created on: Sep 30, 2019
 *      Author: jack
 */

/*  D E V I C E   I N C L U D E S   */
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"


/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"


/*  A P P L I C A T I O N   I N C L U D E S   */
#include "include/led.h"


/*  T A S K S   */
void prvLED_Heartbeat(void *pvParameters) {
    static TickType_t delay_time = pdMS_TO_TICKS(500);
    init_led_gpio();
    for( ;; ) {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        vTaskDelay(delay_time);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
        vTaskDelay(delay_time);
    }
}


/*  F U N C T I O N S   */
void init_led_gpio(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // enable clock for Port F

    // set PF3 as output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x0);
}
