#ifndef BIAS_H
#define BIAS_H

#include <stdio.h>
#include <stdlib.h>
#include "fpga.h"

typedef struct {
    double x;
    double y;
} vdac_cal_point_t;

typedef struct {
    vdac_cal_point_t *points;
    size_t count;
} vdac_cal_curve_t;


int vdac_load_curve(const char *filename, vdac_cal_curve_t *curve);
int vdac_interpolate(const vdac_cal_curve_t *curve, double x, double *y_out);

int bias_set_vout(mem_map_t mem_map, float voltage, const vdac_cal_curve_t *curve);
int bias_set_dac_code(mem_map_t mem_map, int32_t dac_code);

void bias_enable(mem_map_t mem_map, char state);

#endif
