/*
 * Copyright (c) 2011 by Paul Schmid
 *
 * Copyright (c) 2007 by Hartmut Birr, Thoralt Franz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
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

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "Timer.h"
#include "Encoder.h"

#include "sd_raw_config.h"
#include "fpga.h"
#include "fpga_hw_config.h"
#include "Rtc.h"
#include "parser.h"
#include "helper_macros.h"


//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------
// the following timers are decremented once every 500 us until they reach 0

// timer to avoid button bounce
volatile uint16_t xg_uButtonTimer = 0;
// timer for wait_ms()
volatile uint16_t xg_uMillisecondTimer = 0;
// timer for RTC
volatile uint16_t xg_uSecondTimer = 2000; // 2000 * 0.5ms = 1s
// timer for panel text scrolling
volatile uint16_t xg_uScrollTimer = 0;
// timer for panel value update
volatile uint16_t xg_uPanelUpdateTimer = 0;
// timer for menu fallback time
volatile uint16_t xg_uMenuBackTimer = 0;
// timer for EEPROM write
volatile uint16_t xg_uEEPROM_Timer = 0;
// timer for waiting for Xmodem characters
volatile uint16_t xg_uXmodemWaitTimer = 0;
// timer for ini script command WTS (second)
volatile uint16_t xg_uiScriptWTS = 0;

volatile uint16_t ucScriptTicker = 0;

// timer for ini script command WTM (minute)
volatile uint16_t xg_uiScriptWTM = 0;
// timer for ini script command WTH (hour)
volatile uint16_t xg_uiScriptWTH = 0;
// timer for ini script command DLY (milliseconds)
volatile uint16_t xg_uiScriptDLY = 0;
// semaphore to avoid recursive triggering of jobParse...()
volatile uint8_t xg_ucParseSema = 0;

//----------------------------------------------------------------------------
// ISR(TIMER0_COMP_vect)
//
// Timer 0 compare interrupt
//
// -> --
// <- --
//----------------------------------------------------------------------------

// TODO
// Brauchen wir das für Uni-C da nur ein Modul!?!?!?!?
// make sure to process lab-bus commands for other modules while lengthy internal operations in this module take place
#if defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
ISR(TIMER0_COMPA_vect)
#else
#error Please define your TIMER0 code
#endif
{
    if ( (g_ucModuleBusy != 0) && !xg_ucParseSema)
    {
        xg_ucParseSema = 1;     // avoid recursive calling

        sei();                  // re-enable interrupts again to make sure other interrupts are handled with higher priority

        jobParseData();         // make sure to process lab-bus commands for other modules while lengthy internal operations in this module take place
        
		// TODO KS
		// jobParseFPGACommand();  // make also sure that commands sent by TTF to Basic core that come back to the ATmega are processed right away.

        xg_ucParseSema = 0;
    }
}


//----------------------------------------------------------------------------
// ISR(TIMER2_COMP_vect)
//
// Timer 2 compare interrupt
//
// -> --
// <- --
//----------------------------------------------------------------------------
#if defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
ISR(TIMER2_COMPA_vect)
#else
#error Please define your TIMER2 code
#endif
{
    // count down button press timer
    if(xg_uButtonTimer) xg_uButtonTimer--;
    // count down button press timer
    if(xg_uMillisecondTimer) xg_uMillisecondTimer--;
    // count down panel scroll text timer
    if(xg_uScrollTimer) xg_uScrollTimer--;
    // count down panel value update
    if(xg_uPanelUpdateTimer) xg_uPanelUpdateTimer--;
    // count down menu fallback time
    if(xg_uMenuBackTimer) xg_uMenuBackTimer--;
    // count down Xmodem characters timer
	if(xg_uXmodemWaitTimer) xg_uXmodemWaitTimer--;
    // count down EEPROM timer
    if(xg_uEEPROM_Timer) xg_uEEPROM_Timer--;

    // ************************************************************
    // count down ini script command DLY (milliseconds)
    if(ucScriptTicker) 
	{
		ucScriptTicker--;
	}

    if ((xg_uiScriptDLY) && (ucScriptTicker == 0))
    {
        xg_uiScriptDLY--;
        ucScriptTicker = 2;
    }

    // ************************************************************
    // count down second timer
    if (xg_uSecondTimer)
    {
        xg_uSecondTimer--;
    }
    else
    {
        // handle RTC
        xg_uSecondTimer = 2000; // 2000 * 0.5ms = 1s

        gRTC.ucSeconds++;

        if (xg_uiScriptWTS)
        {
            xg_uiScriptWTS--;
        }

        if (gRTC.ucSeconds > 59)
        {
            gRTC.ucSeconds = 0;
            gRTC.ucMinutes++;

            if (xg_uiScriptWTM)
            {
                xg_uiScriptWTM--;
            }

            if (gRTC.ucMinutes > 59)
            {
                gRTC.ucMinutes = 0;
                gRTC.ucHours++;

                if (xg_uiScriptWTH)
                {
                    xg_uiScriptWTH--;
                }

                if (gRTC.ucHours > 23)
                {
                    gRTC.ucHours = 0;
                    gRTC.ucDay++;

                    if ( (gRTC.ucDay > 31) ||																									// January, March, May, July, August, October, December
                            ((gRTC.ucDay > 30) && ((gRTC.ucMonth == 4) || (gRTC.ucMonth == 6) || (gRTC.ucMonth == 9) || (gRTC.ucMonth == 11) )) ||	// April, June, September, November
                            ((gRTC.ucDay > 29) && (gRTC.ucMonth == 2) && ((gRTC.ucYear % 4) == 0)) ||												// February (leap-year)
                            ((gRTC.ucDay > 28) && (gRTC.ucMonth == 2) && ((gRTC.ucYear % 4) != 0)) )												// February (non leap-year)
                    {
                        gRTC.ucDay = 1;
                        gRTC.ucMonth++;
                        if (gRTC.ucMonth > 12)
                        {
                            gRTC.ucMonth = 1;
                            gRTC.ucYear++;
                            gRTC.uiYearWord++;
                            if (gRTC.ucYear > 99)
                            {
                                gRTC.ucYear = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    Encoder_MainFunction();
}




//----------------------------------------------------------------------------
// InitPcInt2()
//----------------------------------------------------------------------------
void PinChangeInterruptPA3_Init(void)
{
#if defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
//    PCICR  |= (1 << PCIE0);		// Pin Change Interrupt Enable 2
//    PCMSK0 |= (1 << PCINT3);	// Pin Change Enable Mask 3 (= PA3 ) "card present" switch
#else
#error Please define your Interrupt code
#endif
}


//----------------------------------------------------------------------------
// ISR(PCINT2) SD card holder switch
//----------------------------------------------------------------------------
ISR(PCINT0_vect)
{
    if (get_pin_available() && g_ucSDpresent)	// if "card present" switch is open --> high
    {
        g_ucSDpresent = FALSE;
        g_ucSDdisable = TRUE;
    }
}

//----------------------------------------------------------------------------
// InitTimer0
//
// Initialize Timer 0 to interrupt every 8 ms
//
// -> --
// <- --
//----------------------------------------------------------------------------
void Timer0_Init(void)
{
    uint8_t sreg = SREG;
    cli();

#if (__AVR_ATmega324P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)

    // normal stuff -> 16 MHz
    // Timer 0 initialisieren, Periode 8ms, Vorteiler 1024, CTC Mode
    OCR0A = (F_CPU / 128000UL) - 1;
    OCR0B = 0;
    TCNT0 = 0;
    TCCR0A = (1<<WGM01);
    TCCR0B = (1<<CS02)|(1<<CS00);
    TIMSK0 |= (1 << OCIE0A);

#else
#error Please define your TIMER0 code or check the avr manual
#endif
    SREG = sreg;
}


//----------------------------------------------------------------------------
// InitTimer2
//
// Initialize Timer2 to interrupt every 500 us
//
// -> --
// <- --
//----------------------------------------------------------------------------
void Timer2_Init(void)
{
    uint8_t sreg = SREG;
    cli();

#if (__AVR_ATmega324P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)

#if (F_CPU == 20000000)
    // 20 MHz special hardware -> dg1vs
    // Timer 2 initialisieren, Periode 500us, Vorteiler 64, CTC Mode
    OCR2A = (F_CPU / 128000UL) - 1;
    OCR2B = 0;
    TCNT2 = 0;
    TCCR2A = (1<<WGM21);
    TCCR2B = (1<<CS22);
    TIMSK2 |= (1 << OCIE2A);
#else
    // normal stuff -> 16 MHz
    // Timer 2 initialisieren, Periode 500us, Vorteiler 32, CTC Mode
    OCR2A = (F_CPU / 64000UL) - 1;
    OCR2B = 0;
    TCNT2 = 0;
    TCCR2A = (1<<WGM21);
    TCCR2B = (1<<CS21)|(1<<CS20);
    TIMSK2 |= (1 << OCIE2A);
#endif

#else
#error Please define your TIMER2 code or check the avr manual
#endif
    SREG = sreg;
}

//----------------------------------------------------------------------------
// wait_ms()
//
// Waits a given number of milliseconds
//
// -> uMS = milliseconds to wait
// <- --
//----------------------------------------------------------------------------
void wait_ms(uint16_t uMS)
{
    uint16_t u;
    ATOMIC_RW(xg_uMillisecondTimer, (uMS<<1));
    do
    {
        ATOMIC_RW(u, xg_uMillisecondTimer);
    }
    while(u);
}



