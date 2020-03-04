/*
 * motor_control.c
 *
 *  Created on: Oct 6, 2019
 *      Author: jack
 */

/*  I N C L U D E S   */
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/*  A P P L I C A T I O N   I N C L U D E S   */
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/bsp.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/motor_control.h"

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
#include "inc/hw_gpio.h"


/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TEST
//#define BACKNFORTH


/*  G L O B A L   V A R I A B L E S   */
extern eState printer_state;
extern QueueHandle_t motor_instruction_queue;
extern TaskHandle_t thMotorTask;
extern TaskHandle_t thExtruderTask;


/*  P R I V A T E   V A R I A B L E S   */
static Motor_t x_motor;
static Motor_t y_motor;
static Motor_t z_motor;
static Motor_t ex_motor;

static uint32_t x_pwm_count = 0;
static uint32_t y_pwm_count = 0;
static uint32_t z_pwm_count = 0;
static uint32_t ex_pwm_count = 0;

static uint32_t x_needed_step_count = 0;
static uint32_t y_needed_step_count = 0;
static uint32_t z_needed_step_count = 0;
static uint32_t ex_needed_step_count = 0;

static uint8_t x_complete = 0;
static uint8_t y_complete = 0;
static uint8_t z_complete = 0;
static uint32_t task_complete = 0;

volatile Motor_Status_t Task_Status;

//static Motor_Count_Ready_t motors_ready;


/*  T A S K S   */
void prv_Motor(void *pvParameters) {
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 10000 );  // TODO: switch to max port delay
    uint32_t ulNotificationValue;
//    BaseType_t queue_receive_status;
    uint8_t instruction = 1;

//    // init motors
    init_all_motors();
    bool do_it = true;

    Motor_Instruction_t current_instruction, next_instruction;

    current_instruction.x_pos = 0;
    current_instruction.y_pos = 0;
    current_instruction.z_pos = 0;
    current_instruction.speed = 0;
    current_instruction.extruder_pos = 0;

    for( ;; ) {
        /* Wait for current instruction to be completed */
        ulNotificationValue = ulTaskNotifyTake( pdFALSE, xMaxBlockTime );

//        if( (ulNotificationValue == 1)  && (printer_state == Printing)) {
            // pop from queue
        printer_state = Printing;

        if(do_it == true) {

//            xQueueReceive(motor_instruction_queue,  &next_instruction, (TickType_t)5);

//            // set up motors
//            find_direction(current_instruction.x_pos, x_motor);
//            find_direction(current_instruction.y_pos, y_motor);
//            find_direction(current_instruction->z_pos, z_motor);
//            find_direction(current_instruction->x_pos, ex_motor);

            // update positions
//            x_motor.position = current_instruction.x_pos;
//            y_motor.position = current_instruction.y_pos;
//            z_motor.position = current_instruction->z_pos;
//            ex_motor.position = current_instruction->extruder_pos;

            // find step counts
//            x_needed_step_count = dist_to_steps(next_instruction.x_pos - current_instruction.x_pos);
//            y_needed_step_count = dist_to_steps(next_instruction.y_pos - current_instruction.y_pos);
//            z_pwm_count = dist_to_steps(current_instruction->z_pos);
//            ex_pwm_count = dist_to_steps(current_instruction->ex_pos);

            x_needed_step_count = dist_to_steps(20000);
            y_needed_step_count = dist_to_steps(20000);
            z_needed_step_count = dist_to_steps(20000);

//            current_instruction = next_instruction;
            // start PWM on all motors
            motor_change_pwm_duty_cycle(x_motor, 50);
//            motor_start(current_instruction.x_pos, 0, X_MOTOR, STEP_16);
            motor_start(20000, 0, X_MOTOR, STEP_16);

            motor_change_pwm_duty_cycle(y_motor, 50);
//            motor_start(current_instruction.y_pos, 0, Y_MOTOR, STEP_16);
            motor_start(20000, 0, Y_MOTOR, STEP_16);

            motor_change_pwm_duty_cycle(z_motor, 50);
//            motor_start(current_instruction.y_pos, 0, Y_MOTOR, STEP_16);
            motor_start(20000, 0, Z_MOTOR, STEP_16);

            do_it = false;
        }
        else if(do_it == false) {
            motor_disable(x_motor);
            motor_disable(y_motor);
            motor_change_pwm_duty_cycle(x_motor, 0);
            motor_change_pwm_duty_cycle(y_motor, 0);
            do_it = true;
        }
        else {  // taking notification timed out, indicate error occurred
            printer_state = Error;
        }
        vTaskDelay(xMaxBlockTime);


//        xTaskNotifyWait
    }
}

