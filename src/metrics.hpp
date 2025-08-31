#pragma once

#include <vector>
#include <cmath>
#include <limits>

namespace rdmeter {

// Calculate PSNR for the luma (Y) component between two frames
// Returns PSNR in dB, or a large value if frames are identical
double psnr_y(const std::vector<uint8_t>& ref_y, const std::vector<uint8_t>& dist_y, int width, int height);

} // namespace rdmeter