/*
 * IFFF
 *  Waveguide Wizards
 *      Avery Anderson, Jack Danielski, Mike Fruge, Riley Hadjis, Mark Hinkle, Noa Margalit
 */

/*  I N C L U D E S   */
#include <stdint.h>
#include <stdbool.h>

/*  D R I V E R L I B   */
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_timer.h"

/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*  A P P L I C A T I O N    I N C L U D E S   */
#include "include/bsp.h"
#include "include/bumpers.h"
#include "include/calibration.h"
#include "include/error_checking.h"
#include "include/heater_control.h"
#include "include/led.h"
#include "include/motor_control.h"
//#include "include/motorUITest.h"
#include "UI/UI_task.h"
#include "Memory/memory_task.h"
#include "Memory/usb_host_msc.h"


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
TaskHandle_t thMemoryTask = NULL;

/*  Q U E U E S   */
QueueHandle_t qFileName;


/*   --- M A I N ---   */
void main(void)
{

		SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	    FPULazyStackingEnable();

	    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);


	    //
	    // Enable processor interrupts.
	    //
	  //  IntMasterEnable();

	    //
	    // Configure the two 32-bit periodic timers.
	    //
	    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	    TimerLoadSet(TIMER0_BASE, TIMER_B, ROM_SysCtlClockGet());


	    //
	    // Setup the interrupts for the timer timeouts.
	    //
	    IntEnable(INT_TIMER0B);

	    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);


	    //
	    // Enable the timers.
	    //
	    TimerEnable(TIMER0_BASE, TIMER_B);

//
//	uint32_t id[4] = {0,0,0,0};
//	  FLASHInit();
//	  FLASHReadId(id);
//
//		char *fileName[] = {"POLYGO~1.GCO"};
//		//initialize USB
//		usbInit();
//
//
//		uartInit();
//	  usbConnect();
//
//
//	  printFileStructure();
//
//
//	  //READ FILE !!!
//	  read_file(2, fileName);
//
//}
    // set clock source to 16MHz external oscillator, use PLL and divide by 10 to get 20MHz

  //  configASSERT(SysCtlClockGet() == 50000000);

    // Create Tasks
    BaseType_t UIReturned = xTaskCreate(prv_UI, "UI", 400, (void *)NULL, 1, &thUITask);
    BaseType_t MemoryReturned = xTaskCreate(prv_Memory, "Memory", 400, (void *)NULL, 2, &thMemoryTask);

    configASSERT(UIReturned == pdPASS);
    configASSERT(MemoryReturned == pdPASS);

    // Create Queue(s)
    qFileName = xQueueCreate(100, 17);

    // initialize printer_state
    printer_state = Idle;

    /* start scheduler */
    vTaskStartScheduler();
    for( ;; );
}


void vApplicationMallocFailedHook( void )
{
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    for( ;; ) {
//        SysCtlSleep(); // sleep (stop the CPU clock) when the opportunity is given
    }
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

void vPreSleepProcessing( uint32_t ulExpectedIdleTime ) {

}

void vApplicationTickHook( void ) {
//    for( ;; );
}

void *malloc( size_t xSize ) {
    for( ;; );
}


