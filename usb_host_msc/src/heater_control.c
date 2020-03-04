/*
 * heater_control.c
 *
 *  Created on: Feb 7, 2020
 *      Author: jack
 */

/*  I N C L U D E S   */
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/*  A P P L I C A T I O N   I N C L U D E S   */
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/bsp.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/heater_control.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/pid.h"
#include "/Users/margalitnoa/git/USB_Interface_3/usb_host_msc/include/error_checking.h"

/*  D R I V E R   L I B   */
//#include "tm4c123gh6pm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/pwm.h"
#include "driverlib/adc.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_adc.h"


/*  F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*  G L O B A L S   */
extern eState printer_state;
extern TaskHandle_t thExtruderHeaterTask;
extern TaskHandle_t thBedHeaterTask;
extern TaskHandle_t thMotorTask;

/*  P R I V A T E   V A R I A B L E S   */
static PID_t extruder_heater_pid;
static PID_t bed_heater_pid;
static float extruder_target_voltage = 1.250;
static float bed_target_voltage = 1.250;
static bool ex_heater_ready = 0;
static bool bed_heater_ready = 0;


/*  T A S K S */

void prvExtruderHeaterControl(void * prvParameters) {
    uint8_t delay_time_ms = 10;  // 10ms intervals
    TickType_t delay_time = pdMS_TO_TICKS( delay_time_ms );
    PID_init(&extruder_heater_pid, 3.2, 2.1, 2.3, extruder_target_voltage, delay_time_ms);   // initialize PID controller

    // local variables
    static uint32_t ex_adc_value = 0;
    static float ex_temperature_v = 0.0;

    // init hardware
    init_extruder_heater_adc();
    init_extruder_heater_pwm();

    for( ;; ) {
        // 1. Get latest ADC reading
        ex_heater_get_adc(&ex_adc_value);

        // 2. Convert to voltage
        ex_temperature_v = adc_convert_to_v(ex_adc_value);

        // 3. Check for over voltage
//        if(ex_temperature_v > MAX_EX_TEMPERATURE_V) {
//            ex_heater_disable();
//            add_error_to_list(Ex_Heater_Overheat);
//            vTaskSuspend(thExtruderHeaterTask);
//        }
//        else {
            // 4. Update UI with latest temperature reading
            // convert_v_to_celsius();

            // 5. Pre-heat check
//            if(ex_heater_ready == false) {
//                if((ex_temperature_v >= (0.97 * extruder_target_voltage)) && (eTaskGetState(thMotorTask) == eSuspended)) {
//                    ex_heater_ready = true;
//                }
//            }
//            else {
//                if(ex_heater_ready && bed_heater_ready) {
//                    printer_state = Ready_To_Print;
//                    vTaskResume(thMotorTask);
//                }
//            }

            // 6. Calculate PID and Change PWM duty cycle based on PID calculation
//            ex_heater_change_pwm_duty_cycle(PID_calculate(&extruder_heater_pid, ex_temperature_v));
//            vTaskDelay(delay_time);
//        }
        vTaskDelay(delay_time);
    }
}

void prvBedHeaterControl(void * prvParameters) {
    uint8_t delay_time_ms = 10;  // 10ms intervals
    TickType_t delay_time = pdMS_TO_TICKS( delay_time_ms );
    PID_init(&bed_heater_pid, 3.2, 2.1, 2.3, 1.250, delay_time_ms);        // initialize PID controller

    // local variables
    static uint32_t bed_adc_value = 0;
    static float bed_temperature_v = 0.0;

    // init hardware
    init_bed_heater_adc();
//    init_bed_heater_pwm();

    for( ;; ) {
        // 1. Get latest ADC reading
        ex_bed_get_adc(&bed_adc_value);

        // 2. Convert to voltage
        bed_temperature_v = adc_convert_to_v(bed_adc_value);

        // 3. Check for over voltage
//        if(bed_temperature_v > MAX_BED_TEMPERATURE_V) {
//            bed_heater_disable();
//            add_error_to_list(Bed_Heater_Overheat);
//            vTaskSuspend(thBedHeaterTask);
//        }
//        else {
//            // 4. Update UI with latest temperature reading
//            // convert_v_to_celsius();
//
//            // 5. Pre-heat check
//            if(bed_heater_ready == false) {
//                if((bed_temperature_v >= (0.97 * bed_target_voltage)) && (eTaskGetState(thMotorTask) == eSuspended)) {
//                    bed_heater_ready = true;
//                }
//            }
//            else {  // allow for printing to start
//                if(ex_heater_ready && bed_heater_ready) {
//                    printer_state = Ready_To_Print;
//                    vTaskResume(thMotorTask);
//                }
//            }
//
//            // 6. Calculate PID and Change PWM duty cycle based on PID calculation
//            bed_heater_change_pwm_duty_cycle(PID_calculate(&bed_heater_pid, bed_temperature_v));
//            vTaskDelay(delay_time);
//        }
        vTaskDelay(delay_time);
    }
}

