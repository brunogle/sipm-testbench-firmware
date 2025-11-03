#include "fpga.h"
#include "dac8562.h"
#include "addr.h"
#include "bias.h"
#include "histogram.h"

#include <string.h>
#include <unistd.h>   // for usleep



int main(){

    
    mem_map_t mem_map;

    fpga_map_devices(&mem_map);

    
    histogram_reset(mem_map);
    //usleep(100000);
    
    
    histogram_enable(mem_map, 1);
    usleep(1000000);
    histogram_enable(mem_map, 0);

    uint32_t hist[HIST_BINS];

    histogram_read(mem_map, hist);


    FILE *fp = fopen("hist.txt", "w");
    if (!fp) {
        perror("Failed to open file");
        return 0 ;
    }

    for (size_t i = 0; i < HIST_BINS; ++i) {
        if (fprintf(fp, "%d\n", hist[i]) < 0) {
            perror("Failed to write sample");
            break;
        }
    }

    fclose(fp);

    fpga_unmap_devices(mem_map);

    return 0;
}
