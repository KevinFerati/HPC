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

const int16_t gauss_kernel[GAUSSIAN_KERNEL_SIZE*GAUSSIAN_KERNEL_SIZE] = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1,
};

struct img_1D_t *edge_detection_1D(const struct img_1D_t *input_img){
    struct img_1D_t *res_img;
    struct img_1D_t *grayed_gaussian;
    struct img_1D_t *grayed;

    grayed = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
    rgb_to_grayscale_1D(input_img, grayed);

    grayed_gaussian =  allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
    gaussian_filter_1D(grayed, grayed_gaussian, gauss_kernel);
    free_image(grayed);
    grayed = NULL;

    res_img = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
    sobel_filter_1D(grayed_gaussian, res_img, sobel_v_kernel, sobel_h_kernel);
    free_image(grayed_gaussian);
    grayed_gaussian = NULL;

    return res_img;
}

void rgb_to_grayscale_1D(const struct img_1D_t *img, struct img_1D_t *result){
    // copy each grayscaled pixel to the destination
    for (int current_pixel = 0, result_px = 0;
        result_px < img->width * img->height;
        current_pixel += img->components, ++result_px) {

            const uint8_t grayscaled_px =
                img->data[current_pixel + R_OFFSET] * FACTOR_R
              + img->data[current_pixel + G_OFFSET] * FACTOR_G
              + img->data[current_pixel + B_OFFSET] * FACTOR_B;

            result->data[result_px] = grayscaled_px;
    }
}

int apply_convolutional_kernel_1d(const struct img_1D_t *img, const int16_t *kernel, int curr_row, int curr_col) {
    int accumulation = 0;
    for (int img_row = curr_row - 1, kernel_idx = 0; img_row < curr_row + 2; img_row++) {
        for (int img_col = curr_col - 1; img_col < curr_col + 2; img_col++, kernel_idx++) {
            const int img_px = img_row * img->width + img_col;
            accumulation += kernel[kernel_idx] * img->data[img_px];
        }
    }
    return accumulation;
}

void gaussian_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const int16_t *kernel){
    const uint16_t gauss_ponderation = 16;

    for (int row = 0; row < img->height; ++row) {
        for (int col = 0; col < img->width; ++col) {
            const int current_px = row * img->width + col;
            // at edges, simply copy the source pixel
            if (row == 0 || row == img->height - 1 || col == 0 || col == img->width - 1) {
                res_img->data[current_px] = img->data[current_px];
                continue;
            }
            // apply the gaussian filter in the center of the image
            int accumulation = apply_convolutional_kernel_1d(img, kernel, row, col);
            accumulation /= gauss_ponderation;
            res_img->data[current_px] = accumulation;

        }
    }
}

void sobel_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const int16_t *v_kernel, const int16_t *h_kernel){
    for (int row = 0; row < img->height; ++row) {
        for (int col = 0; col < img->width; ++col) {
            const int current_px = row * img->width + col;
            // at edges, simply copy the source pixel
            if (row == 0 || row == img->height - 1 || col == 0 || col == img->width - 1) {
                res_img->data[current_px] = img->data[current_px];
                continue;
            }

            int h_value = abs(apply_convolutional_kernel_1d(img, h_kernel, row, col));
            int v_value = abs(apply_convolutional_kernel_1d(img, v_kernel, row, col));

            if (h_value + v_value >= SOBEL_BINARY_THRESHOLD) {
                res_img->data[current_px] = 0;
            } else {
                res_img->data[current_px] = UINT8_MAX;
            }
        }
    }
}


struct img_chained_t *edge_detection_chained(const struct img_chained_t *input_img){
    struct img_chained_t *res_img;

    struct img_chained_t *grayed = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);

    rgb_to_grayscale_chained(input_img, grayed);

    res_img = grayed;

    return res_img;
}

void rgb_to_grayscale_chained(const struct img_chained_t *img, struct img_chained_t *result){
    struct pixel_t *current_pixel = img->first_pixel;
    struct pixel_t *res_current_pixel = result->first_pixel;
    while (current_pixel != NULL) {

        *res_current_pixel->pixel_val =
            current_pixel->pixel_val[R_OFFSET] * FACTOR_R
          + current_pixel->pixel_val[G_OFFSET] * FACTOR_G
          + current_pixel->pixel_val[B_OFFSET] * FACTOR_B;


        res_current_pixel = res_current_pixel->next_pixel;
        current_pixel = current_pixel->next_pixel;
    }
}

void gaussian_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const uint16_t *kernel){
    const uint16_t gauss_ponderation = 16;

    //TODO
}

void sobel_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img,
                  const int16_t *v_kernel, const int16_t *h_kernel){

    //TODO
}