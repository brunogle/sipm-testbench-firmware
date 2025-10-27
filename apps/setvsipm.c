#include "fpga.h"
#include "dac8562.h"
#include "addr.h"
#include "bias.h"

#include <string.h>


int main(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "Error: Arguments");
        return 1;
    }


    mem_map_t mem_map;

    fpga_map_devices(&mem_map);

    spi_init(mem_map);
    dac8562_initialize(mem_map);

    if(strcmp(argv[1], "0") == 0){
        bias_set_dac_code(mem_map, 0);
        hv_out_set(mem_map, 0);
        return 0;
    }

    float voltage = atof(argv[1]);

    vdac_cal_curve_t vdac_cal_curve;
   
    vdac_load_curve("vsipm_dac_calibration.txt", &vdac_cal_curve);

    if(bias_set_vout(mem_map, voltage, &vdac_cal_curve) == -1){
        printf("Voltage out of range\n");
        return -1;
    }

    hv_out_set(mem_map, 1);

    fpga_unmap_devices(mem_map);

    return 0;
}
