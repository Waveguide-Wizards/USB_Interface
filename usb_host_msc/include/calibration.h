/*
 * calibration.h
 *
 *  Created on: Feb 8, 2020
 *      Author: jack
 */

#ifndef CALIBRATION_H_
#define CALIBRATION_H_

typedef enum {
    Cal_X_ID = 0,
    Cal_Y_ID = 1,
    Cal_Z_ID = 2
}eCalibration_Motor_ID;


void prvCalibration(void *pvParameters);
void reset_calibration_checks(void);
void set_calibration(eCalibration_Motor_ID motor_id);
bool is_motor_calibrated(eCalibration_Motor_ID motor_id);
bool is_calibrated(void);

#endif /* CALIBRATION_H_ */
