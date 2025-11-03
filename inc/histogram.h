#include "fpga.h"

#define HIST_BINS 8192

void histogram_read(mem_map_t mem_map, uint32_t * histogram);
void histogram_reset(mem_map_t mem_map);
void histogram_enable(mem_map_t mem_map, char enable);