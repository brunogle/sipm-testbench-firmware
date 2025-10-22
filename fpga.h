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




void * map_device(size_t base_addr, size_t size);
int unmap_device(void *map, size_t size);

void spi_init(void * spi_map);
void spi_transfer(void * spi_map, uint8_t *tx, int len);

void hv_out_set(void * gpio_map, uint8_t state);
