#include "histogram.h"


void histogram_read(mem_map_t mem_map, uint32_t * histogram){
    for(int i = 0; i < HIST_BINS; i++){
        histogram[i] = RD_REG_32(mem_map.hist_map, 0x10000 + (i<<2));
    }
}


void histogram_reset(mem_map_t mem_map){
    WR_REG_32(mem_map.hist_map, 0x0, RD_REG_32(mem_map.hist_map, 0x0) | 0x2);

    while(RD_REG_32(mem_map.hist_map, 0x0) & 0x2){}
}

void histogram_enable(mem_map_t mem_map, char enable){
    if(enable)
        WR_REG_32(mem_map.hist_map, 0x0, RD_REG_32(mem_map.hist_map, 0x0) | 0x1);
    else
        WR_REG_32(mem_map.hist_map, 0x0, RD_REG_32(mem_map.hist_map, 0x0) & ~0x1);
}