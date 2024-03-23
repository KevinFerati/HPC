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
 * \brief Convert the img pixels (assuming a single component) to an array of uint8_t
 * \param img the source image to be converted
 * \param dest the destination one, must be preallocated
 */
void data_to_local(const struct img_chained_t *img, uint8_t *dest) {
    struct pixel_t* current_pixel = img->first_pixel;
    for (int pixel = img->height * img->width - 1; pixel >= 0; --pixel) {
        dest[pixel] = *current_pixel->pixel_val;
        current_pixel = current_pixel->next_pixel;
    }
}

/**
 * \brief  Apply a convolutional kernel on the image data (assuming a 1d array) and returns its sum
 * \param img_width Width of the original image
 * \param img_data Pixels of the original image
 * \param kernel Which kernel to apply
 * \param curr_row The current row in te main iteration (the y component of the pixel coords)
 * \param curr_col The current col in the main iteration (the x component of the pixel coords)
 */
int apply_convolutional_kernel(
    int img_width,
    uint8_t *img_data,
    const int16_t *kernel,
    int curr_row,
    int curr_col) {

    int accumulation = 0;
    for (int img_row = curr_row - 1, kernel_idx = 0; img_row < curr_row + 2; img_row++) {
        for (int img_col = curr_col - 1; img_col < curr_col + 2; img_col++, kernel_idx++) {
            const int img_px = img_row * img_width + img_col;
            accumulation += kernel[kernel_idx] * img_data[img_px];
        }
    }
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
            int accumulation = apply_convolutional_kernel(img->width, img->data, kernel, row, col);
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
            int h_value = abs(apply_convolutional_kernel(img->width, img->data, h_kernel, row, col));
            int v_value = abs(apply_convolutional_kernel(img->width, img->data, v_kernel, row, col));

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
    struct img_chained_t *grayed = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
    rgb_to_grayscale_chained(input_img, grayed);

    struct img_chained_t *grayed_gaussian  = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
    gaussian_filter_chained(grayed, grayed_gaussian, gauss_kernel);
    free_image_chained(grayed);
    grayed = NULL;

    struct img_chained_t *res_img = allocate_image_chained(input_img->width, input_img->height, COMPONENT_GRAYSCALE);
    sobel_filter_chained(grayed_gaussian, res_img, sobel_v_kernel, sobel_h_kernel);
    free_image_chained(grayed_gaussian);
    grayed_gaussian = NULL;

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

void gaussian_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img, const int16_t *kernel){
    const uint16_t gauss_ponderation = 16;
    uint8_t pixels[img->width * img->height];
    data_to_local(img, pixels);

    struct pixel_t *res_current_pixel = res_img->first_pixel;
    for (int row = img->height - 1; row >= 0; --row) {
        for (int col = img->width - 1; col >= 0; --col) {
            // copy the pixel in the borders
            if (row == 0 || row == img->height - 1 || col == 0 || col == img->width - 1) {
                *res_current_pixel->pixel_val = pixels[row * img->width + col];
            } else {  // apply the filter

                int accumulation = apply_convolutional_kernel(img->width, pixels, gauss_kernel, row, col);
                *res_current_pixel->pixel_val = accumulation / gauss_ponderation;
            }
            res_current_pixel = res_current_pixel->next_pixel;
        }
    }

}

void sobel_filter_chained(const struct img_chained_t *img, struct img_chained_t *res_img,
                  const int16_t *v_kernel, const int16_t *h_kernel){
    uint8_t pixels[img->width * img->height];
    data_to_local(img, pixels);
    struct pixel_t *res_current_pixel = res_img->first_pixel;
    for (int row = img->height - 1; row >= 0; --row) {
        for (int col = img->width - 1; col >= 0; --col) {
            // copy the pixel in the borders
            if (row == 0 || row == img->height - 1 || col == 0 || col == img->width - 1) {
                *res_current_pixel->pixel_val = pixels[row * img->width + col];
            } else {
                const int h_value = abs(apply_convolutional_kernel(img->width, pixels, h_kernel, row, col));
                const int v_value = abs(apply_convolutional_kernel(img->width, pixels, v_kernel, row, col));

                if (h_value + v_value >= SOBEL_BINARY_THRESHOLD) {
                    *res_current_pixel->pixel_val = BLACK;
                } else {
                    *res_current_pixel->pixel_val = WHITE;
                }
            }
            res_current_pixel = res_current_pixel->next_pixel;
        }
    }
}

