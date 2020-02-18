/*
 * flash.h
 *
 *  Created on: Dec 1, 2019
 *      Author: hadji
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

extern void FLASHSendCommand(uint32_t * data, uint32_t size);
extern void FLASHReadResponse(uint32_t * data, uint32_t size);
extern void FLASHInit();
extern void FLASHWriteEnable();
extern void FLASHWriteAddress(uint32_t * address, uint32_t * data, uint32_t data_width);
extern void FLASHReadAddress(uint32_t * address, uint32_t * data, uint32_t data_width);
extern void FLASHReadId(uint32_t * id);
extern void FLASHEraseSector(uint32_t * address);
extern int  FLASHIsBusy();

#endif /* FLASH_H_ */
