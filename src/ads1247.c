#include "ads1247.h"
#include "fpga.h"


#include <unistd.h>

/*
Send command to ADS1247
*/
void ads1247_send_command(mem_map_t mem_map, uint8_t cmd){
    uint8_t packet[1] = { cmd }; // Commmands are always 1 byte
    spi_transfer(mem_map, packet, 1, ADS1247_SPI_SLAVE_NUM);
}


/*
Writes ADS1247 register
*/
void ads1247_write_reg(mem_map_t mem_map, uint8_t reg, uint8_t data){
    
    uint8_t packet[3];
    packet[0] = ADS1247_CMD_WREG | (reg & 0x0F); // Command + start address
    packet[1] = 0x00;                    // Write 1 register
    packet[2] = data;                    // Data byte
    spi_transfer(mem_map, packet, 3, ADS1247_SPI_SLAVE_NUM);
    usleep(1000);
}

/*
Reads ADS1247 Register
*/
uint8_t ads1247_read_reg(mem_map_t mem_map, uint8_t reg){
    uint8_t tx[2], rx[1];
    tx[0] = ADS1247_CMD_RREG | (reg & 0x0F);
    tx[1] = 0x00; // Read 1 register
    for (volatile int i = 0; i < 500; ++i);
    spi_transfer(mem_map, tx, 2, ADS1247_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
    spi_receive(mem_map, rx, 1, ADS1247_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
    return rx[0];
}



/*
Resets ADS1247
*/
void ads1247_reset(mem_map_t mem_map){
    ads1247_send_command(mem_map, ADS1247_CMD_RESET);
    usleep(10000);
}

void ads1247_set_sys(mem_map_t mem_map, int pga_cfg, int dr_cfg){
    ads1247_write_reg(mem_map, ADS1247_REG_SYS0, pga_cfg << 4 | dr_cfg); 
}

/*
Initialize ADS1247
*/
void ads1247_init(mem_map_t mem_map){

    set_gpio_out_bit(mem_map, GPIO_ADC_RESET, 1);
    set_gpio_out_bit(mem_map, GPIO_ADC_START, 1);


    ads1247_reset(mem_map);

    
    
    ads1247_write_reg(mem_map, ADS1247_REG_MUX0, ADS1247_MUX0_CURR_SENSE); 
    ads1247_write_reg(mem_map, ADS1247_REG_VBIAS, ADS1247_VBIAS_DEFAULT);
    ads1247_write_reg(mem_map, ADS1247_REG_MUX1, ADS1247_MUX1_DEFAULT);
    ads1247_write_reg(mem_map, ADS1247_REG_SYS0, ADS1247_SYS0_DEFAULT);
    ads1247_write_reg(mem_map, ADS1247_REG_IDAC0, ADS1247_IDAC0_DEFAULT);

    ads1247_send_command(mem_map, ADS1247_CMD_SDATAC);   // Stop continuous read mode
}


/*
Initialize ADS1247
*/
int32_t ads1247_read_data(mem_map_t mem_map){
    uint8_t cmd = ADS1247_CMD_RDATA;
    uint8_t rx[3];
    for (volatile int i = 0; i < 500; ++i);
    spi_transfer(mem_map, &cmd, 1, ADS1247_SPI_SLAVE_NUM);
    for (volatile int i = 0; i < 500; ++i);
    spi_receive(mem_map, rx, 3, ADS1247_SPI_SLAVE_NUM);
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

char ads1247_data_ready(mem_map_t mem_map){

    return (read_gpio_in_bit(mem_map, GPIO2_ADC_DRDY) == 0);
}
