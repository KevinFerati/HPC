#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "image.h"
#include "sobel.h"

#define GAUSSIAN_KERNEL_SIZE    3
#define SOBEL_KERNEL_SIZE       3
#define SOBEL_BINARY_THRESHOLD  150  // in the range 0 to uint8_max (255)

const int16_t sobel_v_kernel[SOBEL_KERNEL_SIZE*SOBEL_KERNEL_SIZE] = {
    -1, -2, -1,
     0,  0,  0,
     1,  2,  1,
};

const int16_t sobel_h_kernel[SOBEL_KERNEL_SIZE*SOBEL_KERNEL_SIZE] = {
    -1,  0,  1,
    -2,  0,  2,
    -1,  0,  1,
};

const uint16_t gauss_kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE] = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1,
};

struct img_1D_t *edge_detection_1D(const struct img_1D_t *input_img){
    struct img_1D_t *res_img;

    //TODO

    return res_img;
}

void rgb_to_grayscale_1D(const struct img_1D_t *img, struct img_1D_t *result){
    // init a new image
    const int count_pixels = img->height * img->width;
    result->data = calloc(count_pixels, sizeof(uint8_t));
    result->width = img->width;
    result->height = img->height;
    result->components = COMPONENT_GRAYSCALE;

    // copy each grayscaled pixel to the destination
    for (int current_pixel = 0, result_px = 0;
        result_px < count_pixels;
        current_pixel += img->components, ++result_px) {

            const uint8_t grayscaled_px =
                img->data[current_pixel + R_OFFSET] * FACTOR_R
              + img->data[current_pixel + G_OFFSET] * FACTOR_G
              + img->data[current_pixel + B_OFFSET] * FACTOR_B;

            result->data[result_px] = grayscaled_px;
    }
}

void gaussian_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const uint16_t *kernel){
    const uint16_t gauss_ponderation = 16;

    //TODO
}

void sobel_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const int16_t *v_kernel, const int16_t *h_kernel){
    //TODO
}


struct img_chained_t *edge_detection_chained(const struct img_chained_t *input_img){
    struct img_chained_t *res_img;
    //TODO

    return res_img;
}

void rgb_to_grayscale_chained(const struct img_chained_t *img, struct img_chained_t *result){

    //TODO

}

void gaussian_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const uint16_t *kernel){
    const uint16_t gauss_ponderation = 16;

    //TODO
}

void sobel_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img,
                  const int16_t *v_kernel, const int16_t *h_kernel){

    //TODO
}