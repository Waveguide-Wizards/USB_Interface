/*
 * IFFF
 *  Waveguide Wizards
 *      Avery Anderson, Jack Danielski, Mike Fruge, Riley Hadjis, Mark Hinkle, Noa Margalit
 */

/*  I N C L U D E S   */
#include <stdint.h>
#include <stdbool.h>

#include "Memory/usb_host_msc.h"

/*  D R I V E R L I B   */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOS/Source/include/task.h"
#include "FreeRTOS/Source/include/queue.h"

/*  A P P L I C A T I O N    I N C L U D E S   */
#include "include/bsp.h"
#include "include/bumpers.h"
#include "include/calibration.h"
#include "include/error_checking.h"
#include "include/heater_control.h"
#include "include/led.h"
#include "include/motor_control.h"
//#include "include/UI_task.h"


/*  F R E E R T O S   H O O K S   */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vPreSleepProcessing( uint32_t ulExpectedIdleTime );
void vApplicationTickHook( void);
void *malloc( size_t xSize );


/*  G L O B A L S   */
volatile eState printer_state;
QueueHandle_t motor_instruction_queue;

/*  T A S K   H A N D L E S   */
TaskHandle_t thConfig = NULL;
TaskHandle_t thBlinkyTask = NULL;
TaskHandle_t thCalibration = NULL;
TaskHandle_t thErrorTask = NULL;
TaskHandle_t thExtruderTask = NULL;
TaskHandle_t thExtruderHeaterTask = NULL;
TaskHandle_t thBedHeaterTask = NULL;
TaskHandle_t thMotorTask = NULL;
TaskHandle_t thUITask = NULL;


/*   --- M A I N ---   */
void main(void)
{
	char *fileName[] = {"POLYGO~1.GCO"};
	//initialize USB
	usbInit();


	uartInit();
    usbConnect();


    printFileStructure();


    //READ FILE !!!
    read_file(2, fileName);
    // set clock source to 16MHz external oscillator, use PLL and divide by 10 to get 20MHz
//    SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
////    SysCtlClockFreqSet();
//
//    configASSERT(SysCtlClockGet() == 20000000);
//
//    /* I N I T */
////    init_all_motors();
//    init_bumper_gpio();
//
//    /* Q U E U E S */
////    motor_instruction_queue = xQueueCreate(10, sizeof(Motor_Instruction_t));
//
//    /* create first task */
////    BaseType_t configReturned = xTaskCreate(configTask, "Config", 400, (void *)NULL, 2, &thConfig);
////    configASSERT(configReturned == pdPASS);
//
//    /* create tasks */
//
//    // Priority 1
////    BaseType_t XMotorReturned = xTaskCreate(prv_Motor, "Motor Control", 500, (void *)NULL, 2, &thMotorTask);
////    vTaskSuspend(thMotorTask);
//
////    BaseType_t UIReturned = xTaskCreate(prv_UI, "UI", 400, (void *)NULL, 1, &thUITask);
////    vTaskSuspend(thUITask);
//
////    BaseType_t ExHeaterReturned = xTaskCreate(prvExtruderHeaterControl, "ExtruderHeater", 700, (void *)NULL, 2, &thExtruderHeaterTask);
////    vTaskSuspend(thExtruderHeaterTask);
////
////    BaseType_t BedHeaterReturned = xTaskCreate(prvBedHeaterControl, "BedHeater", 500, (void *)NULL, 2, &thBedHeaterTask);
////    vTaskSuspend(thBedHeaterTask);
//
////    BaseType_t ErrorCheckReturned = xTaskCreate(prv_ErrorCheck, "ErrorChecking", configMINIMAL_STACK_SIZE, (void *)NULL, 2, &thErrorTask);
//
//    // Priority 2
////    BaseType_t CalibrationReturned = xTaskCreate(prvCalibration, "Calibration", configMINIMAL_STACK_SIZE, (void *)NULL, 3, &thCalibration);
////    vTaskSuspend(thCalibration);
//
//    // Priority 3
//    // Priority 4
//    // Priority 5
////    BaseType_t BlinkyReturned = xTaskCreate(prvLED_Heartbeat, "HeartbeatLED", 300, (void *)NULL, 5, &thBlinkyTask);
//
//    /* check that tasks were created successfully */
// //   configASSERT(XMotorReturned == pdPASS);
////    configASSERT(UIReturned == pdPASS);
////    configASSERT(ExHeaterReturned == pdPASS);
////    configASSERT(BedHeaterReturned == pdPASS);
////    configASSERT(BlinkyReturned == pdPASS);
//
//    printer_state = Idle;
//
//#ifdef TEST_PREHEATING
//    printer_state = Preheating;
//    vTaskResume(thExtruderHeaterTask);
//    vTaskResume(thBedHeaterTask);
//#endif
//#ifdef TEST_CALIBRATION
//    printer_state = Calibration;
//    vTaskResume(thCalibration);
//    vTaskDelete(thConfig);
//#endif
//
//    /* start scheduler */
//    vTaskStartScheduler();
//    for( ;; );
//}
//
//
//void vApplicationMallocFailedHook( void )
//{
//    taskDISABLE_INTERRUPTS();
//    for( ;; );
//}
///*-----------------------------------------------------------*/
//
//void vApplicationIdleHook( void )
//{
//    for( ;; ) {
////        SysCtlSleep(); // sleep (stop the CPU clock) when the opportunity is given
//    }
//}
///*-----------------------------------------------------------*/
//
//void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
//{
//    ( void ) pcTaskName;
//    ( void ) pxTask;
//    taskDISABLE_INTERRUPTS();
//    for( ;; );
//}
//
//void vPreSleepProcessing( uint32_t ulExpectedIdleTime ) {
//
//}
//
//void vApplicationTickHook( void ) {
////    for( ;; );
//}
//
//void *malloc( size_t xSize ) {
//    for( ;; );
}
