#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include "sobel.h"

// DONE - write your pthread-ready sobel filter function here for a range of rows

// Note: You have access to all the global variables here.

void *sobel_worker(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int start_row = args->start_row;
    int end_row   = args->end_row;
 
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < width; j++) {
 
            // Set Border pixels to black
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
                output_image[i][j] = BLACK;
                continue;
            }
 
            // do Kx (horizontal) kernel
            int gx = 0;
            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    gx += Kx[ki + 1][kj + 1] * input_image[i + ki][j + kj];
                }
            }
 
            // do Ky (vertical) kernel
            int gy = 0;
            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    gy += Ky[ki + 1][kj + 1] * input_image[i + ki][j + kj];
                }
            }
 
            // gradient magnitude
            int g = (int)sqrt((double)(gx * gx + gy * gy));
 
            // Clamp to 255
            if (g > WHITE) {
                g = WHITE;
            }
 
            // Apply threshold
            if (g < threshold) {
                output_image[i][j] = BLACK;
            } else {
                output_image[i][j] = (unsigned char)g;
            }
        }
    }
 
    return NULL;
}
 