void prv_Extruder_Motor(void *pvParameters) {
    // configure motor to run
//    x_motor.direction = Forward;
    //    ex_pwm_count = dist_to_steps((uint32_t)pvParameters);    // extrude 2cm
    init_all_motors();
//    x_motor.direction = Forward;
    x_motor.direction = Backward;
     motor_set_direction(x_motor, x_motor.direction);
     motor_set_step_size(x_motor, STEP_16);
     motor_enable(x_motor);
     motor_change_pwm_duty_cycle(x_motor, 50);

    // wait for notification from interrupt
//    uint32_t ulNotificationValue;
//    ulNotificationValue = ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
//    if(ulNotificationValue == 1) {
//        vTaskDelete(xExtruderTask);
//    }
//    vTaskDelay(pdMS_TO_TICKS( 1500 ));
      vTaskDelay(pdMS_TO_TICKS( 4760 ));
//    vTaskDelay(pdMS_TO_TICKS( 20000 ));
//     motor_disable(x_motor);
//     x_motor.direction = Forward;
//     motor_set_direction(x_motor, x_motor.direction);
//     set_motor_step_size(x_motor, STEP_16);
//     motor_enable(x_motor);
//     motor_change_pwm_duty_cycle(x_motor, 50);
//     vTaskDelay(pdMS_TO_TICKS( 100 ));
//     vTaskDelay(pdMS_TO_TICKS( 1000 ));
     motor_disable(x_motor);

    vTaskSuspend(thExtruderTask);
}


/*  F U N C T I O N S   */


//This is used to convert the desired distance into a step count.
uint32_t dist_to_steps(uint32_t distance) {
    return (uint32_t)((distance * USTEP_PER_DIST) + 0.5);
}



/*  I N I T I A L I Z A T I O N    F U N C T I O N S   */

/* M O T O R S */

void init_all_motors(void) {
    init_x_motor();
    init_y_motor();

    // Commented for POC testing
#ifndef TEST
    init_z_motor();
    init_ex_motor();
#endif
}

void init_x_motor(void) {
    motor_init_x_gpio();
    motor_init_x_pwm();
    motor_disable(x_motor);
}

void init_y_motor(void) {
    motor_init_y_gpio();
    motor_init_y_pwm();
    motor_disable(y_motor);
}

#ifndef TEST

void init_z_motor(void) {
    motor_init_z_gpio();
    motor_init_z_pwm();
    motor_disable(z_motor);
}



void init_ex_motor(void) {
    motor_init_ex_gpio();
    motor_init_ex_pwm();
    motor_disable(ex_motor);
}
#endif



void init_motor_status(uint8_t x_init_status,uint8_t y_init_status,uint8_t z_init_status){
    Task_Status.x_done = x_init_status;
    Task_Status.y_done = y_init_status;
    Task_Status.z_done = z_init_status;
}

void start_motor_calibration(eMotor_ID motor) {
    switch(motor) {
        case(X_Motor_ID): {
            x_motor.direction = Forward;
            motor_set_direction(x_motor, x_motor.direction);
            motor_set_step_size(x_motor, STEP_16);
            motor_enable(x_motor);
            motor_change_pwm_duty_cycle(x_motor, 50);
            break;
        }
        case(Y_Motor_ID): {
            y_motor.direction = Forward;
            motor_set_direction(y_motor, y_motor.direction);
            motor_set_step_size(y_motor, STEP_16);
            motor_enable(y_motor);
            motor_change_pwm_duty_cycle(y_motor, 50);
            break;
        }
        case(Z_Motor_ID): {
            z_motor.direction = Forward;
            motor_set_direction(z_motor, z_motor.direction);
            motor_set_step_size(z_motor, STEP_16);
            motor_enable(z_motor);
            motor_change_pwm_duty_cycle(z_motor, 50);
            break;
        }
    }
}


/*  M O T O R   P W M   */
void motor_init_x_pwm(void) {
    x_motor.PWM_Base = X_MOTOR_PWM_BASE;
    x_motor.PWM_Channel = X_MOTOR_PWM_CHANNEL;
    x_motor.PWM_Block = X_PWM_BLOCK;
 //   x_motor.PWM_Pin_Map = X_MOTOR_PWM_OUT;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);         // The PWM peripheral must be enabled for use.

    /* init GPIO pin */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);            // enable GPIO port if not already enabled

    /* setup and enable clock */
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);                 // Set the PWM clock to the system clock.

