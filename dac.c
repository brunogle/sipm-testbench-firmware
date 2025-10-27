
#include "dac.h"
#include "fpga.h"

#include <unistd.h>
#include <stdlib.h>

static void dac_write_reg(void * spi_map, uint8_t cmd, uint16_t data) {
    uint8_t packet[3];
    packet[0] = cmd;
    packet[1] = (data >> 8) & 0xFF;
    packet[2] = data & 0xFF;
    for (volatile int i = 0; i < 500; ++i);
    
    spi_transfer(spi_map, packet, 3, DAC_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
}

void dac_initialize(void * spi_map) {
    
    spi_init(spi_map);

    dac_write_reg(spi_map, CMD_RESET_ALL_REG, DATA_RESET_ALL_REG);
    
    dac_write_reg(spi_map, CMD_PWR_UP_A_B, DATA_PWR_UP_A_B);
    dac_write_reg(spi_map, CMD_INTERNAL_REF_EN, DATA_INTERNAL_REF_EN);
    dac_write_reg(spi_map, CMD_GAIN, DATA_GAIN_B1_A1);
    dac_write_reg(spi_map, CMD_LDAC_DIS, DATA_LDAC_DIS);
}

void dac_set_a(void * spi_map, uint16_t code) {
    dac_write_reg(spi_map, CMD_SETA_UPDATEA, code);
}

void dac_set_b(void * spi_map, uint16_t code) {
    dac_write_reg(spi_map, CMD_SETB_UPDATEB, code);
}

