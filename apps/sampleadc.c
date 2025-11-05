#include "fpga.h"
#include "dac8562.h"
#include "addr.h"
#include "bias.h"

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


    mem_map_t mem_map;

    printf("Mapping FPGA devices\n");
    fpga_map_devices(&mem_map);

    
    // Prepare DMA mapped memory
    uint32_t phys_dma = 0;
    void * dma_map = map_dma_mem(DMA_SIZE, &phys_dma);
    memset(dma_map, 0, DMA_SIZE);


    // Start DMA
    printf("Starting DMA\n");
    dma_s2mm_start(mem_map, phys_dma, DMA_SIZE);

    // Start ADC Sampler
    printf("Starting ADC Sampler\n");
    sampler_set_count(mem_map, samples);
    sampler_start(mem_map);

    // Wait for DMA to finish
    printf("Waiting for DMA to finish\n");
    dma_s2mm_sync(mem_map);
    
    printf("Writing samples\n");
    write_samples("adc_samples.txt", dma_map, samples);


    fpga_unmap_devices(mem_map);

    return 0;
}