//    HWREG(GPIO_PORTB_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
//    HWREG(GPIO_PORTE_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
//
//    // Port B pins that are locked are 3 and 2, so unlock them by writing 1100 into the CR reg
//    HWREG(GPIO_PORTB_BASE + GPIO_O_CR)  |= 0xC;


    GPIOPinConfigure(x_motor.PWM_Pin_Map);                // configure pin for PWM
    GPIOPinTypePWM(x_motor.STEP.base, x_motor.STEP.pin);

    /* Count down without synchronization */
    PWMGenConfigure(x_motor.PWM_Base, X_PWM_BLOCK, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    /* Set PWM period to: 0.02ms or 50kHz */
    PWMGenPeriodSet(x_motor.PWM_Base, x_motor.PWM_Block, CALC_PERIOD(PWM_FREQUENCY));

    /* initialize to no output */
    PWMPulseWidthSet(x_motor.PWM_Base, X_PWM_OUT, 0);

    /* Enable Interrupts */
    PWMGenIntRegister(x_motor.PWM_Base, x_motor.PWM_Block, PWM0Gen0IntHandler);

    IntPrioritySet(INT_PWM0_0, 0xF0);

    IntEnable(INT_PWM0_0);

    /* Enable PWM Signal output */
    PWMOutputState(x_motor.PWM_Base, (1 << x_motor.PWM_Channel), false);

//    /* Enable the generator block to start timer */
//    PWMGenEnable(x_motor.PWM_Base, x_motor.PWM_Block);

}


void motor_init_y_pwm(void) {

    y_motor.PWM_Base = Y_MOTOR_PWM_BASE;
    y_motor.PWM_Channel = Y_MOTOR_PWM_CHANNEL;
    y_motor.PWM_Block = Y_PWM_BLOCK;
    y_motor.PWM_Pin_Map = Y_MOTOR_PWM_OUT;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);         // The PWM peripheral must be enabled for use.

    // Port B pins that are locked are 3 and 2, so unlock them by writing 1100 into the CR reg
    // HWREG(GPIO_PORTB_BASE + GPIO_O_CR)  |= 0xC;

    /* setup and enable clock */
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);                 // Set the PWM clock to the system clock.

    GPIOPinConfigure(y_motor.PWM_Pin_Map);                // configure pin for PWM rather than for GPIO

    GPIOPinTypePWM(y_motor.STEP.base, y_motor.STEP.pin);

    /* Count down without synchronization */
    PWMGenConfigure(y_motor.PWM_Base, Y_PWM_BLOCK, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    /* Set PWM period to: 0.02ms or 50kHz */
    PWMGenPeriodSet(y_motor.PWM_Base, y_motor.PWM_Block, CALC_PERIOD(PWM_FREQUENCY));

    /* initialize to no output */
    PWMPulseWidthSet(y_motor.PWM_Base, Y_PWM_OUT, 0);

    /* Register Interrupts  to be enabled*/
    PWMGenIntRegister(y_motor.PWM_Base, y_motor.PWM_Block, PWM0Gen1IntHandler);

    // Need to set the priority of the interrupt to be lower than (larger number) configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
    /* Priority only depends on the 3 highest bits */

    IntPrioritySet(INT_PWM0_1, 0xF0);

    int prio;
    prio = IntPriorityGet(INT_PWM0_1);

    IntEnable(INT_PWM0_1);

    /* Ensure PWM signal output is off */
    PWMOutputState(y_motor.PWM_Base, (1 << y_motor.PWM_Channel), false);

}


void motor_init_z_pwm(void) {
    z_motor.PWM_Base = Z_MOTOR_PWM_BASE;
    z_motor.PWM_Channel = Z_MOTOR_PWM_CHANNEL;
    z_motor.PWM_Pin_Map = Z_MOTOR_STEP;

    /* setup and enable clock */
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);                 // Set the PWM clock to the system clock.

    // TODO: need to see if there's a way to make this more generic
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);         // The PWM peripheral must be enabled for use.

    /* init GPIO pin */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);            // enable GPIO port if not already enabled
    GPIOPinConfigure(z_motor.PWM_Pin_Map);                // configure pin for PWM
    GPIOPinTypePWM(z_motor.STEP.base, z_motor.STEP.pin);

    /* Count down without synchronization */
    PWMGenConfigure(z_motor.PWM_Base, Z_PWM_BLOCK, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    /* Set PWM period to: 0.02ms or 50kHz */
    PWMGenPeriodSet(z_motor.PWM_Base, Z_PWM_BLOCK, CALC_PERIOD(PWM_FREQUENCY));

    /* initialize to no output */
    PWMPulseWidthSet(z_motor.PWM_Base, Z_PWM_OUT, 0);

    /* Enable Interrupts */
    PWMGenIntRegister(z_motor.PWM_Base, Z_PWM_BLOCK, PWM0Gen3IntHandler);
}


