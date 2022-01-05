/*
 * helper.c
 *
 * Created: 09.09.2017 20:58:17
 *  Author: ks
 */ 


#include "helper.h"

// ****************************************************************************
// Support functions (note: sscanf_P uses about 1600 bytes of code)
// ****************************************************************************
void HexString2Value(uint8_t* uiPtr, uint32_t *ulpValue)
{
	uint8_t i = 0;
	char c;
	uint32_t ulValue = 0;

	while ( (c= uiPtr[i++]) != 0)
	{
		if ((c >= '0') && (c <= '9'))
		c -= '0';

		if ((c >= 'A') && (c <= 'F'))
		c = c - 'A' + 10;

		if ((c >= 'a') && (c <= 'f'))
		c = c - 'a' + 10;

		ulValue = (ulValue << 4) + c;
	}

	*ulpValue = ulValue;
}


void DecString2Value(uint8_t* uiPtr, uint32_t *ulpValue)
{
	uint8_t i = 0;
	char c;
	uint32_t ulValue = 0;

	while ( (c= uiPtr[i++]) != 0)
	{
		if ((c >= '0') && (c <= '9'))
		c -= '0';

		ulValue = ulValue * 10 + c;
	}

	*ulpValue = ulValue;
}

void LIMIT_float(float *param, float min, float max)
{
	if (*param > max)
	*param = max;
	else if (*param < min)
	*param = min;
}

void LIMIT_UINT8(uint8_t *param, uint8_t min, uint8_t max)
{
	if (*param > max)
	*param = max;
	else if (*param < min)
	*param = min;
}

void LIMIT_INT16(int16_t *param, int16_t min, int16_t max)
{
	if (*param > max)
	*param = max;
	else if (*param < min)
	*param = min;
}