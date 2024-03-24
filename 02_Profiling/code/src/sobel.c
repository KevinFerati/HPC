#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <likwid-marker.h>
#include "image.h"
#include "sobel.h"

#define GAUSSIAN_KERNEL_SIZE    3
#define SOBEL_KERNEL_SIZE       3
#define SOBEL_BINARY_THRESHOLD  150  // in the range 0 to uint8_max (255)
#define BLACK 0
#define WHITE 255



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

/**
 * \brief  Apply a convolutional kernel on the image data (assuming a 1d array) and returns its sum
 * \param img_width Width of the original image
 * \param img_data Pixels of the original image
 * \param kernel Which kernel to apply
 * \param curr_row The current row in te main iteration (the y component of the pixel coords)
 * \param curr_col The current col in the main iteration (the x component of the pixel coords)
 */
int sum_accumulation(
    int img_width,
    uint8_t *img_data,
    const int16_t *kernel,
    int current_px) {


    int accumulation = 0;

    const int last_row = current_px - img_width;
    const int next_row = current_px + img_width;

    accumulation += kernel[0] * img_data[last_row - 1];
    accumulation += kernel[1] * img_data[last_row];
    accumulation += kernel[2] * img_data[last_row + 1];

    accumulation += kernel[3] * img_data[current_px - 1];
    accumulation += kernel[4] * img_data[current_px];
    accumulation += kernel[5] * img_data[current_px + 1];

    accumulation += kernel[6] * img_data[next_row - 1];
    accumulation += kernel[7] * img_data[next_row];
    accumulation += kernel[8] * img_data[next_row + 1];

    return accumulation;
}


struct img_1D_t *edge_detection_1D(const struct img_1D_t *input_img){
    struct img_1D_t *res_img;
    struct img_1D_t *grayed_gaussian;
    struct img_1D_t *grayed;

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_REGISTER("greyscaling");
    LIKWID_MARKER_REGISTER("gauss");
    LIKWID_MARKER_REGISTER("sobel");
    //LIKWID_MARKER_REGISTER("branch");

    grayed = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);

    LIKWID_MARKER_START("greyscaling");
    rgb_to_grayscale_1D(input_img, grayed);
    LIKWID_MARKER_STOP("greyscaling");

    grayed_gaussian =  allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);

    LIKWID_MARKER_START("gauss");
    gaussian_filter_1D(grayed, grayed_gaussian, gauss_kernel);
    LIKWID_MARKER_STOP("gauss");

    free_image(grayed);
    grayed = NULL;

    res_img = allocate_image_1D(input_img->width, input_img->height, COMPONENT_GRAYSCALE);

    LIKWID_MARKER_START("sobel");
	sobel_filter_1D(grayed_gaussian, res_img, sobel_v_kernel, sobel_h_kernel);
    LIKWID_MARKER_STOP("sobel");

    free_image(grayed_gaussian);
    grayed_gaussian = NULL;
    LIKWID_MARKER_CLOSE;
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

void gaussian_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const int16_t *kernel){

	const uint16_t gauss_ponderation = 16;

    // copy the whole row
    for (int col = 0; col < img->width; ++col) {
        res_img->data[col] = img->data[col];
    }

    for (int row = 1; row < img->height - 1; ++row) {
        // copy the first and last columns
        const int row_begin = row * img->width;
        const int row_end = row_begin + img->width - 1;
        res_img->data[row_begin] = img->data[row_begin];
        for (int col = 1; col < img->width - 1; ++col) {
            const int current_px = row_begin + col;
            // apply the gaussian filter in the center of the image
            int accumulation = sum_accumulation(img->width, img->data, kernel, current_px);
            res_img->data[current_px] = accumulation / gauss_ponderation;
        }
        res_img->data[row_end] = img->data[row_end];
    }

    // copy the whole row
    const int last_row = img->width * (img->height - 1);
    for (int col = 0; col < img->width; ++col) {
        res_img->data[last_row + col] = img->data[last_row + col];
    }
}

void sobel_filter_1D(const struct img_1D_t *img, struct img_1D_t *res_img, const int16_t *v_kernel, const int16_t *h_kernel){
    // copy the whole row
    for (int col = 0; col < img->width; ++col) {
        res_img->data[col] = img->data[col];
    }

    for (int row = 1; row < img->height - 1; ++row) {
        // copy the first and last columns
        const int row_begin = row * img->width;
        res_img->data[row_begin] = img->data[row_begin];
        for (int col = 1; col < img->width - 1; ++col) {
            const int current_px = row_begin + col;
            int h_value_r = sum_accumulation(img->width, img->data, h_kernel, current_px);
            int v_value_r = sum_accumulation(img->width, img->data, v_kernel, current_px);
            int h_value = abs(h_value_r);
            int v_value = abs(v_value_r);
            if (h_value + v_value >= SOBEL_BINARY_THRESHOLD) {
                res_img->data[current_px] = BLACK;
            } else {
                res_img->data[current_px] = WHITE;
            }
        }
        const int row_end = row_begin + img->width - 1;
        res_img->data[row_end] = img->data[row_end];
    }
    // copy the whole row
    const int last_row = img->width * (img->height - 1);
    for (int col = 0; col < img->width; ++col) {
        res_img->data[last_row + col] = img->data[last_row + col];
    }
}


struct img_chained_t *edge_detection_chained(const struct img_chained_t *input_img){
}
