#include "fpga.h"
#include "dac.h"
#include "addr.h"
#include "vdac_cal.h"

#include <string.h>

void write_samples(const char *filename, void *dma_map, uint32_t sample_size) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open file");
        return;
    }

    for (size_t i = 0; i < sample_size; ++i) {
        if (fprintf(fp, "%d\n", (int16_t)RD_REG_16(dma_map, i<<1)) < 0) {
            perror("Failed to write sample");
            break;
        }
    }

    fclose(fp);
}


int main(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "Error: Arguments");
        return 1;
    }
    
    uint32_t samples = atoi(argv[1]);

    if(samples > DMA_SIZE/2){
        fprintf(stderr, "Error: Size exceeds DMA Memory Size (%d samples)\n", (DMA_SIZE/2));
        return 1;
    }


    void * dma_cfg_map = map_device(DMA_CFG_ADDR, DMA_CFG_SIZE);
    void * adc_sampler_map = map_device(ADC_SAMPLER_ADDR, ADC_SAMPLER_SIZE);

    // Prepare DMA mapped memory
    uint32_t phys_dma = 0;
    void * dma_map = map_dma_mem(DMA_SIZE, &phys_dma);
    memset(dma_map, 0, DMA_SIZE);


    // Start DMA
    dma_s2mm_start(dma_cfg_map, phys_dma, DMA_SIZE);

    // Start ADC Sampler
    WR_REG_32(adc_sampler_map, 0x4, samples);
    WR_REG_32(adc_sampler_map, 0x0, 0x1);

    // Wait for DMA to finish
    dma_s2mm_sync(dma_cfg_map);
    

    write_samples("adc_samples.txt", dma_map, samples);


    unmap_device(dma_cfg_map, DMA_CFG_SIZE);
    unmap_device(dma_map, DMA_SIZE);

    return 0;
}
