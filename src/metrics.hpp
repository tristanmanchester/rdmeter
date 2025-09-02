#pragma once

#include <vector>
#include <cmath>
#include <limits>

namespace rdmeter {

// Calculate PSNR for the luma (Y) component between two frames
// Returns PSNR in dB, or a large value if frames are identical
double psnr_y(const std::vector<uint8_t>& ref_y, const std::vector<uint8_t>& dist_y, int width, int height);

// Generate 1D Gaussian kernel for SSIM calculation
std::vector<double> generate_gaussian_kernel(int size, double sigma);

// Apply 2D Gaussian filter to image for SSIM local statistics
std::vector<double> apply_gaussian_filter(const std::vector<uint8_t>& image, int width, int height, const std::vector<double>& kernel);

// Calculate single-scale SSIM for the luma (Y) component between two frames
// Returns SSIM value between 0 and 1, where 1 indicates perfect similarity
double ssim_y(const std::vector<uint8_t>& ref_y, const std::vector<uint8_t>& dist_y, int width, int height);

// Downsample image by factor of 2 using 2x2 average pooling
std::vector<uint8_t> downsample_2x2(const std::vector<uint8_t>& image, int width, int height, int& new_width, int& new_height);

// Calculate Multi-Scale SSIM for the luma (Y) component between two frames
// Returns MS-SSIM value between 0 and 1, where 1 indicates perfect similarity
double msssim_y(const std::vector<uint8_t>& ref_y, const std::vector<uint8_t>& dist_y, int width, int height);

} // namespace rdmeter