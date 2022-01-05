 /*
 * Copyright (c) 2012 David Rodrigues
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_


//#include <time.h>
#include <string.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

 

extern uint8_t g_ucSlaveCh;

// === auxiliar functions
static inline char *timenow();
//#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__


#define NO_LOG          0x00
#define ERROR_LEVEL     0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03

#ifndef LOG_LEVEL
#define LOG_LEVEL   ERROR_LEVEL
#endif

//ORIG #define PRINTFUNCTION(format, ...)      fprintf(stderr, format, __VA_ARGS__)
#define DbgPrint(...)   printf_P(__VA_ARGS__)

//ORIG #define LOG_FMT             "%s | %-7s | %-15s | %s:%d | "
//ORIG #define LOG_ARGS(LOG_TAG)   timenow(), LOG_TAG, _FILE, __FUNCTION__, __LINE__

//ORIG #define NEWLINE     "\n"

#define ERROR_TAG   "ERROR"
#define INFO_TAG    "INFO"
#define DEBUG_TAG   "DEBUG"

#define CHECKPOINT      do { DbgPrint(PSTR("#%d:-1=(%s:%d)\n"), g_ucSlaveCh, __FILE__, __LINE__); } while(0)

#if LOG_LEVEL >= DEBUG_LEVEL
//ORIG #define LOG_DEBUG(message, args...)     PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(DEBUG_TAG), ## args)
#define LOG_DEBUG(...)     do { DbgPrint(PSTR("#%d:D=(%s:%d) "), g_ucSlaveCh, __FILE__, __LINE__); DbgPrint(__VA_ARGS__); } while(0)
#else
//ORIG #define LOG_DEBUG(message, args...)
#define LOG_DEBUG(...) 
#endif

#if LOG_LEVEL >= INFO_LEVEL
//ORIG #define LOG_INFO(message, args...)      PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(INFO_TAG), ## args)
#define LOG_INFO(...)     do { DbgPrint(PSTR("#%d:I=(%s:%d) "), g_ucSlaveCh, __FILE__, __LINE__); DbgPrint(__VA_ARGS__); } while(0)
#else
//ORIG #define LOG_INFO(message, args...)
#define LOG_INFO(...) 
#endif

#if LOG_LEVEL >= ERROR_LEVEL
//ORIG #define LOG_ERROR(message, args...)     PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TAG), ## args)
#define LOG_ERROR(...)     do { DbgPrint(PSTR("#%d:E=(%s:%d) "), g_ucSlaveCh, __FILE__, __LINE__); DbgPrint(__VA_ARGS__); } while(0)
#else
//ORIG #define LOG_ERROR(message, args...)
#define LOG_ERROR(...) 
#endif

#if LOG_LEVEL >= NO_LOGS
//ORIG #define LOG_IF_ERROR(condition, message, args...) if (condition) PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TAG), ## args)
#define LOG_IF_ERROR(...)     do { DbgPrint(PSTR("#%d:C=(%s:%d) "), g_ucSlaveCh, __FILE__, __LINE__); DbgPrint(__VA_ARGS__); } while(0)
#else
//ORIG #define LOG_IF_ERROR(condition, message, args...)
#define LOG_IF_ERROR(...) 
#endif

static inline char *timenow() 
{
    static char buffer[64];
    //time_t rawtime;
    //struct tm *timeinfo;
    //
    //time(&rawtime);
    //timeinfo = localtime(&rawtime);
    //
    //strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);
    //
    return buffer;
}

#ifdef __cplusplus
extern "C" {
#endif

#endif // _DEBUG_H_





/*

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <inttypes.h>
#include <avr/pgmspace.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


#define DbgPrint(...)   printf_P(__VA_ARGS__)


#ifdef DEBUG

    extern uint8_t g_ucSlaveCh;

#ifndef NDEBUG
#define DPRINT(...)     do { DbgPrint(PSTR("#%d:-1=(%s:%d) "), g_ucSlaveCh, __FILE__, __LINE__); DbgPrint(__VA_ARGS__); } while(0)
#define CHECKPOINT      do { DbgPrint(PSTR("#%d:-1=(%s:%d)\n"), SlaveCh, __FILE__, __LINE__); } while(0)
#else
#define DPRINT(...)
#define CHECKPOINT
#endif

#define DPRINT1(...)    do { DbgPrint(PSTR("#%d:-1=(%s:%d) "), SlaveCh, __FILE__, __LINE__); DbgPrint(__VA_ARGS__); } while(0)
#define CHECKPOINT1     do { DbgPrint(PSTR("#%d:-1(%s:%d)\n"), SlaveCh, __FILE__, __LINE__); } while(0)

#else

#define DPRINT(...)
#define CHECKPOINT

#define DPRINT1(...)
#define CHECKPOINT1
#endif

#ifdef __cplusplus
}
#endif

#endif

*/