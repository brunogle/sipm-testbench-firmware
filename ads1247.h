#include <stdint.h>
#include <stdbool.h>

#define ADC_SPI_SLAVE_NUM  1

// --- Command opcodes  ---
#define CMD_WAKEUP      0x00
#define CMD_SLEEP       0x02
#define CMD_SYNC        0x04
#define CMD_RESET       0x06
#define CMD_RDATA       0x12
#define CMD_RDATAC      0x14
#define CMD_SDATAC      0x16
#define CMD_RREG        0x20
#define CMD_WREG        0x40
#define CMD_SYSOCAL     0x60
#define CMD_SYSGCAL     0x61
#define CMD_SELFOCAL    0x62
#define CMD_NOP         0xFF

// --- Register addresses ---
#define REG_MUX0        0x00
#define REG_VBIAS       0x01
#define REG_MUX1        0x02
#define REG_SYS0        0x03
#define REG_IDAC0       0x0A
#define REG_IDAC1       0x0B
#define REG_GPIOCFG     0x0C
#define REG_GPIODIR     0x0D
#define REG_GPIODAT     0x0E

// --- Hardware Specific Configurations ---

#define MUX0_CURR_SENSE 0x13
#define MUX0_VOLT_SENSE 0x04 // AIN1=positive   AIN0=negative

// --- Default configuration

#define VBIAS_DEFAULT 0x00 // No Vbias
#define MUX1_DEFAULT 0x30 // Internal reference always on and used, normal operation
#define SYS0_DEFAULT 0x00 // PGA=1 DR=5SPS
#define IDAC0_DEFAULT 0x00

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



void ads1247_send_command(void *spi_map, uint8_t cmd);
void ads1247_reset(void *spi_map);
void ads1247_write_reg(void *spi_map, uint8_t reg, uint8_t data);
uint8_t ads1247_read_reg(void *spi_map, uint8_t reg);
void ads1247_init(void *spi_map, void *gpio_map);
void ads1247_set_sys(void *spi_map, int pga_cfg, int dr_cfg);
char ads1247_data_ready(void *gpio_map);
int32_t ads1247_read_data(void *spi_map);
float ads1247_code_to_voltage(int32_t code, int pga, char * saturated);