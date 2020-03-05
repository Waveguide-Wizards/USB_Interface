/*
 * UI_task.c
 *
 *  Created on: Feb 24, 2020
 *      Author: marka
 */


// File specific header
#include "UI_task.h"

//Yeak idk other includes
#include "include/bsp.h"

/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*  G L O B A L   V A R I A B L E S   */
extern eState printer_state;
extern TaskHandle_t thMemoryTask;

extern bool begin_usb_connect;
extern bool begin_file_transfer;
extern uint32_t file_sel_index;
extern uint32_t file_index;

void prv_UI(void *pvParameters)
{
    static const TickType_t delay_time = pdMS_TO_TICKS( 100 );
    static uint32_t error_code = 0;
    static BaseType_t ret;

    //Initialize Variables
    UI_Init(SysCtlClockGet());

    // UI Main
    for(ever)
    {
        switch(printer_state)
        {

        case Idle:
            /* wait for user to signal to begin memory IT */
            if(begin_usb_connect == true)
            {
                begin_usb_connect = false;
                printer_state = Storage_Device_Inserted;
                xTaskNotifyGive(thMemoryTask);

                //XXX For Development
//                UI_UpdateFileNames();
//                printer_state = Select_Print;
            }
            break;

        case Storage_Device_Inserted:
            /* wait for notification that device is connected to USB device */
            ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            if(ret == pdPASS)
            {
                printer_state = Select_Print;
                UI_UpdateFileNames();
            }
            break;

        case Select_Print:
            /* wait for user to select file and notify uC to begin transfer */
            if(begin_file_transfer == true)
            {
                begin_file_transfer = false;
//                printer_state = USB_Transfer;
                file_index = 0;
                xTaskNotify(thMemoryTask, file_sel_index, eSetValueWithOverwrite);
//                xTaskNotifyGive(thMemoryTask);
            }
            break;

        case USB_Transfer:
            /* wait for memory transfer to complete */
//            ret = xTaskNotifyWait(UI_CLEAR_BITS_ON_ENTRY, UI_CLEAR_BITS_ON_EXIT, &error_code, UI_NOTIFY_WAIT_TIME);
            ret = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        	if(ret == pdPASS)
            {
                printer_state = Printing;
                vTaskSuspend(thMemoryTask);
                UI_MemTestComplete(error_code);
            }
            break;

        }

        //continue normal UI operation
        WidgetMessageQueueProcess();
        vTaskDelay(delay_time);
    }
}
