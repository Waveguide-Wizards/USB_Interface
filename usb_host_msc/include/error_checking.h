/*
 * error_checking.h
 *
 *  Created on: Nov 21, 2019
 *      Author: jack
 */

#ifndef ERROR_CHECKING_H_
#define ERROR_CHECKING_H_

#define NUM_ERROR_SOURCES                   12
#define IT_NUM_SUBMODULES                   1
//
#define ERR_CODE_ID_BASE                    (0x00000000UL)
#define ERR_MOTOR_CODE_BASE_ID              (ERR_CODE_ID_BASE + (0x0 << 16))


#include <stdbool.h>

typedef enum {
    None = -1,
    X_nFAULT = 0,
    Y_nFAULT,
    Z_nFAULT,
    Ex_nFAULT,
    X_Bumper,
    Y_Bumper,
    Z_Bumper,
    PWR_eFuse,  // redundant if there is only 24V eFuse
    Ex_Heater_Overheat,
    Bed_Heater_Overheat,
    USB,
    SD_Card,
}eError_Source;

void prv_ErrorCheck(void *pvParameters);
void error_list_init(void);
void add_error_to_list(eError_Source error);
void delete_error_from_list(eError_Source error);
uint8_t get_error_count(void);
void update_error_count(void);

#endif /* ERROR_CHECKING_H_ */
