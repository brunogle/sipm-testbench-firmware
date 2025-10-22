#include "fpga.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

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

void spi_transfer(void * spi_map, uint8_t *tx, int len) {
    
    WR_REG_32(spi_map, SPISSR, 0x0); // select slave
    
    WR_REG_32(spi_map, SPICR, RD_REG_32(spi_map, SPICR) & ~CR_MASTER_INH); // enable
    
    for (int i = 0; i < len; ++i) {
        WR_REG_32(spi_map, SPIDTR, tx[i]);        
        spi_wait_tx_empty(spi_map);
    }
    
    WR_REG_32(spi_map, SPISSR, 0x1); // deselect
    WR_REG_32(spi_map, SPICR, RD_REG_32(spi_map, SPICR) | CR_MASTER_INH); // disable
}


void hv_out_set(void * gpio_map, uint8_t state){
    if(state != 0)
        WR_REG_32(gpio_map, 0x0, RD_REG_32(gpio_map, 0x0) | 0x1);
    else
        WR_REG_32(gpio_map, 0x0, RD_REG_32(gpio_map, 0x0) & ~0x1);
}
