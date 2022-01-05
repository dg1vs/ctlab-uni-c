/*
 * Uni_C_hw.h
 *
 * Created: 31.03.2014 21:37:44
 *  Author: ks
 */ 


#ifndef _UNI_C_HW_H_
#define _UNI_C_HW_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// IO Module Port Option 16 Bit I2C Piggy-Back 
#define PORTIO_PCA9555D_ADDRESS 0x21  // dec 33

#define PORTIO_PCA9555D_REGISTER_INPUT_PORT_0 0x00
#define PORTIO_PCA9555D_REGISTER_INPUT_PORT_1 0x01
#define PORTIO_PCA9555D_REGISTER_OUTPUT_PORT_0 0x02
#define PORTIO_PCA9555D_REGISTER_OUTPUT_PORT_1 0x03
#define PORTIO_PCA9555D_REGISTER_POL_INVERSION_0 0x04
#define PORTIO_PCA9555D_REGISTER_POL_INVERSION_1 0x05
#define PORTIO_PCA9555D_REGISTER_CONFIG_0 0x06
#define PORTIO_PCA9555D_REGISTER_CONFIG_1 0x07


/* From manual
0 Input port 0
1 Input port 1
2 Output port 0
3 Output port 1
4 Polarity Inversion port 0
5 Polarity Inversion port 1
6 Configuration port 0
7 Configuration port 1
*/

float Adc_Read(uint8_t ch);
uint16_t Adc_RawRead(uint8_t channel);
void Adc_Init(void);
void Adc_DeInit(void);


void Dac_Init(void);
void Dac_DeInit(void);
// 4 DAC at all
void Dac_Output(uint8_t dacChannel, float val);


void RelayPort_Init(void);
void RelayPort_DeInit(void);
void RelayPort_Set(uint8_t port, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* _UNI_C_HW_H_ */