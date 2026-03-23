#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sobel.h"
#include "rtclock.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Sobel kernels
int Kx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

int Ky[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

// Globals: Image and threading data
unsigned char **input_image;
unsigned char **output_image;
unsigned char threshold = 127;
int width, height;
int num_threads;

/**
 * Main method
 */
int main(int argc, char *argv[]) {

    // DONE - Handle command line inputs
    if (argc < 4) {
        printf("Usage: ./sobel <input-file> <num-threads (>= 1)> <threshold (0-255)>\n");
        return -1;
    }

    char *filename = argv[1];
    num_threads = atoi(argv[2]);
    threshold   = (unsigned char)atoi(argv[3]);

    if (num_threads < 1) {
        printf("Usage: ./sobel <input-file> <num-threads (>= 1)> <threshold (0-255)>\n");
        return -1;
    }

    // DONE - Read image file into array a 1D array (see assignment write-up)
    unsigned char *data = stbi_load(filename, &width, &height, NULL, 1);
    if (data == NULL) {
        printf("Error: Could not load image '%s'\n", filename);
        return -1;
    }

    printf("Loaded %s. Height=%d, Width=%d\n", filename, height, width);


    // make 1D array to 2D input_image
    input_image = (unsigned char **)malloc(sizeof(unsigned char *) * height);
    for (int i = 0; i < height; i++) {
        input_image[i] = &data[i * width];
    }
 
    // Allocate 2D output_image
    output_image = (unsigned char **)malloc(sizeof(unsigned char *) * height);
    for (int i = 0; i < height; i++) {
        output_image[i] = (unsigned char *)malloc(sizeof(unsigned char) * width);
    }


    // Start clocking!
    double startTime, endTime;
    startTime = rtclock();

    // DONE - Prepare and create threads
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    ThreadArgs *args   = malloc(sizeof(ThreadArgs) * num_threads);

    // Divide rows evenly among threads
    int rows_per_thread = height / num_threads;
    int leftover = height % num_threads;

    int current_row = 0;
    for (int t = 0; t < num_threads; t++) {
        args[t].start_row = current_row;
        // Distribute leftover rows one-by-one
        args[t].end_row = current_row + rows_per_thread + (t < leftover ? 1 : 0);
        current_row = args[t].end_row;
 
        pthread_create(&threads[t], NULL, sobel_worker, &args[t]);
    }
 
    // DONE - Wait for threads to finish
    for (int t = 0; t < num_threads; t++) {
    pthread_join(threads[t], NULL);
    }
    
    // End clocking!
    endTime = rtclock();
    printf("Time taken (thread count = %d): %.6f sec\n", num_threads, (endTime - startTime));

    // DONE - Save the file!
 
    // insert -sobel before the extension
    char outfilename[512];
    char *dot = strrchr(filename, '.');
    if (dot != NULL) {
        int base_len = (int)(dot - filename);
        snprintf(outfilename, sizeof(outfilename), "%.*s-sobel%s", base_len, filename, dot);
    } else {
        snprintf(outfilename, sizeof(outfilename), "%s-sobel", filename);
    }
 
    // Remap 2D output_image back to a 1D
    unsigned char *array1D = malloc(width * height * sizeof(unsigned char));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            array1D[i * width + j] = output_image[i][j];
        }
    }
 
    stbi_write_jpg(outfilename, width, height, 1, array1D, 80);
    printf("Saved as %s\n", outfilename);

    // DONE - Free allocated memory

    free(array1D);
    array1D = NULL;

    // Free output_image rows, then pointer
    for (int i = 0; i < height; i++) {
        free(output_image[i]);
        output_image[i] = NULL;
    }
    free(output_image);
    output_image = NULL;

    // Free input_image pointer array
    free(input_image);
    input_image = NULL;

    // Free original stb data buffer
    stbi_image_free(data);
    data = NULL;

    free(threads);
    threads = NULL;
    free(args);
    args = NULL;
    return 0;
}
