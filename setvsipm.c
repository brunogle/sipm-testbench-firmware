#include "fpga.h"
#include "dac.h"
#include "addr.h"
#include "vdac_cal.h"

#include <string.h>


int main(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "Error: Arguments");
        return 1;
    }


    void * spi_map = map_device(SPI_ADDR, SPI_SIZE);
    void * gpio_map = map_device(GPIO0_ADDR, GPIO0_SIZE);

    dac_initialize(spi_map);

    if(strcmp(argv[1], "0") == 0){
        dac_set_a(spi_map, 0);
        hv_out_set(gpio_map, 0);
        return 0;
    }

    float voltage = atof(argv[1]);

    vdac_cal_curve_t vdac_cal_curve;
   
    vdac_load_curve("vsipm_dac_calibration.txt", &vdac_cal_curve);

    

    double code;
    if(vdac_interpolate(&vdac_cal_curve, voltage, &code) == -1){
        printf("Voltage out of range\n");
        return -1;
    }

    dac_set_a(spi_map, (uint16_t)code);
    hv_out_set(gpio_map, 1);

    unmap_device(spi_map, SPI_SIZE);
    unmap_device(gpio_map, GPIO0_SIZE);

    return 0;
}
