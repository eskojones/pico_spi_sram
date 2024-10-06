#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "stdbool.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "sram.h"




int main()
{
    stdio_init_all();
    sleep_ms(5000);

    //init sram and configure for sequential mode with 256byte pages
    mem_init();
    mem_set_mode(SRAM_MODE_SEQ);
    mem_set_page(SRAM_PAGE_LARGE);
    mem_set_slew(SRAM_SLEW_RATE3);
    mem_set_drive(SRAM_DRIVE_STR5);
    mem_print_status();

    srand(1337);
    bool running = true;
    int count = 0;
    while(running) {
        char buff[256], source[256];

        // make a random page of chars
        for (uint32_t i = 0; i < 256; i++) {
            buff[i] = 65 + (rand() % 26);
        }

        // random page address
        uint32_t addr = (rand() % 2048) * 256; 

        // write `buff` to the page
        if (!mem_seq_write(addr, buff, 256)) {
            printf("\n\n!!! Failed to write the whole page @ %06x !!!\n\n", addr);
            while (true) sleep_ms(1);
        }

        // copy `buff` into `source` and zero `buff`
        memcpy(source, buff, 256);
        memset(buff, 0, 256);

        // read the page back into `buff`
        if (mem_seq_read(addr, buff, 256) != 256) {
            printf("\n\n!!! Failed to read the whole page @ %06x !!!\n\n", addr);
            while (true) sleep_ms(1);
        }

        // compare `source[]` and `buff[]`
        for (uint32_t i = 0; i < 256; i += 16) {
            printf("0x%06x: ", addr + i);
            for (uint32_t j = 0; j < 16; j++) {
                printf("%02x ", (uint8_t)buff[i+j]);
                if (buff[i+j] != source[i+j]) {
                    printf(
                        "\n\n!!! Read doesnt match Write @ 0x%06x (%02x != %02x) !!!\n\n",
                        addr + (i * 16), source[i+j], buff[i+j]
                    );
                    while (true) sleep_ms(1);
                }
            }
            printf("\n");
        }

        sleep_ms(1000);
    }

    return 0;

}
