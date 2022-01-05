/*
 * fpga_hw_config.h
 *
 * Copyright (c) 2012 by Paul Schmid
 * Copyright (c) 2017 by Karsten Schmidt
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


#ifndef _FPGA_HW_H_
#define _FPGA_HW_H_

#define FPGAINTERFACEPORT _SFR_IO_ADDR(PORTA)

#define FPGA_DIN   PA4   // out
#define FPGA_CCLK  PA5   // out
#define FPGA_PROG  PA6   // out
#define FPGA_DONE  PA7   // in 


// NET "F_DS"      LOC = "P51" | IOSTANDARD = LVCMOS33 | CLOCK_DEDICATED_ROUTE = FALSE;  # Data register R/W, ist bei Konf. DIN
// NET "F_RS"      LOC = "P53" | IOSTANDARD = LVCMOS33 | CLOCK_DEDICATED_ROUTE = FALSE;  # Control register write, ist bei Konf. CCLK
// a_DATASEL                  = 4; //FPGA SPI-Register bis 32 Bit
// a_REGSEL                   = 5; //FPGA Registerauswahl 16 Bit mit Write Enable

// Check chip-select!!!!
#define FPGA_SPI_DS   PA4
#define FPGA_SPI_RS   PA5
#define FPGA_BIT_SPI_DS   4
#define FPGA_BIT_SPI_RS   5

//#define FPGA_SPI_DS   PB3
//#define FPGA_SPI_RS   PB1
//#define FPGA_BIT_SPI_DS   3
//#define FPGA_BIT_SPI_RS   1

// #define FPGA_BIT_SPI_AUX  0

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* FPGA_HW_H_ */