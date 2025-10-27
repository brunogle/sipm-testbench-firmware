#include "gpio.h"
#include "addr.h"
#include "fpga.h"



void set_gpio_bit(void *gpio_map, uint32_t bitmask, uint8_t val){
    if(val != 0){
        WR_REG_32(gpio_map, GPIO_DATA, RD_REG_32(gpio_map, GPIO_DATA) | bitmask);
    }
    else{
        WR_REG_32(gpio_map, GPIO_DATA, RD_REG_32(gpio_map, GPIO_DATA) & ~bitmask);
    }
}

char read_gpio_bit(void *gpio_map, uint32_t bitmask){
    if(RD_REG_32(gpio_map, GPIO2_DATA) & bitmask){
        return 1;
    }
    else{
        return 0;
    }
    
}