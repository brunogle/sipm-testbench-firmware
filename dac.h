#include <stdint.h>


#define CMD_SETA_UPDATEA          0x18
#define CMD_SETB_UPDATEB          0x19
#define CMD_UPDATE_ALL_DACS       0x0F

#define CMD_GAIN                  0x02 
#define DATA_GAIN_B2_A2           0x0000
#define DATA_GAIN_B2_A1           0x0001
#define DATA_GAIN_B1_A2           0x0002
#define DATA_GAIN_B1_A1           0x0003     

#define CMD_PWR_UP_A_B            0x20
#define DATA_PWR_UP_A_B           0x0003 

#define CMD_RESET_ALL_REG         0x28 
#define DATA_RESET_ALL_REG        0x0001

#define CMD_LDAC_DIS              0x30
#define DATA_LDAC_DIS             0x0003

#define CMD_INTERNAL_REF_DIS      0x38
#define DATA_INTERNAL_REF_DIS     0x0000
#define CMD_INTERNAL_REF_EN       0x38 
#define DATA_INTERNAL_REF_EN      0x0001

#define DAC_SPI_SLAVE_NUM         0

void dac_initialize(void * spi_map);
void dac_set_a(void * spi_map, uint16_t code);
void dac_set_b(void * spi_map, uint16_t code);