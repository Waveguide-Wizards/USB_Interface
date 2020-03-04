/*
 * UI_task.h
 *
 *  Created on: Feb 24, 2020
 *      Author: marka
 */

#ifndef UI_UI_TASK_H_
#define UI_UI_TASK_H_

#include <stdio.h>
#include <stdint.h>
#include "UI.h"

#define ever                                ;;        //credit Jack Danielski - Such a funny guy can you believe that joke my god
//
#define UI_CLEAR_BITS_ON_ENTRY              0
#define UI_CLEAR_BITS_ON_EXIT               0
#define UI_NOTIFY_WAIT_TIME                 0

void prv_UI(void *pvParameters);

#endif /* UI_UI_TASK_H_ */
