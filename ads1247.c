#include "ads1247.h"
#include "fpga.h"
#include "gpio.h"


#include <unistd.h>

/*
Send command to ADS1247
*/
void ads1247_send_command(void *spi_map, uint8_t cmd){
    uint8_t packet[1] = { cmd }; // Commmands are always 1 byte
    spi_transfer(spi_map, packet, 1, ADC_SPI_SLAVE_NUM);
}


/*
Writes ADS1247 register
*/
void ads1247_write_reg(void *spi_map, uint8_t reg, uint8_t data){
    
    uint8_t packet[3];
    packet[0] = CMD_WREG | (reg & 0x0F); // Command + start address
    packet[1] = 0x00;                    // Write 1 register
    packet[2] = data;                    // Data byte
    spi_transfer(spi_map, packet, 3, ADC_SPI_SLAVE_NUM);
    usleep(1000);
}

/*
Reads ADS1247 Register
*/
uint8_t ads1247_read_reg(void *spi_map, uint8_t reg){
    uint8_t tx[2], rx[1];
    tx[0] = CMD_RREG | (reg & 0x0F);
    tx[1] = 0x00; // Read 1 register
    for (volatile int i = 0; i < 500; ++i);
    spi_transfer(spi_map, tx, 2, ADC_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
    spi_receive(spi_map, rx, 1, ADC_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
    return rx[0];
}

/*
Resets ADS1247
*/
void ads1247_reset(void *spi_map){
    ads1247_send_command(spi_map, CMD_RESET);
    usleep(10000);
}

void ads1247_set_sys(void *spi_map, int pga_cfg, int dr_cfg){
    ads1247_write_reg(spi_map, REG_SYS0, pga_cfg << 4 | dr_cfg); 
}

/*
Initialize ADS1247
*/
void ads1247_init(void *spi_map, void *gpio_map){

    set_gpio_bit(gpio_map, GPIO_ADC_RESET, 1);
    set_gpio_bit(gpio_map, GPIO_ADC_START, 1);


    ads1247_reset(spi_map);

    
    
    ads1247_write_reg(spi_map, REG_MUX0, MUX0_CURR_SENSE); 
    ads1247_write_reg(spi_map, REG_VBIAS, VBIAS_DEFAULT);
    ads1247_write_reg(spi_map, REG_MUX1, MUX1_DEFAULT);
    ads1247_write_reg(spi_map, REG_SYS0, SYS0_DEFAULT);
    ads1247_write_reg(spi_map, REG_IDAC0, IDAC0_DEFAULT);

    ads1247_send_command(spi_map, CMD_SDATAC);   // Stop continuous read mode
}


/*
Initialize ADS1247
*/
int32_t ads1247_read_data(void *spi_map){
    uint8_t cmd = CMD_RDATA;
    uint8_t rx[3];
    for (volatile int i = 0; i < 500; ++i);
    spi_transfer(spi_map, &cmd, 1, ADC_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
    spi_receive(spi_map, rx, 3, ADC_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);

    int32_t result = ((int32_t)rx[0] << 16) |
                     ((int32_t)rx[1] << 8) |
                     ((int32_t)rx[2]);

    // Sign-extend to 32 bits
    if (result & 0x800000)
        result |= 0xFF000000;

    return result;
}

/*
Converts ADS1247 code to voltage
*/
float ads1247_code_to_voltage(int32_t code, int pga, char * saturated){
    float voltage = (code / (float)(1 << 23)) * (ADS1247_INTERNAL_VREF / pga);
    

    if(saturated != NULL){
        *saturated = ((code == (int32_t)0x7FFFFF) || (code == (int32_t)0xFF800000)) ? 1 : 0;
    }

    return voltage;
}

char ads1247_data_ready(void * gpio_map){

    return (read_gpio_bit(gpio_map, GPIO2_ADC_DRDY) == 0);
}