/*  A D C   */
void init_extruder_heater_adc(void) {
    /* Configure ADC peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(EX_HEATER_ADC_PORT, EX_HEATER_ADC_PIN);
    ADCSequenceConfigure(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER, 0, EX_HEATER_ADC_CHANNEL | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER);
    ADCIntClear(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER);
}

void init_bed_heater_adc(void) {
    /* Configure ADC peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(BED_HEATER_ADC_PORT, BED_HEATER_ADC_PIN);
    ADCSequenceConfigure(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER, 0, BED_HEATER_ADC_CHANNEL | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER);
    ADCIntClear(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER);
}

void ex_heater_get_adc(uint32_t * put_data) {
    ADCProcessorTrigger(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER);
    while(!ADCIntStatus(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER, false));
    ADCIntClear(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER);
    ADCSequenceDataGet(EX_HEATER_ADC_BASE, EX_HEATER_ADC_SEQUENCER, put_data);
}

void ex_bed_get_adc(uint32_t * put_data) {
    ADCProcessorTrigger(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER);
    while(!ADCIntStatus(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER, false));
    ADCIntClear(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER);
    ADCSequenceDataGet(BED_HEATER_ADC_BASE, BED_HEATER_ADC_SEQUENCER, put_data);
}

/*  P W M */
void init_extruder_heater_pwm(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);         // The PWM peripheral must be enabled for use.

    /* init GPIO pin */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);            // enable GPIO port if not already enabled

    /* setup and enable clock */
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);                 // Set the PWM clock to the system clock.


    GPIOPinConfigure(EX_HEATER_PWM_PIN_MAP);                // configure pin for PWM
    GPIOPinTypePWM(EX_HEATER_PWM_PORT, EX_HEATER_PWM_PIN);

    /* Count down without synchronization */
    PWMGenConfigure(EX_HEATER_PWM_BASE, EX_HEATER_PWM_BLOCK, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    PWMGenPeriodSet(EX_HEATER_PWM_BASE, EX_HEATER_PWM_BLOCK, CALC_PERIOD(PWM_FREQUENCY));

    /* initialize to no output */
    PWMPulseWidthSet(EX_HEATER_PWM_BASE, EX_HEATER_PWM_OUT, 0);

//    /* Enable Interrupts */
//    PWMGenIntRegister(x_motor.PWM_Base, x_motor.PWM_Block, PWM0IntHandler);

    /* Enable the generator block to start timer */
    PWMGenEnable(EX_HEATER_PWM_BASE, EX_HEATER_PWM_BLOCK);

    PWMOutputState(EX_HEATER_PWM_BASE, (1 << EX_HEATER_PWM_CHANNEL), true);
}

void init_bed_heater_pwm(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);         // The PWM peripheral must be enabled for use.

    /* init GPIO pin */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);        // enable GPIO port if not already enabled

    /* setup and enable clock */
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);                 // Set the PWM clock to the system clock.