void motor_init_ex_pwm(void) {

    ex_motor.PWM_Base = EX_MOTOR_PWM_BASE;
    ex_motor.PWM_Channel = EX_MOTOR_PWM_CHANNEL;
    ex_motor.PWM_Pin_Map = EX_MOTOR_STEP;

    /* setup and enable clock */
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);                 // Set the PWM clock to the system clock.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);         // The PWM peripheral must be enabled for use.

    /* init GPIO pin */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);            // enable GPIO port if not already enabled
    GPIOPinConfigure(ex_motor.PWM_Pin_Map);                // configure pin for PWM
    GPIOPinTypePWM(ex_motor.STEP.base, ex_motor.STEP.pin);

    /* Count down without synchronization */
    PWMGenConfigure(ex_motor.PWM_Base, EX_PWM_BLOCK, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    /* Set PWM period to: 0.02ms or 50kHz */
    PWMGenPeriodSet(ex_motor.PWM_Base, EX_PWM_BLOCK, CALC_PERIOD(PWM_FREQUENCY));

    /* initialize to no output */
    PWMPulseWidthSet(ex_motor.PWM_Base, EX_PWM_OUT, 0);

    /* Enable Interrupts */
//    PWMGenIntRegister(ex_motor.PWM_Base, EX_PWM_BLOCK, PWM0Gen3IntHandler);
}


/* @param uint8_t duty_cycle: 0-100 */
void motor_change_pwm_duty_cycle(Motor_t motor, uint8_t duty_cycle) {
    PWMPulseWidthSet(motor.PWM_Base, motor.PWM_Block, ((duty_cycle * CALC_PERIOD(PWM_FREQUENCY))/100));
}


/*   M O T O R   G P I O   */

/* Configure GPIO pins for DRV8886
 *
 *  DIR - direction input,
 *  ENABLE - input, HIGH is enabled, LOW is disabled
 *  M0 and M1 - inputs, microstepping mode, M1=0 and M0=1 for 1/16 step
 *  nFAULT - Fault indication output from DRV8886
 *  nSLEEP - sleep mode input, LOW to enter low-power
 *  DECAY - ?
 *  TRQ - ?
 */

