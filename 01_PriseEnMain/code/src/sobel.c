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

void copy_components(const struct img_1D_t *src,  struct img_1D_t *dest) {
    dest->width = src->width;
    dest->height = src->height;
    dest->components = src->components;
}

struct img_1D_t *setup_img_1d(const struct img_1D_t* input_img) {
    const int count_pixels = input_img->height * input_img->width;
    struct img_1D_t* res = malloc(sizeof(struct img_1D_t));
    res->data = calloc(count_pixels, sizeof(uint8_t));
    res->width = input_img->width;
    res->height = input_img->height;
    res->components = COMPONENT_GRAYSCALE;
    return res;
}

void free_all(struct img_1D_t* img) {
    free(img->data);
    img->data = NULL;
    free(img);
}

struct img_1D_t *edge_detection_1D(const struct img_1D_t *input_img){
    struct img_1D_t *res_img;
    struct img_1D_t *grayed_gaussian;
    struct img_1D_t *grayed;

    grayed = setup_img_1d(input_img);
    rgb_to_grayscale_1D(input_img, grayed);

    grayed_gaussian =  setup_img_1d(input_img);
    gaussian_filter_1D(grayed, grayed_gaussian, gauss_kernel);
    free_all(grayed);
    grayed = NULL;

    res_img = setup_img_1d(input_img);
    sobel_filter_1D(grayed_gaussian, res_img, sobel_v_kernel, sobel_h_kernel);
    free_all(grayed_gaussian);
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

void gaussian_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const uint16_t *kernel){
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
            int accumulation = 0;
            for (int img_row = row - 1, kernel_idx = 0; img_row < row + 2; img_row++) {
                for (int img_col = col - 1; img_col < col + 2; img_col++, kernel_idx++) {
                    const int img_px = img_row * img->width + img_col;
                    accumulation += kernel[kernel_idx] * img->data[img_px];
                }
            }
            accumulation /= gauss_ponderation;
            res_img->data[current_px]= accumulation;

        }
    }
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