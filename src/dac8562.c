#include "dac8562.h"
#include "fpga.h"

#include <unistd.h>
#include <stdlib.h>

static void dac8562_write_reg(mem_map_t mem_map, uint8_t cmd, uint16_t data) {
    uint8_t packet[3];
    packet[0] = cmd;
    packet[1] = (data >> 8) & 0xFF;
    packet[2] = data & 0xFF;

    for (volatile int i = 0; i < 500; ++i);
    spi_transfer(mem_map, packet, 3, DAC8562_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
} 

void dac8562_initialize(mem_map_t mem_map) {

    dac8562_write_reg(mem_map, DAC8562_CMD_RESET_ALL_REG,  DAC8562_DATA_RESET_ALL_REG);
    
    dac8562_write_reg(mem_map,  DAC8562_CMD_PWR_UP_A_B,  DAC8562_DATA_PWR_UP_A_B);
    dac8562_write_reg(mem_map,  DAC8562_CMD_INTERNAL_REF_EN,  DAC8562_DATA_INTERNAL_REF_EN);
    dac8562_write_reg(mem_map,  DAC8562_CMD_GAIN,  DAC8562_DATA_GAIN_B1_A1);
    dac8562_write_reg(mem_map,  DAC8562_CMD_LDAC_DIS,  DAC8562_DATA_LDAC_DIS);



}

void dac8562_set_a(mem_map_t mem_map, uint16_t code) {
    dac8562_write_reg(mem_map,  DAC8562_CMD_SETA_UPDATEA, code);
}

void dac8562_set_b(mem_map_t mem_map, uint16_t code) {
    dac8562_write_reg(mem_map,  DAC8562_CMD_SETB_UPDATEB, code);
}

