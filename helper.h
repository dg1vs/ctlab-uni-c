/*
 * help.h
 *
 * Created: 09.09.2017 20:54:16
 *  Author: ks
 */ 


#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void HexString2Value(uint8_t* uiPtr, uint32_t *ulpValue);
void DecString2Value(uint8_t* uiPtr, uint32_t *ulpValue);
void LIMIT_float(float *param, float min, float max);
void LIMIT_UINT8(uint8_t *param, uint8_t min, uint8_t max);
void LIMIT_INT16(int16_t *param, int16_t min, int16_t max);

#ifdef __cplusplus
}
#endif

#endif /* _HELPER_H_ */