//    HWREG(GPIO_PORTB_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
//    HWREG(GPIO_PORTE_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
//
//    // Port B pins that are locked are 3 and 2, so unlock them by writing 1100 into the CR reg
//    HWREG(GPIO_PORTB_BASE + GPIO_O_CR)  |= 0xC;

    GPIOPinConfigure(BED_HEATER_PWM_PIN_MAP);                // configure pin for PWM
    GPIOPinTypePWM(BED_HEATER_PWM_PORT, BED_HEATER_PWM_PIN);

    /* Count down without synchronization */
    PWMGenConfigure(BED_HEATER_PWM_BASE, BED_HEATER_PWM_BLOCK, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    PWMGenPeriodSet(BED_HEATER_PWM_BASE, BED_HEATER_PWM_BLOCK, CALC_PERIOD(PWM_FREQUENCY));

    /* initialize to no output */
    PWMPulseWidthSet(BED_HEATER_PWM_BASE, BED_HEATER_PWM_OUT, 0);

//    /* Enable Interrupts */
//    PWMGenIntRegister(x_motor.PWM_Base, x_motor.PWM_Block, PWM0IntHandler);

    /* Enable the generator block to start timer */
    PWMGenEnable(BED_HEATER_PWM_BASE, BED_HEATER_PWM_BLOCK);

    PWMOutputState(BED_HEATER_PWM_BASE, (1 << BED_HEATER_PWM_CHANNEL), true);
}

/* @param uint8_t duty_cycle: 0-100 */
void ex_heater_change_pwm_duty_cycle(uint8_t duty_cycle) {
    PWMPulseWidthSet(EX_HEATER_PWM_BASE, EX_HEATER_PWM_BLOCK, ((duty_cycle * CALC_PERIOD(PWM_FREQUENCY))/100));
}

/* @param uint8_t duty_cycle: 0-100 */
void bed_heater_change_pwm_duty_cycle(uint8_t duty_cycle) {
    PWMPulseWidthSet(EX_HEATER_PWM_BASE, EX_HEATER_PWM_BLOCK, ((duty_cycle * CALC_PERIOD(PWM_FREQUENCY))/100));
}

void ex_heater_enable(void) {
    PWMOutputState(EX_HEATER_PWM_BASE, (1 << EX_HEATER_PWM_CHANNEL), true); // disables PWM output
    PWMGenEnable(EX_HEATER_PWM_BASE, PWM_GEN_0);                            // enables PWM
}

void ex_heater_disable(void) {
    PWMOutputState(EX_HEATER_PWM_BASE, (1 << EX_HEATER_PWM_CHANNEL), false);    // disables PWM output
    PWMGenDisable(EX_HEATER_PWM_BASE, PWM_GEN_0);                               // disable PWM
}

void bed_heater_enable(void) {
    PWMOutputState(BED_HEATER_PWM_BASE, (1 << BED_HEATER_PWM_CHANNEL), true);   // disables PWM output
    PWMGenEnable(BED_HEATER_PWM_BASE, PWM_GEN_0);                               // enables PWM
}

void bed_heater_disable(void) {
    PWMOutputState(BED_HEATER_PWM_BASE, (1 << BED_HEATER_PWM_CHANNEL), false);  // disables PWM output
    PWMGenDisable(BED_HEATER_PWM_BASE, PWM_GEN_0);                              // disable PWM
}

void emergency_heaters_disable(void) {
    ex_heater_disable();
    bed_heater_disable();
}

/* T E M P E R A T U R E */

// TODO: characterize temperature readings, possibly use LUT
void change_extruder_temperature(uint32_t temperature) {
    extruder_target_voltage = temperature / 5.0;
}

// TODO: characterize temperature readings, possibly use LUT
void change_bed_temperature(uint32_t temperature) {
    bed_target_voltage = temperature / 5.0;
}

/*   C O N V E R S I O N S   */

/**
 * @brief Converts the ADC result to millivolts
 *
 * @details Multiplies the ADC results by the number of mV
 *      in VREF and then divides by the maximum value of the ADC.
 *
 * @return Value in mV
 */
uint32_t adc_convert_to_mv(uint32_t adc_result) {
    return ((VREF_MV * adc_result) / ADC_MAX_VALUE);
}

/**
 * @brief Converts the ADC result to volts
 *
 * @details Multiplies the ADC results by the value of VREF
 *      and then divides by the maximum value of the ADC.
 *
 * @return Value in Volts
 */
float adc_convert_to_v(uint32_t adc_result) {
    return (((float)adc_result * VREF_MV_F) / ADC_MAX_VALUE_F);
}
