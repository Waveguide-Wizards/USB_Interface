/*
 * UI.h
 *
 *  Created on: Feb 14, 2020
 *      Author: marka
 */

#ifndef UI_H_
#define UI_H_

// *** Includes
//
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
//
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/flash.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/rom.h"
//
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/touch.h"
//
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/checkbox.h"
#include "grlib/container.h"
#include "grlib/pushbutton.h"
#include "grlib/radiobutton.h"
#include "grlib/slider.h"
//
#include "UI/drivers/images.h"
#include "include/error_checking.h"

// *** Defines
//
#define UI_UTMENU_SEL_MEM_X                 60
#define UI_UTMENU_SEL_MEM_Y                 60
#define UI_UTMENU_SEL_MEM_W                 200
#define UI_UTMENU_SEL_MEM_H                 50
//
#define UI_UTMENU_SEL_MOTOR_X               60
#define UI_UTMENU_SEL_MOTOR_Y               120
#define UI_UTMENU_SEL_MOTOR_W               200
#define UI_UTMENU_SEL_MOTOR_H               50
//
#define UI_MS_TO_TICKS(x)                   (x * 40000 / 3)
#define UI_WARMUP_DELAY_TIME                40
//
#define UI_MAX_FILE_NUM                     3
#define UI_MAX_FILENAME_LEN                 17

// *** Function Definitions
//
void UI_Init(uint32_t ui32SysClock);
void UI_InitWidgetsIntegrationTest(void);
void UI_GenerateHeader(void);
//
void UI_OnWarmUpPaint(tContext *psContext);                             /*      */
void UI_OnIntroPaint(tWidget *psWidget, tContext *psContext);           /*      */
void UI_OnTestMenuPaint(tWidget *psWidget, tContext *psContext);        /*      */
void UI_OnMemTestPaint(tWidget *psWidget, tContext *psContext);         /*      */
void UI_OnFilesystemPaint(tWidget *psWidget, tContext *psContext);      /*      */
void UI_OnFileSelPaint(tWidget *psWidget, tContext *psContext);         /*      */
void UI_OnTransferPaint(tWidget *psWidget, tContext *psContext);        /*      */
void UI_OnMemCompletePaint(tWidget *psWidget, tContext *psContext);     /*      */
void UI_OnMotorTestPaint(tWidget *psWidget, tContext *psContext);       /*      */
void UI_OnMotorGoPaint(tWidget *psWidget, tContext *psContext);         /*      */
void UI_OnMotorColPaint(tWidget *psWidget, tContext *psContext);        /*      */
void UI_OnMotorCompletePaint(tWidget *psWidget, tContext *psContext);   /*      */
//
void UI_ReturnHome(tWidget * psWidget);
void UI_OnPrevious(tWidget *psWidget);
void UI_OnNext(tWidget *psWidget);
//
void UI_SelectMemTest(tWidget *psWidget);
void UI_SelectMotorTest(tWidget *psWidget);
//
void UI_MemSelectFile(tWidget *psWidget);
void UI_SliderMemConfrimFile(tWidget * psWidget, int32_t i32Value);
//
void UI_OnSliderChange(tWidget *psWidget, int32_t i32Value);
//
void UI_HandleErrors(uint32_t err);
void UI_UpdateFileNames(void);
void UI_MemTestComplete(uint32_t bytes_written);


#endif /* UI_H_ */
