/*
 * error_checking.c
 *
 *  Created on: Nov 21, 2019
 *      Author: jack
 */


/*  I N C L U D E S   */
#include <stdbool.h>
#include <stdint.h>

/*  A P P L I C A T I O N   I N C L U D E S   */
#include "include/bsp.h"
#include "include/error_checking.h"
#include "include/heater_control.h"
#include "include/motor_control.h"

/*  D R I V E R   L I B   */
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*  G L O B A L   V A R I A B L E S   */
extern TaskHandle_t thBlinkyTask;
extern TaskHandle_t thCalibration;
extern TaskHandle_t thErrorTask;
extern TaskHandle_t thExtruderTask;
extern TaskHandle_t thExtruderHeaterTask;
extern TaskHandle_t thBedHeaterTask;
extern TaskHandle_t thMotorTask;

/*  G L O B A L S   */
extern eState printer_state;

/*  P R I V A T E   V A R I A B L E S   */
static bool error_list[NUM_ERROR_SOURCES];
static uint8_t error_count = 0; // for UI to utilize
static eError_Source newest_error = None;

/*  T A S K S   */
void prv_ErrorCheck(void *pvParameters) {
    error_list_init();
    for( ;; ) {
        // wait for task notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // suspend tasks that might be faulty
        vTaskSuspend(thMotorTask);
        vTaskSuspend(thExtruderHeaterTask);
        vTaskSuspend(thBedHeaterTask);
        vTaskSuspend(thExtruderTask);

        // disable possibly faulty peripherals
        emergency_disable_motors();
        emergency_heaters_disable();

        /* react to the latest error */
        switch(newest_error) {
            /* BUMPERS: slight movement towards center */
            case(X_Bumper): error_bumper_retract(X_Motor_ID); break;
            case(Y_Bumper): error_bumper_retract(Y_Motor_ID); break;
            case(Z_Bumper): error_bumper_retract(Z_Motor_ID); break;
        }

        /* determine number of errors */
        update_error_count();

        /* TODO: remove error from list if capable */
    }
}

/*  F U N C T I O N S   */

void error_list_init(void) {
    uint8_t i;
    for(i = 0; i < NUM_ERROR_SOURCES; i++) {
        error_list[i] = false;
    }
}

void add_error_to_list(eError_Source error) {
    error_list[error] = true;
    newest_error = error;
    error_count++;
}

void delete_error_from_list(eError_Source error) {
    error_list[error] = false;
    error_count--;
}

uint8_t get_error_count(void) {
    return error_count;
}

void update_error_count(void) {
    uint8_t index, count = 0;
    for(index = 0; index < NUM_ERROR_SOURCES; index++) {
        if(error_list[index] == true) {
            count++;
        }
    }
    error_count = count;
}


