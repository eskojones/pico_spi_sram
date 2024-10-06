#ifndef _SRAM_H_
#define _SRAM_H_

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"


/*


  Microchip 23AA04M/23LCV04M SPI/SDI/SQI Serial SRAM IC
  Reference: https://au.mouser.com/datasheet/2/268/23AA04M_23LCV04M_4_Mbit_SPI_SDI_SQI_143_Mhz_Serial-3434931.pdf
  

  NOTES: 
  - Written with 23AA04M in mind, which is a (max) 3.6V IC. I used Pin 36 on the Pico for 3.3V.
  - This library likely works for a number of similar SRAM ICs, especially by those Microchip.
  

  NOT YET IMPLEMENTED:
  - Fast-Read Instruction (see page 17 in reference)
  - SDI and SQI protocol support
  - Page Mode Operation for Read/Write as it doesn't seem functionally different to Seq mode
  - Reset I/O as I'm not sure it would ever be useful?
  - It is assumed the HOLD pin is held to 3.3V (inactive)


  Example configuration:
    //Configure SPI and GPIO pins
    mem_init()
    //Set the SRAM read/write mode to sequential (keep reading or writing while CS is active)
    mem_set_mode(SRAM_MODE_SEQ);
    //Set the SRAM page size to "large" (256 bytes in this IC)
    mem_set_page(SRAM_PAGE_LARGE);
    //Slew rate is currently not understood and seemingly not needed
    mem_set_slew(SRAM_SLEW_RATE3);
    //Set SRAM output drive strength to the default 50%
    mem_set_drive(SRAM_DRIVE_STR5);


  Example usage:
    //Read 100 bytes from 0x001337
    if (mem_seq_read(0x001337, buff, 100) != 100) {
      // read failed
    }
    //Write "foo" to 0x001337
    if (!mem_seq_write(0x001337, "foo", 3)) {
      // write failed
    }


*/


#define SRAM_SPI_PORT spi0 // SPI Port as defined in spi.h
#define SRAM_PIN_MISO 16   // GPIO Pin for MISO
#define SRAM_PIN_CS   17   // GPIO Pin for Chip Select
#define SRAM_PIN_SCK  18   // GPIO Pin for Serial Clock
#define SRAM_PIN_MOSI 19   // GPIO Pin for MOSI


/*
  SRAM Instructions
*/

#define SRAM_INST_READ     0x03 // read (byte(s))
#define SRAM_INST_FASTREAD 0x0B // high-speed read (not yet implemented)
#define SRAM_INST_WRITE    0x02 // write (byte(s))
#define SRAM_INST_EDIO     0x3B // enter dual i/o mode (not yet implemented)
#define SRAM_INST_EQIO     0x38 // enter quad i/o mode (not yet implemented)
#define SRAM_INST_RSTIO    0xFF // reset i/o mode (not yet implemented)
#define SRAM_INST_RDSR     0x05 // read status register -- chip responds with 16bits of status info
#define SRAM_INST_WRSR     0x01 // write status register -- send 16bits status info


/*
  SRAM STATUS Register (16 bits)

  Default register value: 01,0,00,00,0,000,10,100

  BITS   | NAME   | FUNCTION               | DEFAULT | R/W
  -------+--------+------------------------+---------+------
  15,14  | MODE   | 00 = Byte              | 01      | RW
         |        | 10 = Page              |         |
         |        | 01 = Sequential        |         |
  -------+--------+------------------------+---------+------
  13     | ECS    | Error Correction Latch | 0       | R
         |        | 1 = Previous read did  |         |
         |        | require error correct  |         |
  -------+--------+------------------------+---------+------
  12,11  | PROT   | Bus Protocol           | 00      | R
         |        | 00=SPI, 01=SDI, 10=SQI |         |
  -------+--------+------------------------+---------+------
  10,9   | RES    | Reserved for future use| 00      | R
  -------+--------+------------------------+---------+------
  8      | PAGE   | Bytes Per Page         | 0       | RW
         | SIZE   | 0 = 32 Bytes           |         |
         |        | 1 = 256 Bytes          |         |
  -------+--------+------------------------+---------+------
  7,6,5  | RES    | Reserved for future use| 000     | R
  -------+--------+------------------------+---------+------
  4,3    | SLEW   | Output Slew Rate (V/ns)| 10      | RW
         |        | 00 = 1.44   01 = 2.88  |         |
         |        | 10 = 4.33   11 = 6.00  |         |
  -------+--------+------------------------+---------+------
  2,1,0  | DRIVE  | Output Drive Strength% | 100     | RW
         |        | 000 = 12.5  001 = 25.0 |         |
         |        | 010 = 35.0  011 = 42.5 |         |
         |        | 100 = 50.0  101 = 60.0 |         |
         |        | 110 = 75.0  111 = 100.0|         |
  -------+--------+------------------------+---------+------
*/

