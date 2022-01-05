/*
 * Copyright (c) 2007 by Hartmut Birr, Thoralt Franz
 *
 * This program is free software; you can redistribute it and/or
 * mmodify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include <inttypes.h>
#include <avr/interrupt.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// external declaration of all timer variables
//----------------------------------------------------------------------------
extern volatile uint16_t xg_uButtonTimer;
extern volatile uint16_t xg_uMillisecondTimer;
extern volatile uint16_t xg_uScrollTimer;
extern volatile uint16_t xg_uEEPROM_Timer;

volatile uint16_t xg_uPanelUpdateTimer;
volatile uint16_t xg_uMenuBackTimer;

extern volatile uint16_t xg_uXmodemWaitTimer;

#ifdef ENCODER_NAVIGATION
extern volatile uint16_t g_uTurnSymbolTimer;
extern volatile uint16_t g_uEncoderMenuTimer;
#endif

extern volatile uint16_t xg_uiScriptWTS;
extern volatile uint16_t xg_uiScriptWTM;
extern volatile uint16_t xg_uiScriptWTH;
extern volatile uint16_t xg_uiScriptDLY;

//----------------------------------------------------------------------------
// prototypes
//----------------------------------------------------------------------------
void Timer0_Init(void);
void Timer2_Init(void);

//extern void Interrupt2_Init(uint8_t Int2Enable, uint8_t TriggerSlope);
extern void PinChangeInterruptPA3_Init(void);

void wait_ms(uint16_t);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
