#include "sram.h"

//couple of convenience things...
//Get the nth bit of an integer
#define BIT(x, n) ((x >> n) & 1)
//Set the nth bit of an integer
#define BITSET(x, n, v) (v == 1 ? x | (1 << n) : x & (~(1 << n)))





bool mem_write_byte(uint32_t address, uint8_t value) {
    bool ret = true;
    gpio_put(SRAM_PIN_CS, 0);
    uint8_t msg[5] = {
        SRAM_INST_WRITE,
        (address >> 16) & 0xff,
        (address >> 8) & 0xff,
        address & 0xff,
        value
    };
    if (spi_write_blocking(SRAM_SPI_PORT, msg, 5) != 5) {
        ret = false;
    }
    gpio_put(SRAM_PIN_CS, 1);
    return ret;
}


bool mem_seq_write(uint32_t address, char *buff, uint32_t len) {
    bool ret = true;
    gpio_put(SRAM_PIN_CS, 0);
    uint8_t msg[4 + len];
    msg[0] = SRAM_INST_WRITE;
    msg[1] = (address >> 16) & 0xff;
    msg[2] = (address >> 8) & 0xff;
    msg[3] = address & 0xff;
    for (int i = 0; i < len; i++) msg[4 + i] = buff[i];
    if (spi_write_blocking(SRAM_SPI_PORT, msg, 4 + len) != 4 + len) {
        ret = false;
    }
    gpio_put(SRAM_PIN_CS, 1);
    return ret;
}


uint8_t mem_read_byte(uint32_t address) {
    char buff[1];
    uint8_t msg[4] = {
        SRAM_INST_READ,
        (address >> 16) & 0xff,
        (address >> 8) & 0xff,
        address & 0xff
    };
    gpio_put(SRAM_PIN_CS, 0);
    spi_write_blocking(SRAM_SPI_PORT, msg, 4);
    spi_read_blocking(SRAM_SPI_PORT, 0, buff, 1);
    gpio_put(SRAM_PIN_CS, 1);
    return buff[0];
}


uint32_t mem_seq_read(uint32_t address, char *buff, uint32_t len) {
    uint8_t msg[4] = {
        SRAM_INST_READ,
        (address >> 16) & 0xff,
        (address >> 8) & 0xff,
        address & 0xff
    };
    gpio_put(SRAM_PIN_CS, 0);
    spi_write_blocking(SRAM_SPI_PORT, msg, 4);
    int bytes_read = spi_read_blocking(SRAM_SPI_PORT, 0, buff, len);
    gpio_put(SRAM_PIN_CS, 1);
    return bytes_read;
}


uint16_t mem_get_status() {
    gpio_put(SRAM_PIN_CS, 0);
    uint8_t inst = SRAM_INST_RDSR;
    spi_write_blocking(SRAM_SPI_PORT, &inst, 1);
    uint8_t status[2];
    spi_read_blocking(SRAM_SPI_PORT, 0, status, 2);
    gpio_put(SRAM_PIN_CS, 1);
    return (uint16_t)((status[0] << 8) + status[1]);
}


void mem_set_status(uint16_t status) {
    gpio_put(SRAM_PIN_CS, 0);
    uint8_t inst = SRAM_INST_WRSR;
    spi_write_blocking(SRAM_SPI_PORT, &inst, 1);
    uint8_t msg[2] = { (status >> 8), status & 0xff };
    spi_write_blocking(SRAM_SPI_PORT, msg, 2);
    gpio_put(SRAM_PIN_CS, 1);
}


