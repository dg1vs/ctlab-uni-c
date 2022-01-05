
/*
 * fpga_hw.s
 *
 * Copyright (c) 2012 by Paul Schmid
 *
 * based on FPGA Pascal Code, Copyright (c) Carsten Meyer, c't magazin
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

#include "fpga_hw_config.h"
#include "avr/io.h"

.extern gucBuffer                           ; global buffer with bit-data from SD card

.global FGPA_ConfigBuffer
.func FGPA_ConfigBuffer
FGPA_ConfigBuffer:

// count (uint8_t) is provided in r24 ; value of 0 will result in 256 bytes output
// Port of FPGA config Interface is PORTC

    push    r20                             ; used as storage for current status of Port C
    push    r25                             ; used as storage of byte to be sent to FPGA (MSB first)

    ldi     r31, hi8(gucBuffer)             ; fill Z register (r31:r30) with pointer to buffer
    ldi     r30, lo8(gucBuffer)

.ConfigFGPABufferLoop:
    ld      r25, z+                         ; fetch data and auto post-increment pointer
    cbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK low

    in      r20, FPGAINTERFACEPORT          ; load current status of Port C

    bst     r25,7                           ; save MSB in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high
 
    bst     r25,6                           ; save next Bit in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high

    bst     r25,5                           ; save next Bit in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high

    bst     r25,4                           ; save next Bit in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high

    bst     r25,3                           ; save next Bit in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high

    bst     r25,2                           ; save next Bit in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high

    bst     r25,1                           ; save next Bit in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high

    bst     r25,0                           ; save LSB in T flag
    bld     r20, FPGA_DIN                   ; save T flag in register
    out     FPGAINTERFACEPORT, r20          ; implictly with CCLK set to low
    sbi     FPGAINTERFACEPORT, FPGA_CCLK    ; CCLK high

    dec     r24
    brne    .ConfigFGPABufferLoop           ; max 256 loops

    pop     r25
    pop     r20 

    ret

.endfunc
