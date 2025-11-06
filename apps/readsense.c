#include "ads1247.h"
#include "fpga.h"
#include "addr.h"
#include "sense.h"
#include "bias.h"
#include <string.h>
#include <unistd.h>   // for usleep



int main(void){

    mem_map_t mem_map;

    fpga_map_devices(&mem_map);

    spi_init(mem_map);
    ads1247_init(mem_map);

    ads1247_set_sys(mem_map, ADS1247_PGA_1, ADS1247_DR_5);

    ads1247_write_reg(mem_map, ADS1247_REG_MUX0, ADS1247_MUX0_CURR_SENSE);
    set_gpio_out_bit(mem_map, GPIO_VMON_EN, 1);
    bias_enable(mem_map, 0);
    usleep(2000000);
    ads1247_send_command(mem_map, ADS1247_CMD_SYSOCAL);
    usleep(3500000);

    /*
    ads1247_write_reg(mem_map, ADS1247_REG_MUX1, ADS1247_MUX1_DEFAULT | 0x2);
    usleep(100000);
    ads1247_send_command(mem_map, ADS1247_CMD_SYSGCAL);
    usleep(3500000);
    ads1247_write_reg(mem_map, ADS1247_REG_MUX1, ADS1247_MUX1_DEFAULT);
    */

    bias_enable(mem_map, 1);


    char sat;
    while (1) {
        set_gpio_out_bit(mem_map, GPIO_VMON_EN, 0);
        usleep(500000);
        float isipm_na = sense_current_blocking(mem_map, &sat);
        //set_gpio_out_bit(mem_map, GPIO_VMON_EN, 1);
        //usleep(2000000);
        //float vsipm_v =  sense_voltage_blocking(mem_map);
        float vsipm_v = 0;
        printf("%.4f V - %.4f nA\n", vsipm_v, isipm_na);
    }

}
