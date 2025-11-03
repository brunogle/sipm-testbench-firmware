#include "ads1247.h"
#include "fpga.h"
#include "addr.h"
#include "sense.h"

#include <string.h>
#include <unistd.h>   // for usleep



int main(void){

    mem_map_t mem_map;

    fpga_map_devices(&mem_map);

    spi_init(mem_map);
    ads1247_init(mem_map);

    ads1247_set_sys(mem_map, ADS1247_PGA_1, ADS1247_DR_5);

    char sat;
    while (1) {
        float isipm_na = sense_current_blocking(mem_map, &sat);
        float vsipm_v =  sense_voltage_blocking(mem_map);

        printf("%.2f V - %f nA\n", vsipm_v, isipm_na);
    }

}
