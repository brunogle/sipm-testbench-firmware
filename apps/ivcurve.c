#include "ads1247.h"
#include "fpga.h"
#include "addr.h"
#include "dac8562.h"
#include "bias.h"
#include "sense.h"

#include <string.h>
#include <unistd.h>   // for usleep




int main(void)
{
    vdac_cal_curve_t vdac_cal_curve;
   
    vdac_load_curve("vsipm_dac_calibration.txt", &vdac_cal_curve);

    mem_map_t mem_map;

    fpga_map_devices(&mem_map);

    spi_init(mem_map);
    ads1247_init(mem_map);
    dac8562_initialize(mem_map);

    
    ads1247_set_sys(mem_map, ADS1247_PGA_1, ADS1247_DR_5);
    
    bias_set_vout(mem_map, 60, &vdac_cal_curve);
    bias_enable(mem_map, 1);

    usleep(3000000);

    FILE *fp = fopen("ivcurve.txt", "w");
    
    for(float v = 65; v < 69.5; v+=0.02){

        bias_set_vout(mem_map, v, &vdac_cal_curve);
        usleep(500000);

        
        char sat;
        float isense_na = sense_current_blocking(mem_map, &sat);

        printf("%f : ", v);
        printf("%f nA", isense_na);
        printf("  %.2f%%", 100.0*(v-65.0)/(69.5-65.0));
        if(sat) printf(" (sat)");
        printf("\n");

        fprintf(fp, "%f %f\n", v, isense_na);

        
    }

    bias_set_dac_code(mem_map, 0);
    bias_enable(mem_map, 0);

    fclose(fp);

}