void mem_print_status() {
    uint16_t status = mem_get_status();
    printf("--- Mem Status ---\n");
    uint8_t bits[] = {
        (BIT(status, 15) << 1) + BIT(status, 14), //mode
        BIT(status, 13), //ecs
        (BIT(status, 12) << 1) + BIT(status, 11), //protocol
        BIT(status, 8), //page size
        (BIT(status, 4) << 1) + BIT(status, 3), //output slew rate
        (BIT(status, 2) << 2) + (BIT(status, 1) << 1) + BIT(status, 0) //output drive strength
    };

    char mode[5] = "";
    if      (bits[0] == SRAM_MODE_BYTE) sprintf(mode, "Byte");
    else if (bits[0] == SRAM_MODE_PAGE) sprintf(mode, "Page");
    else if (bits[0] == SRAM_MODE_SEQ)  sprintf(mode, "Seq");

    char protocol[4] = "";
    if      (bits[2] == SRAM_PROT_SPI) sprintf(protocol, "SPI");
    else if (bits[2] == SRAM_PROT_SDI) sprintf(protocol, "SDI");
    else if (bits[2] == SRAM_PROT_SQI) sprintf(protocol, "SQI");

    char slew[10] = "";
    if      (bits[4] == SRAM_SLEW_RATE1) sprintf(slew, "1.44 V/ns");
    else if (bits[4] == SRAM_SLEW_RATE2) sprintf(slew, "2.88 V/ns");
    else if (bits[4] == SRAM_SLEW_RATE3) sprintf(slew, "4.33 V/ns");
    else if (bits[4] == SRAM_SLEW_RATE4) sprintf(slew, "6.00 V/ns");

    char drive[6] = "";
    if      (bits[5] == SRAM_DRIVE_STR1) sprintf(drive, "12.5%");
    else if (bits[5] == SRAM_DRIVE_STR2) sprintf(drive, "25.0%");
    else if (bits[5] == SRAM_DRIVE_STR3) sprintf(drive, "35.0%");
    else if (bits[5] == SRAM_DRIVE_STR4) sprintf(drive, "42.5%");
    else if (bits[5] == SRAM_DRIVE_STR5) sprintf(drive, "50.0%");
    else if (bits[5] == SRAM_DRIVE_STR6) sprintf(drive, "60.0%");
    else if (bits[5] == SRAM_DRIVE_STR7) sprintf(drive, "75.0%");
    else if (bits[5] == SRAM_DRIVE_STR7) sprintf(drive, "100%");

    printf("Raw Received: %04x\n" \
           "Mode: %s\n" \
           "Error Correction Flag: %s\n" \
           "Protocol: %s\n" \
           "Page Size: %s\n" \
           "Output Slew Rate: %s\n" \
           "Output Drive Strength: %s\n",
           status, mode, bits[1] == 1 ? "On" : "Off", protocol, 
           bits[3] == 1 ? "256 Bytes" : "32 Bytes", slew, drive
    );
}


void mem_set_mode(int mode) {
    uint16_t status = mem_get_status();
    status = BITSET(status, 15, BIT(mode, 1));
    status = BITSET(status, 14, BIT(mode, 0));
    mem_set_status(status);
}


void mem_set_page(int page) {
    uint16_t status = mem_get_status();
    status = BITSET(status, 8, page);
    mem_set_status(status);
}


void mem_set_slew(int slew) {
    uint16_t status = mem_get_status();
    status = BITSET(status, 4, BIT(slew, 1));
    status = BITSET(status, 3, BIT(slew, 0));
    mem_set_status(status);
}


void mem_set_drive(int drive) {
    uint16_t status = mem_get_status();
    status = BITSET(status, 2, BIT(drive, 2));
    status = BITSET(status, 1, BIT(drive, 1));
    status = BITSET(status, 0, BIT(drive, 0));
    mem_set_status(status);
}


void mem_init() {
    uint MHz = 50;
    uint baud = spi_init(SRAM_SPI_PORT, MHz * 1000 * 1000);
    spi_set_format(SRAM_SPI_PORT, 8 /* bits */, 1 /* polarity */, 1 /* phase */, SPI_MSB_FIRST);
    gpio_set_function(SRAM_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SRAM_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SRAM_PIN_MISO, GPIO_FUNC_SPI);
    gpio_init(SRAM_PIN_CS);
    gpio_set_dir(SRAM_PIN_CS, GPIO_OUT);
    gpio_put(SRAM_PIN_CS, 1);
}


