/*
 * print_tasks.c
 *
 *  Created on: Feb 8, 2020
 *      Author: jack
 *
 * Contains tasks related to accomplishing a 3D Print:
 *  - Calibration
 *  - Actuation based on Gcode instruction queue
 */

/*  I N C L U D E S   */
#include <stdint.h>
#include <stdbool.h>

/*  A P P L I C A T I O N   I N C L U D E S   */
#include "include/calibration.h"
#include "include/bsp.h"
#include "include/error_checking.h"
#include "include/motor_control.h"

/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*  G L O B A L S   */
extern eState printer_state;

/*  P R I V A T E   V A R I A B L E S   */
static bool calibrated_motors[3] = {false, false, false};

/*  T A S K S   */
void prvCalibration(void *pvParameters) {
    for( ;; ) {
        // 1. start X motor in forward direction
        start_motor_calibration(X_Motor_ID);

        // 2. wait for notification and check if X Motor was "calibrated"
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(!is_motor_calibrated(Cal_X_ID)) {
            // trigger error
        }

        // 3. start Y motor in forward direction
        start_motor_calibration(Y_Motor_ID);

        // 4. wait for notification and check if Y Motor was "calibrated"
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(!is_motor_calibrated(Cal_Y_ID)) {
            // trigger error
        }

        // 5. start Z motor in forward direction
        start_motor_calibration(Z_Motor_ID);

        // 6. wait for notification and check if Z Motor was "calibrated"
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(!is_motor_calibrated(Cal_Z_ID)) {
            // trigger error
        }

        // 7. set state
        printer_state = Ready_To_Print;
    }
}

/*  F U N C T I O N S   */

void reset_calibration_checks(void) {
    uint8_t i;
    for(i = 0; i < 3; i++) {
        calibrated_motors[i] = false;
    }
}

void set_calibration(eCalibration_Motor_ID motor_id) {
    calibrated_motors[motor_id] = true;
}

bool is_motor_calibrated(eCalibration_Motor_ID motor_id) {
    return calibrated_motors[motor_id];
}

bool is_calibrated(void) {
    uint8_t i;
    for(i = 0; i < 3; i++) {
        if(calibrated_motors[i] == false) {
            return false;
        }
    }
    return true;
}
