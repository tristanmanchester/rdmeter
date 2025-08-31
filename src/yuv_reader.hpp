#pragma once

#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>

namespace rdmeter {

// Structure to hold a YUV420p frame
struct YUVFrame {
    std::vector<uint8_t> y;  // Luma plane
    std::vector<uint8_t> u;  // Chroma U plane
    std::vector<uint8_t> v;  // Chroma V plane
    int width;
    int height;

    YUVFrame(int w, int h) : width(w), height(h) {
        y.resize(width * height);
        u.resize((width / 2) * (height / 2));
        v.resize((width / 2) * (height / 2));
    }
};

// Function to read a single YUV420p frame from a file stream
YUVFrame read_yuv420p_frame(std::ifstream& file, int width, int height) {
    YUVFrame frame(width, height);

    // Read Y plane
    file.read(reinterpret_cast<char*>(frame.y.data()), frame.y.size());
    if (!file) {
        throw std::runtime_error("Failed to read Y plane from YUV file");
    }

    // Read U plane
    file.read(reinterpret_cast<char*>(frame.u.data()), frame.u.size());
    if (!file) {
        throw std::runtime_error("Failed to read U plane from YUV file");
    }

    // Read V plane
    file.read(reinterpret_cast<char*>(frame.v.data()), frame.v.size());
    if (!file) {
        throw std::runtime_error("Failed to read V plane from YUV file");
    }

    return frame;
}

} // namespace rdmeter