#include <stdio.h>
#include <stdlib.h>


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