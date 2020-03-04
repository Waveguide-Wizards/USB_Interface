/*
 * heater_control.h
 *
 *  Created on: Feb 7, 2020
 *      Author: jack
 */

#ifndef HEATER_CONTROL_H_
#define HEATER_CONTROL_H_

#define ADC_MAX_VALUE           4095    // 12-bit
#define ADC_MAX_VALUE_F         4095.0  // 12-bit floating point
#define VREF_MV                 3300U   // 3.3V in millivolts
#define VREF_MV_F               3.3     // 3.3V

#define MAX_EX_TEMPERATURE_V    3.0
#define MAX_BED_TEMPERATURE_V    3.0


#define SOURCE_FREQUENCY        20000000    // 20MHz
#define PWM_FREQUENCY           400         // 5kHz, DRV8886 f_pwm range is 0-100kHz
#define CALC_PERIOD(X)          (SOURCE_FREQUENCY / X)

#include <stdint.h>

/*  T A S K S */
void prvExtruderHeaterControl(void * prvParameters);
void prvBedHeaterControl(void * prvParameters);

/*  A D C   */
void init_extruder_heater_adc(void);
void init_bed_heater_adc(void);
void ex_heater_get_adc(uint32_t * put_data);
void ex_bed_get_adc(uint32_t * put_data);

/* P W M */
void init_extruder_heater_pwm(void);
void init_bed_heater_pwm(void);
void ex_heater_change_pwm_duty_cycle(uint8_t duty_cycle);
void bed_heater_change_pwm_duty_cycle(uint8_t duty_cycle);
void ex_heater_enable(void);
void ex_heater_disable(void);
void bed_heater_enable(void);
void bed_heater_disable(void);
void emergency_heaters_disable(void);

/* T E M P E R A T U R E */
void change_extruder_temperature(uint32_t temperature);
void change_bed_temperature(uint32_t temperature);

/* C O N V E R S I O N S   */
uint32_t adc_convert_to_mv(uint32_t adc_result);
float adc_convert_to_v(uint32_t adc_result);

#endif /* HEATER_CONTROL_H_ */
