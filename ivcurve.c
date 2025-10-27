#include "ads1247.h"
#include "fpga.h"
#include "addr.h"
#include "gpio.h"
#include "dac.h"
#include "vdac_cal.h"

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
    vdac_cal_curve_t vdac_cal_curve;
   
    vdac_load_curve("vsipm_dac_calibration.txt", &vdac_cal_curve);

    void * spi_map = map_device(SPI_ADDR, SPI_SIZE);
    void * gpio_map = map_device(GPIO0_ADDR, GPIO0_SIZE);

    spi_init(spi_map);
    usleep(100000);
    ads1247_init(spi_map, gpio_map);
    usleep(200000);
    dac_initialize(spi_map);

    
    ads1247_set_sys(spi_map, ADS1247_PGA_1, ADS1247_DR_5);

    set_gpio_bit(gpio_map, GPIO_ADC_START, 1);

    ads1247_send_command(spi_map, CMD_SDATAC);   // Stop continuous read mode
    //ads1247_send_command(spi_map, CMD_SLEEP);
    usleep(10000);
        double dac_code;

    vdac_interpolate(&vdac_cal_curve, 60, &dac_code);
    dac_set_a(spi_map, dac_code);
    hv_out_set(gpio_map, 1);
    set_gpio_bit(gpio_map, GPIO_SCALE, 0);
    usleep(3000000);

    FILE *fp = fopen("ivcurve.txt", "w");

    char scale = 0;
    
    for(float v = 60; v < 69.5; v+=0.02){

        vdac_interpolate(&vdac_cal_curve, v, &dac_code);
        dac_set_a(spi_map, dac_code);
        usleep(500000);

        ads1247_send_command(spi_map, CMD_SYNC);
        while(!ads1247_data_ready(gpio_map)){}

        int32_t adc_code = ads1247_read_data(spi_map);
        char sat;
        float adc_v = ads1247_code_to_voltage(adc_code, 1, &sat);
        if(sat == 1 && scale == 0){
            set_gpio_bit(gpio_map, GPIO_SCALE, 1);
            usleep(100000);
            ads1247_send_command(spi_map, CMD_SYNC);
            while(!ads1247_data_ready(gpio_map)){}
            adc_code = ads1247_read_data(spi_map);
            adc_v = ads1247_code_to_voltage(adc_code, 1, &sat);
            scale = 1;
            //usleep(100000);
        }
        if(abs(adc_code) < 0x4000 && scale == 1){
            set_gpio_bit(gpio_map, GPIO_SCALE, 0);
            usleep(100000);
            ads1247_send_command(spi_map, CMD_SYNC);
            while(!ads1247_data_ready(gpio_map)){}
            adc_code = ads1247_read_data(spi_map);
            adc_v = ads1247_code_to_voltage(adc_code, 1, &sat);
            scale = 0;
            //usleep(100000);
        }


        float isense_na = -adc_v*500.0;
        if(scale) isense_na *= 100;
        printf("%f : ", v);
        printf("%f nA", isense_na);
        printf("  %.2f%%", 100.0*(v-60.0)/(69.5-60.0));
        if(sat) printf(" (sat)");
        printf("\n");

        fprintf(fp, "%f %f\n", v, isense_na);

        
    }

    dac_set_a(spi_map, 0);
    hv_out_set(gpio_map, 0);

    fclose(fp);
    
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



}
