/*
 * memory_task.c
 *
 *  Created on: Mar 2, 2020
 *      Author: margalitnoa
 */


#include "memory_task.h"
#include "Memory/usb_host_msc.h"
#include "include/bsp.h"

#include <string.h>

/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/*  G L O B A L   V A R I A B L E S   */
extern eState printer_state;
extern TaskHandle_t thMotorTask;
extern TaskHandle_t thExtruderTask;
extern TaskHandle_t thUITask;
extern TaskHandle_t thMemoryTask;

extern uint32_t file_index;

extern QueueHandle_t qFileName;


void prv_Memory(void *pvParameters)
{
    static TickType_t delay_time = pdMS_TO_TICKS( 100 );
    bool transfer_complete = false;


    for(ever) {
        /* When in the Select_Printer state and a notification is RX'd,
         *  the available files should be shown on the UI */
        switch(printer_state) {
		case(Storage_Device_Inserted): {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    // wait until notification is RX'd

            /* Get and display files */
    	    // these init's might need to be moved to main if it crashes the OS
    	    usbInit();
            usbConnect();
            int ret = printFileStructure();
//            char *fileName[] = {"POLYGO~1.GCO"};
//            int ret1 = read_file(2, fileName);
//            configASSERT(ret1 == 1); // TODO: if(!= 0): go to ErrorHandling Task
            xTaskNotifyGive(thUITask);
            break;
        }

		case(Select_Print): {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    // wait until notification is RX'd
            printer_state = USB_Transfer;
            break;
        }

        /* Where the fun stuff happens ;) */
		case(USB_Transfer): {
            if(transfer_complete == false) {
//                uint32_t file_index_2 = 0xFFF;

                // Wait for notification that contains the index of the
                // file to be read
//                xTaskNotifyWait(0xFFFF, 0, &file_index_2, portMAX_DELAY);
//                if(file_index == 0) {
//                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    // wait until notification is RX'd
//                if(file_index != 0xFFFF) {
//                    configASSERT(false);
//                }

                // Suspends all tasks, prevents context switching
//                vTaskSuspendAll();

                /* Transfer the file */
               // char * filename[MAX_FILENAME_STRING_LEN] = {""};
              //  memcpy(*filename, g_pcFilenames[file_index], sizeof(g_pcFilenames[file_index]));
                char *fileName[] = {"POLYGO~1.GCO"};
                int ret = read_file(2, fileName);
                configASSERT(ret == 1);
                transfer_complete = true;
                xTaskNotifyGive(thUITask);


                /* The operation is complete, set the scheduler back into the Active state. */
//                if( !xTaskResumeAll() ) {
//                   taskYIELD ();
//                }
//                }
            }
            if(transfer_complete == true) {
                xTaskNotifyGive(thUITask);
            }
            break;
        }

        /* Don't let this task run anymore once the file has been transferred */
		case(Printing): {
            vTaskSuspend(thMemoryTask);
            break;
        }
        }
        vTaskDelay(delay_time);
    }

}
