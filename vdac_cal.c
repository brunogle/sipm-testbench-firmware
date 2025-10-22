#include "vdac_cal.h"


int vdac_load_curve(const char *filename, vdac_cal_curve_t *curve) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open calibration file");
        return -1;
    }

    // First pass: count lines
    size_t count = 0;
    double x, y;
    while (fscanf(file, "%lf %lf", &x, &y) == 2)
        count++;

    if (count == 0) {
        fclose(file);
        fprintf(stderr, "Error: Calibration file is empty or invalid\n");
        return -1;
    }

    // Allocate memory
    curve->points = malloc(count * sizeof(vdac_cal_point_t));
    if (!curve->points) {
        fclose(file);
        perror("malloc failed");
        return -1;
    }
    curve->count = count;

    // Second pass: read values
    rewind(file);
    for (size_t i = 0; i < count; i++) {
        if (fscanf(file, "%lf %lf", &curve->points[i].x, &curve->points[i].y) != 2) {
            fprintf(stderr, "Error: Invalid line in calibration file\n");
            free(curve->points);
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}


int vdac_interpolate(const vdac_cal_curve_t *curve, double x, double *y_out) {
    if (curve->count < 2)
        return -1;

    // Check range
    if (x < curve->points[0].x || x > curve->points[curve->count - 1].x)
        return -1;

    // Find segment [x_i, x_{i+1}] where x lies
    for (size_t i = 0; i < curve->count - 1; i++) {
        double x1 = curve->points[i].x;
        double x2 = curve->points[i + 1].x;
        double y1 = curve->points[i].y;
        double y2 = curve->points[i + 1].y;

        if (x >= x1 && x <= x2) {
            double t = (x - x1) / (x2 - x1);
            *y_out = y1 + t * (y2 - y1);
            return 0;
        }
    }

    // Shouldn't reach here if data sorted properly
    return -1;
}

