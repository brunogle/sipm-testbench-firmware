#define _DEFAULT_SOURCE

#include "ads1247.h"
#include "fpga.h"
#include "addr.h"
#include "dac8562.h"
#include "bias.h"
#include "sense.h"
#include "histogram.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

//////// RUN CONFIGURATION ///////////

#define DATA_FOLDER "/root/testrun"

#define V_BIAS      66

#define V_SWEEP_MIN 63.0
#define V_SWEEP_MAX 69.0
#define V_SWEEP_DELTA 0.2
#define V_SWEEP_DELAY 2
#define V_SWEEP_PRE_DELAY 5

#define INTERVAL_TRACE      3*60
#define INTERVAL_HISTOGRAM  60
#define INTERVAL_IV_CURVE   5*60
#define INTERVAL_CURRENT    10

#define INTERVAL_SSH_COPY   (10*60)
#define SSH_FOLDER ""

//////// TESTBENCH CONFIGURATION ///////////



#define HIST_INTEGRATION_TIME 1
#define TRACE_SAMPLE_LENGTH 1250000 // 10ms


/*
RUNNAME
    trace
        trace-YYYY-MM-DD_HH-MM-SS.bin
        ...
    histogram
        histogram-YYYY-MM-DD_HH-MM-SS.bin
        ...
    iv
        iv-YYYY-MM-DD_HH-MM-SS.bin
        ...
    current
        current.csv
*/

enum state_t {
    IDLE,
    HIST,
    CURRENT,
    TRACE,
    IV_CURVE_PRE,
    IV_CURVE_DELAY,
    IV_CURVE_WAIT_ADC
};




void path_concat(char *dest, size_t size, const char *path1, const char *path2) {
    if (!path1 || !path2 || !dest || size == 0) return;

    // Remove trailing slash from path1 if present
    size_t len1 = strlen(path1);
    while (len1 > 0 && path1[len1 - 1] == '/')
        len1--;

    // Skip leading slashes from path2
    while (*path2 == '/')
        path2++;

    // Combine with a single '/'
    snprintf(dest, size, "%.*s/%s", (int)len1, path1, path2);
}

int write_trace(void * dma_map, char * dir){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char filename[100];
    strftime(filename, 34, "trace-%Y-%m-%d_%H-%M-%S.txt", t);
    char path[200];
    path_concat(path, 200, dir, filename);


    FILE *fp = fopen(path, "wb");
    if (!fp) {
        perror("Failed to open file");
        return -1;
    }

    for (size_t i = 0; i < TRACE_SAMPLE_LENGTH; ++i) {
        if (fprintf(fp, "%d\n", (int16_t)RD_REG_16(dma_map, i<<1)) < 0) {
            perror("Failed to write sample");
            break;
        }
    }

    fclose(fp);
    return 0;
}

int write_histogram(mem_map_t mem_map, char * dir){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    uint32_t hist[HIST_BINS];

    histogram_read(mem_map, hist);


    char filename[100];
    strftime(filename, 34, "histogram-%Y-%m-%d_%H-%M-%S.txt", t);
    char path[200];
    path_concat(path, 200, dir, filename);


    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("Failed to open file");
        return 0 ;
    }

    for (size_t i = 0; i < HIST_BINS; ++i) {
        if (fprintf(fp, "%d\n", hist[i]) < 0) {
            perror("Failed to write sample");
            break;
        }
    }
    fclose(fp);

    return 0;
}

int write_iv(float * voltage, float * current, int size, char * dir){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char filename[100];
    strftime(filename, 34, "iv-%Y-%m-%d_%H-%M-%S.txt", t);
    char path[200];
    path_concat(path, 200, dir, filename);

    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror("Failed to open file");
        return 0 ;
    }

    for (int i = 0; i < size; ++i) {
        if (fprintf(fp, "%f %f\n", voltage[i], current[i]) < 0) {
            perror("Failed to write sample");
            break;
        }
    }

    fclose(fp);
    return 0;
}

int write_current(char * dir, float value){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char path[200];
    path_concat(path, 200, dir, "current.txt");

    char time[30];
    
    strftime(time, 34, "%Y-%m-%d_%H-%M-%S", t);


    FILE *fp = fopen(path, "a");
    if (!fp) {
        perror("Failed to open file");
        return 0 ;
    }

    fprintf(fp, "%s, %f\n", time, value);


    fclose(fp);

    return 0;
}

