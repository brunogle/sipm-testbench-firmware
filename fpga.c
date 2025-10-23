#include "fpga.h"

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





static inline void spi_wait_tx_empty(void * spi_map) {
    while (!(RD_REG_32(spi_map, SPISR) & SPISR_TX_EMPTY)){}
}

void spi_init(void * spi_map) {
    // Reset
    
    WR_REG_32(spi_map, SRR, 0x0A);
    
    // Reset FIFOs
    WR_REG_32(spi_map, SPICR, CR_MASTER_INH | CR_TXRST | CR_RXRST);

    // Configure mode: master, manual SS, CPOL=0, CPHA=1, enable
    uint32_t ctrl = CR_MASTER | CR_MANUAL_SS | CR_CPHA | CR_SPE;
    WR_REG_32(spi_map, SPICR, ctrl | CR_MASTER_INH);  // start inhibited
    WR_REG_32(spi_map, SPISSR, 0x1); // deselect all slaves
    
}

void spi_transfer(void * spi_map, uint8_t *tx, int len, int slave) {
    
    WR_REG_32(spi_map, SPISSR, ~(1<<slave)); // select slave
    
    WR_REG_32(spi_map, SPICR, RD_REG_32(spi_map, SPICR) & ~CR_MASTER_INH); // enable
    
    for (int i = 0; i < len; ++i) {
        WR_REG_32(spi_map, SPIDTR, tx[i]);        
        spi_wait_tx_empty(spi_map);
    }
    
    WR_REG_32(spi_map, SPISSR, 0xF); // deselect
    WR_REG_32(spi_map, SPICR, RD_REG_32(spi_map, SPICR) | CR_MASTER_INH); // disable
}


void hv_out_set(void * gpio_map, uint8_t state){
    if(state != 0)
        WR_REG_32(gpio_map, 0x0, RD_REG_32(gpio_map, 0x0) | 0x1);
    else
        WR_REG_32(gpio_map, 0x0, RD_REG_32(gpio_map, 0x0) & ~0x1);
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


void dma_s2mm_status(void* dma_cfg_map) {
    unsigned int status =  RD_REG_32(dma_cfg_map, DMA_CFG_S2MM_DMASR);
    printf("Stream to memory-mapped status (0x%08x@0x%02x):", status, DMA_CFG_S2MM_DMASR);
    if (status & 0x00000001) printf(" halted"); else printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");
}

int dma_s2mm_sync(void* dma_cfg_map) {
    unsigned int s2mm_status = RD_REG_32(dma_cfg_map, DMA_CFG_S2MM_DMASR);

    while(!(s2mm_status & 1<<12) || !(s2mm_status & 1<<1)){
        s2mm_status = RD_REG_32(dma_cfg_map, DMA_CFG_S2MM_DMASR);
    }

    return 0;
}

void dma_s2mm_start(void * dma_cfg_map, uint32_t phys_addr, uint32_t max_length){
    WR_REG_32(dma_cfg_map, DMA_CFG_S2MM_DMACR, 0x4);
    WR_REG_32(dma_cfg_map, DMA_CFG_S2MM_DMACR, 0x0);
    WR_REG_32(dma_cfg_map, DMA_CFG_S2MM_DMACR, 0xf001);
    WR_REG_32(dma_cfg_map, DMA_CFG_S2MM_DA, phys_addr);
    WR_REG_32(dma_cfg_map, DMA_CFG_S2MM_LENGTH, max_length);
}
