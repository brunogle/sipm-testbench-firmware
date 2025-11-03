#ifndef ADS1247_H
#define ADS1247_H


#include <stdint.h>
#include <stdbool.h>
#include "fpga.h"

// --- Command opcodes  ---
#define ADS1247_CMD_WAKEUP      0x00
#define ADS1247_CMD_SLEEP       0x02
#define ADS1247_CMD_SYNC        0x04
#define ADS1247_CMD_RESET       0x06
#define ADS1247_CMD_RDATA       0x12
#define ADS1247_CMD_RDATAC      0x14
#define ADS1247_CMD_SDATAC      0x16
#define ADS1247_CMD_RREG        0x20
#define ADS1247_CMD_WREG        0x40
#define ADS1247_CMD_SYSOCAL     0x60
#define ADS1247_CMD_SYSGCAL     0x61
#define ADS1247_CMD_SELFOCAL    0x62
#define ADS1247_CMD_NOP         0xFF

// --- Register addresses ---
#define ADS1247_REG_MUX0        0x00
#define ADS1247_REG_VBIAS       0x01
#define ADS1247_REG_MUX1        0x02
#define ADS1247_REG_SYS0        0x03
#define ADS1247_REG_IDAC0       0x0A
#define ADS1247_REG_IDAC1       0x0B
#define ADS1247_REG_GPIOCFG     0x0C
#define ADS1247_REG_GPIODIR     0x0D
#define ADS1247_REG_GPIODAT     0x0E

// --- Hardware Specific Configurations ---

#define ADS1247_SPI_SLAVE_NUM  1
#define ADS1247_MUX0_CURR_SENSE 0x13
#define ADS1247_MUX0_VOLT_SENSE 0x08 // AIN1=positive   AIN0=negative

// --- Default configuration

#define ADS1247_VBIAS_DEFAULT 0x00 // No Vbias
#define ADS1247_MUX1_DEFAULT 0x30 // Internal reference always on and used, normal operation
#define ADS1247_SYS0_DEFAULT 0x00 // PGA=1 DR=5SPS
#define ADS1247_IDAC0_DEFAULT 0x00

// --- Constants
# define ADS1247_INTERNAL_VREF 2.048f

// --- Configuration Options
#define ADS1247_PGA_1   0x0
#define ADS1247_PGA_2   0x1
#define ADS1247_PGA_4   0x2
#define ADS1247_PGA_8   0x3
#define ADS1247_PGA_16  0x4
#define ADS1247_PGA_32  0x5
#define ADS1247_PGA_64  0x6
#define ADS1247_PGA_128 0x7

#define ADS1247_DR_5     0x0
#define ADS1247_DR_10    0x1
#define ADS1247_DR_20    0x2
#define ADS1247_DR_40    0x3
#define ADS1247_DR_80    0x4
#define ADS1247_DR_160   0x5
#define ADS1247_DR_320   0x6
#define ADS1247_DR_640   0x7
#define ADS1247_DR_1000  0x8
#define ADS1247_DR_2000  0x9



void ads1247_send_command(mem_map_t mem_map, uint8_t cmd);
void ads1247_reset(mem_map_t mem_map);
void ads1247_write_reg(mem_map_t mem_map, uint8_t reg, uint8_t data);
uint8_t ads1247_read_reg(mem_map_t mem_map, uint8_t reg);
void ads1247_init(mem_map_t mem_map);
void ads1247_set_sys(mem_map_t mem_map, int pga_cfg, int dr_cfg);
char ads1247_data_ready(mem_map_t mem_map);
int32_t ads1247_read_data(mem_map_t mem_map);
float ads1247_code_to_voltage(int32_t code, int pga, char * saturated);

#endif