void motor_init_x_gpio(void)
{
    // Assign Struct members to definitions
    x_motor.M0.base   =    X_M0_PORT;
    x_motor.M0.pin    =    X_M0_PIN;

    x_motor.M1.base   =    X_M1_PORT;
    x_motor.M1.pin    =    X_M1_PIN;

    x_motor.DIR.base  =    X_DIR_PORT;
    x_motor.DIR.pin   =    X_DIR_PIN;

    x_motor.ENABLE.base =  X_ENABLE_PORT;
    x_motor.ENABLE.pin  =  X_ENABLE_PIN;

    x_motor.NSLEEP.base =  X_NSLEEP_PORT;
    x_motor.NSLEEP.pin  =  X_NSLEEP_PIN;

    x_motor.NFAULT.base =  X_NFAULT_PORT;
    x_motor.NFAULT.pin  =  X_NFAULT_PIN;

    x_motor.STEP.base   =  X_STEP_PORT;
    x_motor.STEP.pin    =  X_STEP_PIN;

    // Enable Ports

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    /* Set GPIO output pins */
    MAP_GPIODirModeSet(x_motor.M0.base, x_motor.M0.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(x_motor.M1.base, x_motor.M1.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(x_motor.DIR.base, x_motor.DIR.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(x_motor.ENABLE.base, x_motor.ENABLE.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(x_motor.STEP.base, x_motor.STEP.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(x_motor.NSLEEP.base, x_motor.NSLEEP.pin, GPIO_DIR_MODE_OUT);

    /* Set GPIO Input pins */
    MAP_GPIODirModeSet(x_motor.NFAULT.base, x_motor.NFAULT.pin, GPIO_DIR_MODE_IN);

    /* Set Drive Strength */
    MAP_GPIOPadConfigSet(x_motor.M0.base, x_motor.M0.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD); // used to set drive strength
    MAP_GPIOPadConfigSet(x_motor.M1.base, x_motor.M1.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(x_motor.DIR.base, x_motor.DIR.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(x_motor.ENABLE.base, x_motor.ENABLE.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(x_motor.STEP.base, x_motor.STEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(x_motor.NSLEEP.base, x_motor.NSLEEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(x_motor.NFAULT.base, x_motor.NFAULT.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);

    /* Set enable high after init*/
    GPIOPinWrite(x_motor.ENABLE.base, x_motor.ENABLE.pin, x_motor.ENABLE.pin);
}

void motor_init_y_gpio(void)
{
    // Assign Struct members to definitions
    y_motor.M0.base   =    Y_M0_PORT;
    y_motor.M0.pin    =    Y_M0_PIN;

    y_motor.M1.base   =    Y_M1_PORT;
    y_motor.M1.pin    =    Y_M1_PIN;

    y_motor.DIR.base  =    Y_DIR_PORT;
    y_motor.DIR.pin   =    Y_DIR_PIN;

    y_motor.ENABLE.base =  Y_ENABLE_PORT;
    y_motor.ENABLE.pin  =  Y_ENABLE_PIN;

    y_motor.NSLEEP.base =  Y_NSLEEP_PORT;
    y_motor.NSLEEP.pin  =  Y_NSLEEP_PIN;

    y_motor.NFAULT.base =  Y_NFAULT_PORT;
    y_motor.NFAULT.pin  =  Y_NFAULT_PIN;

    y_motor.STEP.base   =  Y_STEP_PORT;
    y_motor.STEP.pin    =  Y_STEP_PIN;

    // Enable Ports

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    /* Set GPIO output pins */
    MAP_GPIODirModeSet(y_motor.M0.base, y_motor.M0.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(y_motor.M1.base, y_motor.M1.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(y_motor.DIR.base, y_motor.DIR.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(y_motor.ENABLE.base, y_motor.ENABLE.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(y_motor.STEP.base, y_motor.STEP.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(y_motor.NSLEEP.base, y_motor.NSLEEP.pin, GPIO_DIR_MODE_OUT);

    /* Set GPIO Input pins */
    MAP_GPIODirModeSet(y_motor.NFAULT.base, y_motor.NFAULT.pin, GPIO_DIR_MODE_IN);

    /* Set Drive Strength */
    MAP_GPIOPadConfigSet(y_motor.M0.base, y_motor.M0.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD); // used to set drive strength
    MAP_GPIOPadConfigSet(y_motor.M1.base, y_motor.M1.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(y_motor.DIR.base, y_motor.DIR.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(y_motor.ENABLE.base, y_motor.ENABLE.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(y_motor.STEP.base, y_motor.STEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(y_motor.NSLEEP.base, y_motor.NSLEEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(y_motor.NFAULT.base, y_motor.NFAULT.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);

    GPIOPinWrite(y_motor.ENABLE.base, y_motor.ENABLE.pin, y_motor.ENABLE.pin);
}


/* Commented for Test */

void motor_init_z_gpio(void)
{
    // Assign Struct members to definitions
    z_motor.M0.base   =    Z_M0_PORT;
    z_motor.M0.pin    =    Z_M0_PIN;

    z_motor.M1.base   =    Z_M1_PORT;
    z_motor.M1.pin    =    Z_M1_PIN;

    z_motor.DIR.base  =    Z_DIR_PORT;
    z_motor.DIR.pin   =    Z_DIR_PIN;

    z_motor.ENABLE.base =  Z_ENABLE_PORT;
    z_motor.ENABLE.pin  =  Z_ENABLE_PIN;

    z_motor.NSLEEP.base =  Z_NSLEEP_PORT;
    z_motor.NSLEEP.pin  =  Z_NSLEEP_PIN;

    z_motor.NFAULT.base =  Z_NFAULT_PORT;
    z_motor.NFAULT.pin  =  Z_NFAULT_PIN;

    z_motor.STEP.base   =  Z_STEP_PORT;
    z_motor.STEP.pin    =  Z_STEP_PIN;

    // Enable Ports

    MAP_SysCtlPeripheralEnable(z_motor.M0.base);      // Port E
    MAP_SysCtlPeripheralEnable(z_motor.M1.base);      // Port J
    MAP_SysCtlPeripheralEnable(z_motor.NFAULT.base);  // Port H

    /* Set GPIO output pins */
    MAP_GPIODirModeSet(z_motor.M0.base, z_motor.M0.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(z_motor.M1.base, z_motor.M1.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(z_motor.DIR.base, z_motor.DIR.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(z_motor.ENABLE.base, z_motor.ENABLE.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(z_motor.STEP.base, z_motor.STEP.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(z_motor.NSLEEP.base, z_motor.NSLEEP.pin, GPIO_DIR_MODE_OUT);

    /* Set GPIO Input pins */
    MAP_GPIODirModeSet(z_motor.NFAULT.base, z_motor.NFAULT.pin, GPIO_DIR_MODE_IN);

    /* Set Drive Strength */
    MAP_GPIOPadConfigSet(z_motor.M0.base, z_motor.M0.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD); // used to set drive strength
    MAP_GPIOPadConfigSet(z_motor.M1.base, z_motor.M1.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(z_motor.DIR.base, z_motor.DIR.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(z_motor.ENABLE.base, z_motor.ENABLE.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(z_motor.STEP.base, z_motor.STEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(z_motor.NSLEEP.base, z_motor.NSLEEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(z_motor.NFAULT.base, z_motor.NFAULT.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);

    GPIOPinWrite(z_motor.ENABLE.base, z_motor.ENABLE.pin, z_motor.ENABLE.pin);
}


void motor_init_ex_gpio(void) {
    // Assign Struct members to definitions
    ex_motor.M0.base   =    EX_M0_PORT;
    ex_motor.M0.pin    =    EX_M0_PIN;

    ex_motor.M1.base   =    EX_M1_PORT;
    ex_motor.M1.pin    =    EX_M1_PIN;

    ex_motor.DIR.base  =    EX_DIR_PORT;
    ex_motor.DIR.pin   =    EX_DIR_PIN;

    ex_motor.ENABLE.base =  EX_ENABLE_PORT;
    ex_motor.ENABLE.pin  =  EX_ENABLE_PIN;

    ex_motor.NSLEEP.base =  EX_NSLEEP_PORT;
    ex_motor.NSLEEP.pin  =  EX_NSLEEP_PIN;

    ex_motor.NFAULT.base =  EX_NFAULT_PORT;
    ex_motor.NFAULT.pin  =  EX_NFAULT_PIN;

    ex_motor.STEP.base   =  EX_STEP_PORT;
    ex_motor.STEP.pin    =  EX_STEP_PIN;

    // Enable Ports

    MAP_SysCtlPeripheralEnable(ex_motor.M0.base);      // Port E
    MAP_SysCtlPeripheralEnable(ex_motor.M1.base);      // Port J
    MAP_SysCtlPeripheralEnable(ex_motor.NFAULT.base);  // Port H

    /* Set GPIO output pins */
    MAP_GPIODirModeSet(ex_motor.M0.base, ex_motor.M0.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(ex_motor.M1.base, ex_motor.M1.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(ex_motor.DIR.base, ex_motor.DIR.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(ex_motor.ENABLE.base, ex_motor.ENABLE.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(ex_motor.STEP.base, ex_motor.STEP.pin, GPIO_DIR_MODE_OUT);
    MAP_GPIODirModeSet(ex_motor.NSLEEP.base, ex_motor.NSLEEP.pin, GPIO_DIR_MODE_OUT);

    /* Set GPIO Input pins */
    MAP_GPIODirModeSet(ex_motor.NFAULT.base, ex_motor.NFAULT.pin, GPIO_DIR_MODE_IN);

    /* Set Drive Strength */
    MAP_GPIOPadConfigSet(ex_motor.M0.base, ex_motor.M0.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD); // used to set drive strength
    MAP_GPIOPadConfigSet(ex_motor.M1.base, ex_motor.M1.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(ex_motor.DIR.base, ex_motor.DIR.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(ex_motor.ENABLE.base, ex_motor.ENABLE.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(ex_motor.STEP.base, ex_motor.STEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(ex_motor.NSLEEP.base, ex_motor.NSLEEP.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPadConfigSet(ex_motor.NFAULT.base, ex_motor.NFAULT.pin, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);

    GPIOPinWrite(ex_motor.ENABLE.base, ex_motor.ENABLE.pin, ex_motor.ENABLE.pin);
}


/* M O T O R    C O N F I G U R A T I O N */


void motor_enable(Motor_t motor) {
    GPIOPinWrite(motor.NSLEEP.base, motor.NSLEEP.pin, motor.NSLEEP.pin);    // set ENABLE pin HIGH
    GPIOPinWrite(motor.ENABLE.base, motor.ENABLE.pin, motor.ENABLE.pin);    // set ENABLE pin HIGH
    GPIOPinWrite(motor.NSLEEP.base, motor.NSLEEP.pin, motor.NSLEEP.pin);
    PWMOutputState(motor.PWM_Base, (1 << motor.PWM_Channel), true);    // disables PWM output
    PWMGenEnable(motor.PWM_Base, motor.PWM_Block);                     // enables PWM
}

void motor_disable(Motor_t motor) {
    GPIOPinWrite(motor.NSLEEP.base, motor.NSLEEP.pin, 0);               // set ENABLE pin LOW
    PWMOutputState(motor.PWM_Base, (1 << motor.PWM_Channel), false);    // disables PWM output
    PWMGenDisable(motor.PWM_Base, motor.PWM_Block);                                // disable PWM
}

/*
 *Enables PWM interrupts, Enables motors
 */
void motor_start(uint32_t distance, uint32_t direction, uint8_t motor, uint8_t step_size)
{
    switch(motor)
    {
    case X_MOTOR:
        /* Enable PWM Module 0 or 1, and the signal Generation block 0 1 2 or 3 */
        PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
        PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
        motor_set_step_size(x_motor, step_size);
        motor_enable(x_motor);
        break;
    case Y_MOTOR:
        PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);
        PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_1, PWM_INT_CNT_ZERO);
        motor_set_step_size(y_motor, step_size);
        motor_enable(y_motor);
        break;
    case Z_MOTOR:
        GPIOPinWrite(z_motor.NSLEEP.base, z_motor.NSLEEP.pin, z_motor.NSLEEP.pin);
        PWMIntEnable(PWM0_BASE, PWM_INT_GEN_2);
        PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_2, PWM_INT_CNT_ZERO);
        motor_set_step_size(y_motor, step_size);
        motor_enable(y_motor);
    case EX_MOTOR:
        GPIOPinWrite(ex_motor.NSLEEP.base, ex_motor.NSLEEP.pin, ex_motor.NSLEEP.pin);
        PWMIntEnable(PWM0_BASE, PWM_INT_GEN_3);
        PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_3, PWM_INT_CNT_ZERO);
        motor_set_step_size(ex_motor, step_size);
        motor_enable(ex_motor);
    default:

        /* Motor has not been implemented yet */
        break;

    }
}

void motor_set_to_sleep(Motor_t motor) {
    GPIOPinWrite(motor.NSLEEP.base, motor.NSLEEP.pin, 0);   // set nSLEEP pin LOW
}

void motor_set_direction(Motor_t motor, eMotor_Direction direction) {
    if(direction == Forward) {
        GPIOPinWrite(motor.DIR.base, motor.DIR.pin, motor.DIR.pin);
    }
    else if(direction == Backward) {
        GPIOPinWrite(motor.DIR.base, motor.DIR.pin, 0);
    }
}


void find_direction(uint32_t instruction, Motor_t motor) {
    if(instruction >= motor.position) {
        motor.direction = Backward;
    }
    else {
        motor.direction = Forward;
    }
}

void motor_set_step_size(Motor_t motor, uint8_t size){

    switch(size)
    {
    case STEP_FULL:
        GPIOPinWrite(motor.M0.base, motor.M0.pin, 0);
        GPIOPinWrite(motor.M1.base, motor.M1.pin, 0);
        break;
    case STEP_16:
        GPIOPinWrite(motor.M0.base, motor.M0.pin, motor.M0.pin);
        GPIOPinWrite(motor.M1.base, motor.M1.pin, 0);
        break;
    case STEP_2:
        GPIOPinWrite(motor.M0.base, motor.M0.pin, 0);
        GPIOPinWrite(motor.M1.base, motor.M1.pin, motor.M1.pin);
        break;
    case STEP_4:
        GPIOPinWrite(motor.M0.base, motor.M0.pin, motor.M0.pin);
        GPIOPinWrite(motor.M1.base, motor.M1.pin, motor.M1.pin);
        break;
    case STEP_8:
        /* In reality, these should be high Z */
        GPIOPinWrite(motor.M0.base, motor.M0.pin, motor.M0.pin);
        GPIOPinWrite(motor.M1.base, motor.M1.pin, motor.M1.pin);
        break;
    }
}




/* C O N V E R S I O N      F U N C T I O N  S */

//This is used to convert the numer of steps taken into a distance in micrometers.
uint32_t motor_steps_to_dist(uint32_t stepCount) {
    return stepCount*DIST_PER_USTEP;
}

//This is used to convert the desired distance into a step count.
uint32_t motor_dist_to_steps(uint32_t distance) {
    return (uint32_t)((distance * USTEP_PER_DIST) + 0.5);
}

uint8_t update_motor_status(uint8_t motor)
{
    switch(motor)
    {
    case X_MOTOR:
        Task_Status.x_done = 1;
        break;
    case Y_MOTOR:
        Task_Status.y_done = 1;
        break;
    case Z_MOTOR:
        Task_Status.z_done = 1;
        break;
    default:
        // Motor has not been implemented yet
        break;
    }

    if(Task_Status.x_done && Task_Status.y_done && Task_Status.z_done){
        init_motor_status(0,0,0);
        return 1;
    }

    return 0;
}





/*  E R R O R   H A N D L I N G   */

void emergency_disable_motors(void) {
    motor_disable(x_motor);
    motor_change_pwm_duty_cycle(x_motor, 0);
    motor_disable(y_motor);
    motor_change_pwm_duty_cycle(y_motor, 0);
    motor_disable(z_motor);
    motor_change_pwm_duty_cycle(z_motor, 0);
    motor_disable(ex_motor);
    motor_change_pwm_duty_cycle(ex_motor, 0);
}

void error_bumper_retract(eMotor_ID motor) {
    switch(motor) {
        case(X_Motor_ID): {
            motor_set_direction(x_motor, Backward);
            motor_change_pwm_duty_cycle(x_motor, 50);
            motor_enable(x_motor);
            uint32_t delay;
            for(delay = 0; delay < 10000; delay++); // should change this
            motor_disable(x_motor);
            motor_change_pwm_duty_cycle(x_motor, 0);
            break;
        }
        case(Y_Motor_ID): {
            motor_set_direction(y_motor, Backward);
            motor_change_pwm_duty_cycle(y_motor, 50);
            motor_enable(y_motor);
            uint32_t delay;
            for(delay = 0; delay < 10000; delay++); // should change this
            motor_disable(y_motor);
            motor_change_pwm_duty_cycle(y_motor, 0);
            break;
        }
        case(Z_Motor_ID): {
            motor_set_direction(z_motor, Backward);
            motor_change_pwm_duty_cycle(z_motor, 50);
            motor_enable(z_motor);
            uint32_t delay;
            for(delay = 0; delay < 10000; delay++); // should change this
            motor_disable(z_motor);
            motor_change_pwm_duty_cycle(z_motor, 0);
            break;
        }
    }
}


/* I N T E R U P T      H A N D L E R S */
/*
 * TODO:
 *  - use LOAD register to count Pulses
 *  - set PWM duty cycle for X/Y/Z to 0 in this handler
 *  - notify motors task once an interrupt flag for each motor has been raised
 */

/* X - Motor */
void PWM0Gen0IntHandler(void) {

    PWMIntDisable(PWM0_BASE, PWM_INT_GEN_0);
    int flags = PWMGenIntStatus(PWM0_BASE, PWM_GEN_0, true);       // Get status of interrupts
    PWMGenIntClear(PWM0_BASE, PWM_GEN_0, flags);     // Clear interrupts

    if(flags & PWM_INT_CNT_ZERO)
    {
        x_pwm_count++;
    }

    if(x_pwm_count == x_needed_step_count)
    {
        x_complete++;
        x_pwm_count = 0;
        /* TODO Set a flag and do this motor disable */
        motor_disable(x_motor);
        if(update_motor_status(X_MOTOR))
        {
            task_complete++;
            // Task complete
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            configASSERT(prv_Motor != NULL);
            vTaskNotifyGiveFromISR(prv_Motor, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
}


/* Y - Motor */

void PWM0Gen1IntHandler(void) {
    PWMIntDisable(PWM0_BASE, PWM_INT_GEN_1);
    int flags = PWMGenIntStatus(PWM0_BASE, PWM_GEN_1, true);       // Get status of interrupts
    PWMGenIntClear(PWM0_BASE, PWM_GEN_1, flags);     // Clear interrupts

    if(flags & PWM_INT_CNT_ZERO)
    {
        y_pwm_count++;
    }

    if(y_pwm_count == y_needed_step_count)
    {
        y_pwm_count = 0;
        y_complete++;
        /* TODO: Set a flag or alert a task to do this */
//        motor_disable(y_motor);
        if(update_motor_status(Y_MOTOR))
        {
            task_complete++;

            // TODO: if step count met for all motors execute this
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            configASSERT(prv_Motor != NULL);
            vTaskNotifyGiveFromISR(prv_Motor, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);
}


/* Z- Motor*/
void PWM0Gen3IntHandler(void) {

    PWMIntDisable(PWM0_BASE, PWM_INT_GEN_3);
    int flags = PWMGenIntStatus(PWM0_BASE, PWM_GEN_3, true);       // Get status of interrupts
    PWMGenIntClear(PWM0_BASE, PWM_GEN_3, flags);     // Clear interrupts

    if(flags & PWM_INT_CNT_ZERO)
    {
        z_pwm_count++;
    }

    if(z_pwm_count == z_needed_step_count)
    {
        z_complete++;
        z_pwm_count = 0;
        /* TODO Set a flag and do this motor disable */
//        motor_disable(x_motor);
        if(update_motor_status(Z_MOTOR))
        {
            task_complete++;
            // Task complete
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            configASSERT(prv_Motor != NULL);
            vTaskNotifyGiveFromISR(prv_Motor, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_3);
}

//void PWM0Gen2IntHandler(void) {
//
//    PWMIntDisable(PWM0_BASE, PWM_INT_GEN2);
//    int flags = PWMGenIntStatus(PWM0_BASE, PWM_GEN_2, true);       // Get status of interrupts
//    PWMGenIntClear(PWM0_BASE, PWM_GEN_2, flags);     // Clear interrupts
//
//    if(flags & PWM_INT_CNT_ZERO)
//    {
//        ex_pwm_count++;
//    }
//
//    if(ex_pwm_count == ex_needed_step_count)
//    {
//        ex_complete++;
//        ex_pwm_count = 0;
//        /* TODO Set a flag and do this motor disable */
//        motor_disable(ex_motor);
//        if(update_motor_status(EX_MOTOR))
//        {
//            task_complete++;
//            // Task complete
//            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//            configASSERT(xMotorTask != NULL);
//            vTaskNotifyGiveFromISR(xMotorTask, &xHigherPriorityTaskWoken);
//            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//        }
//    }
//
//    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_2);
//}
