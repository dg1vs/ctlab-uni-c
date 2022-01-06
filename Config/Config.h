#ifndef __CONFIG_H__
#define __CONFIG_H__

// Please keep in mind, this file will also included in some assembler files.
// So keep this file simple, otherwise strange error can occur 


// ******************************************************************************************************************
// *** user configurable parameters *********************************************************************************
// ******************************************************************************************************************
//#define STRICTSYNTAX                    // requires the address and colon in front of the command; malformed/broken commands are rejected


// #define FIX_CAPSLOCKBUG              // there are some FPGA BASIC versions with a strange behavior regarding the CAPSlock
// #define UNUSED_SD_COMMANDS           // compile with unused SD commands


// ******************************************************************************************************************
// *** internal debugging settings **********************************************************************************
// ******************************************************************************************************************
// #define LOG_LEVEL   INFO_LEVEL
// #define LOG_LEVEL   ERROR_LEVEL

// ******************************************************************************************************************
// *** version strings **********************************************************************************************
// ******************************************************************************************************************
#define VERSSTRSHORT "UNI-C0.4"         // 8 chars max.
#define VERSSTRLONG  "0.4 [UNIC by DKS]"
#define VERSSTRADD   " beta4  "         // 8 chars max.



// **********************************************************************************************



#endif
