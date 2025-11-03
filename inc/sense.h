#ifndef SENSE_H
#define SENSE_H


#include "fpga.h"

#define SCALE_CHANGE_STABLE_DELAY 100000 // Time in us to wait after scale change and performing new measurement
#define SCALE_LOWER_THRESHOLD 0x4000 // ADC Codes lower than this trigger a lowering of scale

#define LOW_SCALE_CURRENT_GAIN 500.0 // Current sense gain for low scale (nA/V)
#define HIGH_SCALE_CURRENT_GAIN 50000.0 // Current sense gain for high scale (nA/V)


float sense_current_blocking(mem_map_t mem_map, char * sat);
float sense_voltage_blocking(mem_map_t mem_map);


#endif