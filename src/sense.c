#include "sense.h"
#include "ads1247.h"
#include <unistd.h> 


float sense_current_blocking(mem_map_t mem_map, char * sat){

    ads1247_write_reg(mem_map, ADS1247_REG_MUX0, ADS1247_MUX0_CURR_SENSE); // Set ADC input to current channel

    ads1247_send_command(mem_map, ADS1247_CMD_SYNC); // Start conversion

    while(!ads1247_data_ready(mem_map)){} // Wait for DRDY signal

    char sat_;
    int32_t adc_code = ads1247_read_data(mem_map);
    float adc_v = ads1247_code_to_voltage(adc_code, 1, &sat_);

    char scale = get_gpio_out_bit(mem_map, GPIO_SCALE);


    if(sat_ == 1 && scale == 0){ // Reading is saturating in low scale
        set_gpio_out_bit(mem_map, GPIO_SCALE, 1); //Change to high scale
        usleep(100000); // Wait for signal to stabilize

        // Perform measurement again
        ads1247_send_command(mem_map, ADS1247_CMD_SYNC);
        while(!ads1247_data_ready(mem_map)){}
        adc_code = ads1247_read_data(mem_map);
        adc_v = ads1247_code_to_voltage(adc_code, 1, &sat_);
        scale = 1;
    }
    else if(abs(adc_code) < 0x4000 && scale == 1){ // Reading is in low end of the high scale
        set_gpio_out_bit(mem_map, GPIO_SCALE, 0); // Change to low scale
        usleep(100000); // Wait for signal to stabilize

        // Perform measurement again
        ads1247_send_command(mem_map, ADS1247_CMD_SYNC);
        while(!ads1247_data_ready(mem_map)){}
        adc_code = ads1247_read_data(mem_map);
        adc_v = ads1247_code_to_voltage(adc_code, 1, &sat_);
        scale = 0;
    }

    if(sat != NULL){
        *sat = sat_;
    }

    if(scale){
        return -adc_v*HIGH_SCALE_CURRENT_GAIN;
    }
    else{
        return -adc_v*LOW_SCALE_CURRENT_GAIN;
    }

}