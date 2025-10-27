#include "fpga.h"
#include "addr.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>

void * map_device(size_t base_addr, size_t size){
    int fd;
    char *name = "/dev/mem";
    void *map;

    if((fd = open(name, O_RDWR)) < 0) {
        perror("open");
        return NULL;
    }

    map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, base_addr);
    if(map == NULL){
        close(fd);
        perror("mmap");
        return NULL;
    }

    close(fd);

    return map;
}

int unmap_device(void *map, size_t size){
    if (munmap(map, size) == -1) {
        perror("munmap");
        return -1;
    }
    return 0;
}





static inline void spi_wait_tx_empty(mem_map_t mem_map) {
    while (!(RD_REG_32(mem_map.spi_map, SPISR) & SPISR_TX_EMPTY)){}
}

void spi_init(mem_map_t mem_map) {
    // Reset
    
    WR_REG_32(mem_map.spi_map, SRR, 0x0A);
    
    // Reset FIFOs
    WR_REG_32(mem_map.spi_map, SPICR, CR_MASTER_INH | CR_TXRST | CR_RXRST);

    // Configure mode: master, manual SS, CPOL=0, CPHA=1, enable
    uint32_t ctrl = CR_MASTER | CR_MANUAL_SS | CR_CPHA | CR_SPE;
    WR_REG_32(mem_map.spi_map, SPICR, ctrl | CR_MASTER_INH);  // start inhibited
    WR_REG_32(mem_map.spi_map, SPISSR, 0x3); // deselect all slaves
    
}

void spi_transfer(mem_map_t mem_map, uint8_t *tx, int len, int slave) {
    WR_REG_32(mem_map.spi_map, SPISSR, ~(1<<slave));
    WR_REG_32(mem_map.spi_map, SPICR, RD_REG_32(mem_map.spi_map, SPICR) & ~CR_MASTER_INH);
    
    for (int i = 0; i < len; ++i) {
        
        WR_REG_32(mem_map.spi_map, SPIDTR, tx[i]);    
            
        spi_wait_tx_empty(mem_map);
    }
    
    WR_REG_32(mem_map.spi_map, SPISSR, 0xF);
    WR_REG_32(mem_map.spi_map, SPICR, RD_REG_32(mem_map.spi_map, SPICR) | CR_MASTER_INH);
}


void spi_receive(mem_map_t mem_map, uint8_t *rx, int len, int slave)
{
    WR_REG_32(mem_map.spi_map, SPISSR, ~(1<<slave));
    WR_REG_32(mem_map.spi_map, SPICR, RD_REG_32(mem_map.spi_map, SPICR) & ~CR_MASTER_INH);

    for (int i = 0; i < len; ++i) {
        WR_REG_32(mem_map.spi_map, SPIDTR, 0xFF);
        for(int xx = 0; xx < 10000; xx++){} // TODO: Why does it need this delay? The while doesnt work i think
        while (RD_REG_32(mem_map.spi_map, SPISR) & SPISR_RX_EMPTY);
        rx[i] = (uint8_t)(RD_REG_32(mem_map.spi_map, SPIDRR) & 0xFF);
    }

    WR_REG_32(mem_map.spi_map, SPISSR, 0xF);
    WR_REG_32(mem_map.spi_map, SPICR, RD_REG_32(mem_map.spi_map, SPICR) | CR_MASTER_INH);

}


void hv_out_set(mem_map_t mem_map, uint8_t state){
    if(state != 0)
        WR_REG_32(mem_map.gpio_map, 0x0, RD_REG_32(mem_map.gpio_map, 0x0) | 0x1);
    else
        WR_REG_32(mem_map.gpio_map, 0x0, RD_REG_32(mem_map.gpio_map, 0x0) & ~0x1);
}


void * map_dma_mem(uint32_t size, uint32_t * dma_phys_addr){

    uint32_t dma_phys_addr_return;
    int fd;

    if((fd = open("/dev/cma", O_RDWR)) < 0){
        perror("open");
        return NULL;
    }
    
    dma_phys_addr_return = size;
    if(ioctl(fd, _IOWR('Z', 0, uint32_t), &dma_phys_addr_return) < 0){
        perror("ioctl");
        return NULL;
    }

    void * dma_addr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(dma_addr == NULL){
        close(fd);
        perror("mmap");
        return NULL;
    }

    close(fd);

    *dma_phys_addr = dma_phys_addr_return;

    return dma_addr;
}


int dma_s2mm_sync(mem_map_t mem_map) {
    unsigned int s2mm_status = RD_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_DMASR);

    while(!(s2mm_status & 1<<12) || !(s2mm_status & 1<<1)){
        s2mm_status = RD_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_DMASR);
    }

    return 0;
}

void dma_s2mm_start(mem_map_t mem_map, uint32_t phys_addr, uint32_t max_length){
    WR_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_DMACR, 0x4);
    WR_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_DMACR, 0x0);
    WR_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_DMACR, 0xf001);
    WR_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_DA, phys_addr);
    WR_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_LENGTH, max_length);
}



void set_gpio_out_bit(mem_map_t mem_map, uint32_t bitmask, uint8_t val){
    if(val != 0){
        WR_REG_32(mem_map.gpio_map, GPIO_DATA, RD_REG_32(mem_map.gpio_map, GPIO_DATA) | bitmask);
    }
    else{
        WR_REG_32(mem_map.gpio_map, GPIO_DATA, RD_REG_32(mem_map.gpio_map, GPIO_DATA) & ~bitmask);
    }
}

char get_gpio_out_bit(mem_map_t mem_map, uint32_t bitmask){
    if(RD_REG_32(mem_map.gpio_map, GPIO_DATA) & bitmask){
        return 1;
    }
    else{
        return 0;
    }
}

char read_gpio_in_bit(mem_map_t mem_map, uint32_t bitmask){
    if(RD_REG_32(mem_map.gpio_map, GPIO2_DATA) & bitmask){
        return 1;
    }
    else{
        return 0;
    }
}

void fpga_map_devices(mem_map_t * mem_map){
    mem_map->spi_map = map_device(SPI_ADDR, SPI_SIZE);
    mem_map->gpio_map = map_device(GPIO0_ADDR, GPIO0_SIZE);
    mem_map->dma_cfg_map = map_device(DMA_CFG_ADDR, DMA_CFG_SIZE);
    mem_map->adc_sampler_map = map_device(ADC_SAMPLER_ADDR, ADC_SAMPLER_SIZE);
}

void fpga_unmap_devices(mem_map_t mem_map){
    unmap_device(mem_map.spi_map, SPI_SIZE);
    unmap_device(mem_map.gpio_map, GPIO0_SIZE);
    unmap_device(mem_map.dma_cfg_map, DMA_CFG_SIZE);
    unmap_device(mem_map.adc_sampler_map, ADC_SAMPLER_SIZE);
}

int sampler_set_count(mem_map_t mem_map, uint32_t sample_count){
    if(sample_count >= DMA_SIZE<<1){
        return -1;
    }
    else{
        WR_REG_32(mem_map.adc_sampler_map, ADC_SAMPLER_COUNT, sample_count);
        return 0;
    }
}

void sampler_start(mem_map_t mem_map){
    WR_REG_32(mem_map.adc_sampler_map, ADC_SAMPLER_START, 1);
}