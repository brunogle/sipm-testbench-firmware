#ifndef DAC8562_H
#define DAC8562_H

#include <stdint.h>
#include "fpga.h"

#define DAC8562_CMD_SETA_UPDATEA          0x18
#define DAC8562_CMD_SETB_UPDATEB          0x19
#define DAC8562_CMD_UPDATE_ALL_DACS       0x0F
#define DAC8562_CMD_GAIN                  0x02 
#define DAC8562_CMD_PWR_UP_A_B            0x20
#define DAC8562_CMD_RESET_ALL_REG         0x28 
#define DAC8562_CMD_LDAC_DIS              0x30
#define DAC8562_CMD_INTERNAL_REF_DIS      0x38
#define DAC8562_CMD_INTERNAL_REF_EN       0x38 

#define DAC8562_DATA_GAIN_B2_A2           0x0000
#define DAC8562_DATA_GAIN_B2_A1           0x0001
#define DAC8562_DATA_GAIN_B1_A2           0x0002
#define DAC8562_DATA_GAIN_B1_A1           0x0003     
#define DAC8562_DATA_PWR_UP_A_B           0x0003 
#define DAC8562_DATA_RESET_ALL_REG        0x0001
#define DAC8562_DATA_LDAC_DIS             0x0003
#define DAC8562_DATA_INTERNAL_REF_DIS     0x0000
#define DAC8562_DATA_INTERNAL_REF_EN      0x0001

#define DAC8562_SPI_SLAVE_NUM         0

void dac8562_initialize(mem_map_t mem_map);
void dac8562_set_a(mem_map_t mem_map, uint16_t code);
void dac8562_set_b(mem_map_t mem_map, uint16_t code);

#endif