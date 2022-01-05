/*
 * IncFile1.h
 *
 * Created: 16.11.2017 21:24:21
 *  Author: ks
 */ 


#ifndef _UNIC_PARSER_H
#define _UNIC_PARSER_H


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PARAM_FLOAT    0
#define PARAM_INT       1
#define PARAM_BYTE      2
#define PARAM_STR       3
#define PARAM_BYTE_DIRECT 5
#define PARAM_INT_DIRECT 6
#define PARAM_DOUBLE_DIRECT 7
#define PARAM_UINT16       8
#define PARAM_UINT16_DIRECT       9
#define PARAM_UINT32_DIRECT       10

#define SCALE_NONE      0
#define SCALE_PROZ      4
#define SCALE_TEMP      5
typedef struct PARAM_DATA
{
	union
	{
		float* f;
		int32_t* l;
		int16_t* i;
		uint16_t* u;
		uint8_t* b;
		const char* s;
		void* v;
	} ram;
	uint8_t type        : 4; // Type of function/variable (int, double etc.)
	uint8_t scale       : 4;
} PARAM_DATA;


void Handle_EEStuff(const uint8_t mode, uint8_t ucScriptMode __attribute__((unused)));
uint8_t HandleSetParamWithEE(float Value, PARAM_DATA D);




typedef struct _PARAMTABLE
{
	uint8_t SubCh; // Command ordinal
	union
	{
		double (*get_f_Function)(void);
		int32_t (*get_l_Function)(void);
		int16_t (*get_i_Function)(void);
		uint16_t (*get_u_Function)(void);
		uint8_t (*get_b_Function)(void);
		char* (*get_s_Function)(void);
		void (*doFunction)(struct _PARAMTABLE*);
		struct
		{
			union
			{
				double* f;
				int32_t* l;
				int16_t* i;
				uint16_t* u;
				uint8_t* b;
				const char* s;
			} ram;
			union
			{
				double* f;
				int32_t* l;
				int16_t* i;
				uint16_t* u;
				uint8_t* b;
				const char* s;
			} eep;
		} s;
	} u;
	uint8_t type        : 4; // Type of function/variable (int, double etc.)
	uint8_t scale       : 4;
	uint8_t rw          : 1; // 0 = read command (returns values to the caller), 1 = write (values to ctlab)
	uint8_t fct         : 2; // 0 = variable access, 1 = function pointer, 2 = special function
} PARAMTABLE;






#ifdef __cplusplus
}
#endif

#endif /* _UNIC_PARSER_H */