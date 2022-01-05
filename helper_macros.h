/*
 * helper_macros.h
 *
 * Created: 23.03.2014 23:46:05
 *  Author: ks
 */ 


#ifndef _HELPER_MACROS_H_
#define _HELPER_MACROS_H_


//----------------------------------------------------------------------------
// some useful shortcuts
//----------------------------------------------------------------------------

#define FALSE 0
#define TRUE 1

#define MIN( a, b ) (a < b) ? a : b
#define MAX( a, b ) (a > b) ? a : b

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define LO(x)   ((x)&0xFF)
#define HI(x)   (((x)>>8)&0xFF)
#define nop()   asm volatile("nop"::)
#define sleep() asm volatile("sleep"::)

// atomic access to multi-byte variables which are modified during interrupts
#define ATOMIC_RW(dst, src) {cli(); dst = src; sei();}

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _HELPER_MACROS_H_ */