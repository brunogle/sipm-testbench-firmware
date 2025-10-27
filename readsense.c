#include "ads1247.h"
#include "fpga.h"
#include "addr.h"
#include "gpio.h"

#include <string.h>
#include <unistd.h>   // for usleep

int32_t ads1247_read_data_poll(void *spi_map)
{
    uint8_t cmd = CMD_RDATA;
    uint8_t rx[3];

    // Wait enough time for conversion based on data rate
    usleep(220000); // 50 ms for 20 SPS, adjust as needed
    for (volatile int i = 0; i < 500; ++i);

    
    spi_transfer(spi_map, &cmd, 1, ADC_SPI_SLAVE_NUM);


    
    for (volatile int i = 0; i < 500; ++i);
    spi_receive(spi_map, rx, 3, ADC_SPI_SLAVE_NUM);



    
    for (volatile int i = 0; i < 500; ++i);
    int32_t result = ((int32_t)rx[0] << 16) |
                     ((int32_t)rx[1] << 8) |
                     ((int32_t)rx[2]);

    if (result & 0x800000)
        result |= 0xFF000000;

    return result;
}




int main(void)
{


    void * spi_map = map_device(SPI_ADDR, SPI_SIZE);
    void * gpio_map = map_device(GPIO0_ADDR, GPIO0_SIZE);

    spi_init(spi_map);
    
    /*ads1247_reset(spi_map);



    // Stop continuous read mode
    ads1247_send_command(spi_map, CMD_SDATAC);
    while(1){
        
        ads1247_write_reg(spi_map, REG_MUX0, 0x30);
        
        printf("x\n");
        usleep(500000);
    }*/
/*
    ads1247_set_nreset_pin(gpio_map, 1);
    ads1247_set_start_pin(gpio_map, 1);
*/
    usleep(200000);

    ads1247_init(spi_map, gpio_map);
    ads1247_set_sys(spi_map, ADS1247_PGA_1, ADS1247_DR_5);

    set_gpio_bit(gpio_map, GPIO_ADC_START, 1);

    ads1247_send_command(spi_map, CMD_SDATAC);   // Stop continuous read mode
    ads1247_send_command(spi_map, CMD_SLEEP);
    usleep(10000);

    //ads1247_send_command(spi_map, CMD_SYSOCAL);
    //usleep(4000000);

    while (1) {
        if (ads1247_data_ready(gpio_map)) {

            int32_t val = ads1247_read_data(spi_map);
            char sat;
            float v = ads1247_code_to_voltage(val, 1, &sat);
            printf("%x   ", val);
            printf("%f nA", -v*500.0);
            if(sat) printf(" (sat)");
            printf("\n");
            
            //ads1247_send_command(spi_map, CMD_SYNC);
        }
    }
    /*
    while (1) {
        
        int32_t val = ads1247_read_data_poll(spi_map);
        char sat;
        float v = ads1247_code_to_voltage(val, 128, &sat);
        printf("%x   ", val);
        printf("%f nA", -v*500.0);
        if(sat) printf(" (sat)");
        printf("\n");

        
    }
    */
}