uint32_t ms_time_diff(struct timespec t1, struct timespec t2){
    return (t1.tv_sec - t2.tv_sec) * 1000 + (t1.tv_nsec - t2.tv_nsec) / 1000000;
}

char dma_ready(mem_map_t mem_map){
    unsigned int s2mm_status = RD_REG_32(mem_map.dma_cfg_map, DMA_CFG_S2MM_DMASR);

    return !(!(s2mm_status & 1<<12) || !(s2mm_status & 1<<1));
}

int ensure_dir(char * path){
    struct stat st;

    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
        } else {
            fprintf(stderr, "Error: %s exists but is not a directory\n", path);
            exit(1);
        }
    }
    else{
        if (mkdir(path, 0755) != 0) {
            perror("mkdir failed");
            exit(1);
        }
    }

    return 0;
}

int main(){
    
    mem_map_t mem_map;
    vdac_cal_curve_t vdac_cal_curve;

    char trace_path[200];
    char histogram_path[200];
    char iv_path[200];

    printf("Starting SiPM: \"%s\"\n", DATA_FOLDER);
    printf("Initializing FPGA AXI Maps\n");

    if(vdac_load_curve("vsipm_dac_calibration.txt", &vdac_cal_curve) == -1){
        printf("Failed to load DAC calibration curve\n");
    }

    if(fpga_map_devices(&mem_map) != 1){
        printf("Error mapping FPGA memory. Exiting\n");
        exit(1);
    }

    ensure_dir(DATA_FOLDER);

    path_concat(trace_path, 200, DATA_FOLDER, "trace");
    path_concat(iv_path, 200, DATA_FOLDER, "iv");
    path_concat(histogram_path, 200, DATA_FOLDER, "histogram");

    ensure_dir(trace_path);
    ensure_dir(iv_path);
    ensure_dir(histogram_path);

    // Prepare DMA mapped memory
    uint32_t phys_dma = 0;
    void * dma_map = map_dma_mem(DMA_SIZE, &phys_dma);
    memset(dma_map, 0, DMA_SIZE);


    printf("Initializing Hardware\n");
    spi_init(mem_map);
    ads1247_init(mem_map);
    dac8562_initialize(mem_map);

    histogram_reset(mem_map);

    bias_set_vout(mem_map, 0, &vdac_cal_curve);
    bias_enable(mem_map, 0);
    
    sense_set_scale(mem_map, LOW_SCALE);
    sense_enable_vmon(mem_map, 0);
    sampler_set_count(mem_map, TRACE_SAMPLE_LENGTH);

    enum state_t state = IDLE;

    struct timespec last_hist;
    struct timespec last_curr;
    struct timespec last_trace;
    struct timespec last_iv;
    struct timespec iv_delay;
    struct timespec now;

    usleep(100000);
    bias_set_vout(mem_map, V_BIAS, &vdac_cal_curve);
    bias_enable(mem_map, 1);
    usleep(2000000);

    float voltage_sweep;

    float iv_current[200];
    float iv_voltage[200];
    int iv_count;

    while(1){
        clock_gettime(CLOCK_REALTIME, &now);


        switch(state){
            case IDLE:
                if(ms_time_diff(now, last_hist) > INTERVAL_HISTOGRAM*1000){
                    histogram_enable(mem_map, 1);
                    last_hist = now;
                    state = HIST;
                    printf("Histogram started... "); fflush(stdout);
                }
                else if(ms_time_diff(now, last_curr) > INTERVAL_CURRENT*1000){
                    sense_current_start(mem_map);
                    last_curr = now;
                    state = CURRENT;
                    printf("Current started... "); fflush(stdout);
                }
                else if(ms_time_diff(now, last_trace) > INTERVAL_TRACE*1000){
                    last_trace = now;
                    dma_s2mm_start(mem_map, phys_dma, DMA_SIZE);                    
                    sampler_start(mem_map);
                    printf("Trace started... "); fflush(stdout);
                    state = TRACE;
                }    
                else if(ms_time_diff(now, last_iv) > INTERVAL_IV_CURVE*1000){
                    last_iv = now;
                    state = IV_CURVE_PRE;
                    voltage_sweep = V_SWEEP_MIN;
                    bias_set_vout(mem_map, voltage_sweep, &vdac_cal_curve);
                    printf("IV Curve started, stabilizing... "); fflush(stdout);
                }            
            break;

            case HIST:
                if(ms_time_diff(now, last_hist) > HIST_INTEGRATION_TIME*1000){
                    printf("Saving... "); fflush(stdout);

                    histogram_enable(mem_map, 0);
                    write_histogram(mem_map, histogram_path);

                    histogram_reset(mem_map);
                    state = IDLE;
                    printf("OK \n"); fflush(stdout);
                }
            break;
            
            case CURRENT:
                float result;
                char sat;
                if(sense_current_get(mem_map, &result, &sat)){
                    //write_current(DATA_FOLDER, );
                    if(sat && !get_gpio_out_bit(mem_map, GPIO_SCALE)){
                        set_gpio_out_bit(mem_map, GPIO_SCALE, 1);
                        sense_current_start(mem_map);
                        printf("Changing scale LOW to HIGH... "); fflush(stdout);
                    }
                    else if(result < 1000 && get_gpio_out_bit(mem_map, GPIO_SCALE)){
                        set_gpio_out_bit(mem_map, GPIO_SCALE, 0);
                        sense_current_start(mem_map);
                        printf("Changing scale HIGH to LOW... "); fflush(stdout);
                    }
                    else{
                        printf("Current measured: %f nA. Saving... ", result); fflush(stdout);
                        write_current(DATA_FOLDER, result);
                        state = IDLE;
                        printf("OK\n"); fflush(stdout);
                    }

                }


            break;

            case TRACE:
                if(dma_ready(mem_map)){
                    printf("Trace acquired. Saving... ");
                    write_trace(dma_map, trace_path);
                    printf("OK \n");
                    state = IDLE;
                }
            break;

            case IV_CURVE_PRE:
                if(ms_time_diff(now, last_iv) > 1000*V_SWEEP_PRE_DELAY){
                    state = IV_CURVE_DELAY;
                    iv_delay = now;
                    printf("Starting sweep...\n"); 
                    iv_count = 0;
                }
            break;

            case IV_CURVE_DELAY:
                if(ms_time_diff(now, iv_delay) > 1000*V_SWEEP_DELAY){
                    printf("Starting ADC... "); fflush(stdout);
                    sense_current_start(mem_map);
                    state = IV_CURVE_WAIT_ADC;
                }
            break;

            case IV_CURVE_WAIT_ADC:
                float curr_result;
                char iv_sat;
                if(sense_current_get(mem_map, &curr_result, &iv_sat)){
                    if(iv_sat && !get_gpio_out_bit(mem_map, GPIO_SCALE)){
                        set_gpio_out_bit(mem_map, GPIO_SCALE, 1);
                        state = IV_CURVE_DELAY;
                        iv_delay = now;
                        printf("Changing scale LOW to HIGH... "); fflush(stdout);
                    }
                    else if(!iv_sat && get_gpio_out_bit(mem_map, GPIO_SCALE) && (curr_result < 1000)){
                        set_gpio_out_bit(mem_map, GPIO_SCALE, 0);
                        state = IV_CURVE_DELAY;
                        iv_delay = now;            
                        printf("Changing scale HIGH to LOW... "); fflush(stdout);            
                    }
                    else{
                        printf("ADC Finished %f V: %f nA\n", voltage_sweep, curr_result);
                        iv_current[iv_count] = curr_result;
                        iv_voltage[iv_count] = voltage_sweep;
                        iv_count++;
                        voltage_sweep += V_SWEEP_DELTA;
                        if(voltage_sweep > V_SWEEP_MAX){
                            state = IDLE;
                            bias_set_vout(mem_map, V_BIAS, &vdac_cal_curve);
                            printf("IV Finished. Saving... "); fflush(stdout);
                            write_iv(iv_voltage, iv_current, iv_count, iv_path);
                            printf("OK\n");
                            
                        }
                        else{
                            bias_set_vout(mem_map, voltage_sweep, &vdac_cal_curve);
                            state = IV_CURVE_DELAY;
                            iv_delay = now;
                        }
                    }
                }
            break;
            
            default:
            break;


        }
        //printf("%d\n", ms_time_diff(now, last_hist));
        usleep(1000);
        
    }



    return 0;
}