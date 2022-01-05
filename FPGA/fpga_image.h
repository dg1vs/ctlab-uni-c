/*
extern const char FPGA_image[] PROGMEM;
extern const char FPGA_image_end[] PROGMEM;
extern const char FPGA_image_size_sym[];
#define FPGA_image_size ((int)FPGA_image_size_sym)
*/

#include <avr/io.h>
#include <avr/pgmspace.h>

extern const char _binary_FPGA_image_bit_start[] PROGMEM;
extern const char _binary_FPGA_image_bit_end[] PROGMEM;
extern const uint16_t _binary_FPGA_image_bit_size PROGMEM;