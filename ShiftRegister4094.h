/*
 *   4094 shift register driver
 *
 */

#ifndef _SHIFTREGISTER4094_H_
#define _SHIFTREGISTER4094_H_
 
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

void ShiftRegister4094_Init(void);

void ShiftRegister4094_DeInit(void);

void ShiftRegister4094_ShiftOut(void);

#ifdef __cplusplus
}
#endif

#endif /*_SHIFTREGISTER4094_H_ */