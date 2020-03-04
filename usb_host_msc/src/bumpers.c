/*
 * bumpers.c
 *
 * Created on: Jan 9, 2020
 *      Author: Avery
 */

#include <stdbool.h>

/*  A P P L I C A T I O N    I N C L U D E S   */
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/bumpers.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/bsp.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/calibration.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/error_checking.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/motor_control.h"

/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*  D R I V E R L I B   */
#include "driverlib/gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

/*  G L O B A L S   */
extern eState printer_state;
extern TaskHandle_t thErrorTask;
extern TaskHandle_t thCalibration;
extern TaskHandle_t thUITask;

/*  F U N C T I O N S   */

void init_bumper_gpio(void){
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //Initialize the X, Y and Z bumper input pins consecutively
    GPIOPinTypeGPIOInput(X_BUMPER_PORT, X_BUMPER_PIN);
//    GPIOPinTypeGPIOInput(Y_BUMPER_PORT, Y_BUMPER_PIN);
//    GPIOPinTypeGPIOInput(Z_BUMPER_PORT, Z_BUMPER_PIN);

    //Set all pins to falling edge interrupts
    GPIOIntTypeSet(X_BUMPER_PORT, X_BUMPER_PIN,GPIO_FALLING_EDGE);
//    GPIOIntTypeSet(Y_BUMPER_PORT, Y_BUMPER_PIN,GPIO_RISING_EDGE);
//    GPIOIntTypeSet(Z_BUMPER_PORT, Z_BUMPER_PIN,GPIO_FALLING_EDGE);

    //Enable all of the interrupts
    GPIOIntEnable(X_BUMPER_PORT, X_BUMPER_PIN);//Pin Probably needs to be swapped out.
//    GPIOIntEnable(Y_BUMPER_PORT, Y_BUMPER_PIN);//Pin Probably needs to be swapped out.
//    GPIOIntEnable(Z_BUMPER_PORT, Z_BUMPER_PIN);//Pin Probably needs to be swapped out.

    IntEnable(INT_GPIOA);
    IntPrioritySet(INT_GPIOA, 0xF0);
}

void GPIO_A_IntHandler(void) {
    // Read the flags
    uint32_t flags = GPIOIntStatus(GPIO_PORTA_BASE,1);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(printer_state == Calibration) {
        configASSERT( thCalibration != NULL );

        if(flags && X_BUMPER_PIN){
            if(!is_motor_calibrated(Cal_X_ID)) {
                set_calibration(Cal_X_ID);
            }
//            else {
//                // TODO: soft error?
//            }
        }
        else if(flags && Y_BUMPER_PIN){
            if(!is_motor_calibrated(Cal_Y_ID)) {
                 set_calibration(Cal_Y_ID);
             }        }
        else if(flags && Z_BUMPER_PIN){
            if(!is_motor_calibrated(Cal_Z_ID)) {
                 set_calibration(Cal_Z_ID);
             }        }
        if(is_calibrated() == true) {
            vTaskNotifyGiveFromISR(thCalibration, &xHigherPriorityTaskWoken);
        }
        GPIOIntClear(GPIO_PORTA_BASE, flags);
    }

    else if(printer_state == Printing) {
        configASSERT( thUITask != NULL );

        /* Disable all motors */
        emergency_disable_motors();

        if(flags && X_BUMPER_PIN){
            add_error_to_list(X_Bumper);
        }
        else if(flags && Y_BUMPER_PIN){
            add_error_to_list(Y_Bumper);
        }
        else if(flags && Z_BUMPER_PIN){
            add_error_to_list(Z_Bumper);
        }

        xTaskNotifyFromISR(thUITask, 0x00001000, eSetBits, &xHigherPriorityTaskWoken);
        GPIOIntClear(GPIO_PORTA_BASE, flags);
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}
