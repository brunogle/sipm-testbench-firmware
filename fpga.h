#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define WR_REG_32(address, offset, value)    (*((uint32_t *)((address) + (offset))) = (value))
#define RD_REG_32(address, offset)           (*((uint32_t *)((address) + (offset))))
#define WR_REG_16(address, offset, value)    (*((uint16_t *)((address) + (offset))) = (value))
#define RD_REG_16(address, offset)           (*((uint16_t *)((address) + (offset))))
#define WR_REG_8(address,  offset, value)    (*((uint8_t *)((address) + (offset))) = (value))
#define RD_REG_8(address,  offset)           (*((uint8_t *)((address) + (offset))))



#define SRR     0x40
#define SPICR   0x60
#define SPISR   0x64
#define SPIDTR  0x68
#define SPIDRR  0x6C
#define SPISSR  0x70

// SPISR bits
#define SPISR_TX_EMPTY (1 << 2)
#define SPISR_RX_EMPTY (1 << 0)

// Control bits
#define CR_LSB_FIRST    (1 << 9)
#define CR_MASTER_INH   (1 << 8)
#define CR_MANUAL_SS    (1 << 7)
#define CR_RXRST        (1 << 6)
#define CR_TXRST        (1 << 5)
#define CR_CPHA         (1 << 4)
#define CR_CPOL         (1 << 3)
#define CR_MASTER       (1 << 2)
#define CR_SPE          (1 << 1)
#define CR_LOOP         (1 << 0)


#define DMA_CFG_ADDR              0x40400000
#define DMA_CFG_SIZE              0x10000
#define DMA_CFG_MM2S_DMACR        0x00
#define DMA_CFG_MM2S_DMASR        0x04
#define DMA_CFG_MM2S_CURDESC      0x08
#define DMA_CFG_MM2S_CURDESC_MSB  0x0C
#define DMA_CFG_MM2S_TAILDESC     0x10
#define DMA_CFG_MM2S_TAILDESC_MSB 0x14
#define DMA_CFG_MM2S_SA           0x18
#define DMA_CFG_MM2S_SA_MSB       0x1C
#define DMA_CFG_MM2S_LENGTH       0x28
#define DMA_CFG_SG_CTL            0x2C
#define DMA_CFG_S2MM_DMACR        0x30
#define DMA_CFG_S2MM_DMASR        0x34
#define DMA_CFG_S2MM_CURDESC      0x38
#define DMA_CFG_S2MM_CURDESC_MSB  0x3C
#define DMA_CFG_S2MM_TAILDESC     0x40
#define DMA_CFG_S2MM_TAILDESC_MSB 0x44
#define DMA_CFG_S2MM_DA           0x48
#define DMA_CFG_S2MM_DA_MSB       0x4C
#define DMA_CFG_S2MM_LENGTH       0x58

#define DMA_SIZE 0x80000 //512KB (262144 16bit samples)

void * map_device(size_t base_addr, size_t size);
int unmap_device(void *map, size_t size);

void spi_init(void * spi_map);
void spi_transfer(void * spi_map, uint8_t *tx, int len, int slave);

void hv_out_set(void * gpio_map, uint8_t state);

void * map_dma_mem(uint32_t size, uint32_t * dma_phys_addr);
void dma_s2mm_status(void* dma_cfg_map);
int dma_s2mm_sync(void* dma_cfg_map);
void dma_s2mm_start(void * dma_cfg_map, uint32_t phys_addr, uint32_t max_length);

