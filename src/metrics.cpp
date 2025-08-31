#include "metrics.hpp"

namespace rdmeter {

double psnr_y(const std::vector<uint8_t>& ref_y, const std::vector<uint8_t>& dist_y, int width, int height) {
    if (ref_y.size() != dist_y.size() || ref_y.size() != static_cast<size_t>(width * height)) {
        throw std::invalid_argument("Frame sizes do not match or invalid dimensions");
    }

    double mse = 0.0;
    size_t total_pixels = ref_y.size();

    for (size_t i = 0; i < total_pixels; ++i) {
        double diff = static_cast<double>(ref_y[i]) - static_cast<double>(dist_y[i]);
        mse += diff * diff;
    }

    mse /= total_pixels;

    if (mse == 0.0) {
        // Frames are identical, return a high value
        return 100.0;
    }

    // PSNR = 20 * log10(MAX_VAL / sqrt(MSE))
    // For 8-bit YUV, MAX_VAL = 255
    double psnr = 20.0 * std::log10(255.0 / std::sqrt(mse));
    return psnr;
}

} // namespace rdmeter