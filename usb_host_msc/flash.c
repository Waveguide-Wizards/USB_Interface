
/*
 * flash.c
 *
 *  Created on: Dec 1, 2019
 *      Author: hadji
 */


#include "flash.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"


void FLASHInit(){
    //
    // The SSI0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
    //
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //
    // Configure the pin muxing for SSI
    // Do not use SSI FSS pin, instead use a GPIO
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PD0_SSI1CLK);
    GPIOPinConfigure(GPIO_PD2_SSI1RX);
    GPIOPinConfigure(GPIO_PD3_SSI1TX);

    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x2);
    //
    // TODO: change this to select the port/pin you are using.


    GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);


    SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);
    //
    // Enable the SSI module.
    // TODO: change to match SSI peripheral
    SSIEnable(SSI1_BASE);

}

void FLASHSendCommand(uint32_t * data, uint32_t size){
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x0);
    volatile uint32_t index = 0;
    for(index = 0; index < size; index++)
    {
        SSIDataPut(SSI1_BASE, data[index]);
        while(SSIBusy(SSI1_BASE))
        {
        }

    }

    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x2);
}

void FLASHSendCommandNoCS(uint32_t * data, uint32_t size){
    volatile uint32_t index = 0;
    for(index = 0; index < size; index++)
    {
        SSIDataPut(SSI1_BASE, data[index]);
        while(SSIBusy(SSI1_BASE))
        {
        }

    }
}

void FLASHClockOut(uint32_t size){
    volatile uint32_t index = 0;
    volatile uint32_t dummy = 0x00;
    for(index = 0; index < size; index++)
    {

        SSIDataPut(SSI1_BASE, 0x00);
        while(SSIBusy(SSI1_BASE))
        {
        }

    }
}

void FLASHReadResponse(uint32_t * data, uint32_t size){
    volatile uint32_t index = 0;
    for(index = 0; index < size; index++)
    {

        SSIDataGet(SSI1_BASE, &data[index]);
        while(SSIBusy(SSI1_BASE))
        {
        }
        data[index] &= 0x00FF;
    }
}

void FLASHClockIn(uint32_t size){
    volatile uint32_t index = 0;
    volatile uint32_t dummy[1];
    for(index = 0; index < size; index++)
    {

        SSIDataGet(SSI1_BASE, &dummy[1]);
        while(SSIBusy(SSI1_BASE))
        {
        }
    }
}


void FLASHWriteEnable(){
    uint32_t dataTx[1];
    uint32_t dataRx[1];
    dataTx[0] = 0x06;
    FLASHSendCommand(dataTx,1);
    FLASHReadResponse(dataRx,1);
}

void FLASHWriteAddress(uint32_t * address, uint32_t * data, uint32_t data_width){
    //data[data_width]
    uint32_t command[1];
    command[0] = 0x02;

    uint32_t i = 1;

    //Build command and address
    uint32_t send_pulse[8];
    send_pulse[0] = command[0];
    for(i = 1; i < 4; i++){
        send_pulse[i] = address[i-1];

    }

    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x0);
    uint32_t j = 0;
    while(j < data_width){
        if(i == 8){
            FLASHSendCommandNoCS(send_pulse,8);
            FLASHClockIn(8);
            i = 0;
        }
        send_pulse[i] = data[j];
        j++;
        i++;

    }
    FLASHSendCommandNoCS(send_pulse,i);
    FLASHClockIn(i);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x2);
}

int FLASHIsBusy(){
    uint32_t command[1];
    command[0] = 0x05;
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x0);
    FLASHSendCommandNoCS(command,1);
    FLASHClockOut(1);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x2);
    uint32_t data[2];
    FLASHReadResponse(data,2);
    volatile int ret = data[1] & 0x1;
    return ret;
}

void FLASHReadAddress(uint32_t * address, uint32_t * data, uint32_t data_width){
    //data[data_width + 4]
    uint32_t command[1];
    uint32_t data2[8];
    uint32_t data3[8];
    command[0] = 0x03;
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x0);
    FLASHSendCommandNoCS(command,1);
    FLASHSendCommandNoCS(address,3);

    uint32_t j = 0;
    uint32_t i = 0;
    uint32_t start = 4;
    while(j < data_width){
        if(i+start == 8){
            start = 0;
            FLASHClockOut(i);
            FLASHReadResponse(data2,8);
            for(i = 0; i < 8; i++) data[j+i] = data2[i];
            i = 0;
            j += 8;
        }
        i++;
    }
    uint32_t left_overs = j - data_width;
    FLASHClockOut(left_overs);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x2);
    FLASHReadResponse(data2,left_overs);
    for(i = 0; i < left_overs; i++) data[j+i] = data2[i];
}

void FLASHReadId(uint32_t * id){
    uint32_t dataTx[4] = {0x9f,0x00,0x00,0x00};
    FLASHSendCommand(dataTx,4);
    FLASHReadResponse(id,4);

}

void FLASHEraseSector(uint32_t * address){
    uint32_t command[1];
    command[0] = 0x20;

    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x0);
    FLASHSendCommandNoCS(command,1);
    FLASHSendCommandNoCS(address,3);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x2);
    FLASHClockIn(4);
    volatile int ret = 0;
    for(ret = 0; ret < 400000; ret++);
}