#define SRAM_MODE_BYTE  0x00 // Byte Mode Operation
#define SRAM_MODE_SEQ   0x01 // Sequential Mode Operation
#define SRAM_MODE_PAGE  0x02 // Page Mode Operation
#define SRAM_PROT_SPI   0x00 // Bus Protocol SPI
#define SRAM_PROT_SDI   0x01 // Bus Protocol SDI
#define SRAM_PROT_SQI   0x02 // Bus Protocol SQI
#define SRAM_PAGE_SMALL 0x00 // 32 Bytes-Per-Page
#define SRAM_PAGE_LARGE 0x01 // 256 Bytes-Per-Page
#define SRAM_SLEW_RATE1 0x00 // 1.44 V/ns
#define SRAM_SLEW_RATE2 0x01 // 2.88 V/ns
#define SRAM_SLEW_RATE3 0x02 // 4.33 V/ns
#define SRAM_SLEW_RATE4 0x03 // 6.00 V/ns
#define SRAM_DRIVE_STR1 0x00 // Output Drive Strength 12.5%
#define SRAM_DRIVE_STR2 0x01 // Output Drive Strength 25%
#define SRAM_DRIVE_STR3 0x02 // Output Drive Strength 35%
#define SRAM_DRIVE_STR4 0x03 // Output Drive Strength 42.5%
#define SRAM_DRIVE_STR5 0x04 // Output Drive Strength 50%
#define SRAM_DRIVE_STR6 0x05 // Output Drive Strength 60%
#define SRAM_DRIVE_STR7 0x06 // Output Drive Strength 75%
#define SRAM_DRIVE_STR8 0x07 // Output Drive Strength 100%

/*
  write one byte to `address`
  returns true on success
*/
bool mem_write_byte(uint32_t address, uint8_t value);
/*
  write bytes from `buff` to `address`
  returns true if all bytes were written
*/
bool mem_seq_write(uint32_t address, char *buff, uint32_t len);
/*
  read one byte from `address`
  returns the read byte
*/
uint8_t mem_read_byte(uint32_t address);
/*
  read bytes from `address` into `buff` for `len` bytes
  returns the number of bytes read
*/
uint32_t mem_seq_read(uint32_t address, char *buff, uint32_t len);
/*
  read the STATUS register
  returns the STATUS register
*/
uint16_t mem_get_status();
/*
  write the STATUS register
*/
void mem_set_status(uint16_t status);
/*
  displays a human readable version of the STATUS register
*/
void mem_print_status();
/*
  set the operation mode for all subsequent read/write calls
  `mode` must be one of the SRAM_MODE_xxx defines
*/
void mem_set_mode(int mode);
/*
  set the page size for all subsequent read/write calls
  `page` must be one of the SRAM_PAGE_xxx defines
*/
void mem_set_page(int page);
/*
  set the output slew rate
  `slew` must be one of the SRAM_SLEW_xxx defines
*/
void mem_set_slew(int slew);
/*
  set the output drive strength
  `drive` must be one of the SRAM_DRIVE_xxx defines
*/
void mem_set_drive(int drive);
/*
  configures the SPI and GPIO pins prior to all other mem_ function calls
*/
void mem_init();

#endif
