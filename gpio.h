
#include <stdint.h>



// --- Outputs
#define GPIO_HV_EN     0x0001
#define GPIO_DAC_CLR   0x0002
#define GPIO_SCALE     0x0004
#define GPIO_ADC_RESET 0x0008
#define GPIO_ADC_START 0x0010

// --- Inputs
#define GPIO2_ADC_DRDY 0x0001

// --- Registers
#define GPIO_DATA 0x00
#define GPIO2_DATA 0x08


void set_gpio_bit(void *gpio_map, uint32_t bitmask, uint8_t val);
char read_gpio_bit(void *gpio_map, uint32_t bitmask);