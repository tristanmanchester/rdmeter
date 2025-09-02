#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "src/metrics.hpp"
#include <vector>
#include <algorithm>

using namespace rdmeter;
using Catch::Approx;

TEST_CASE("Gaussian kernel generation", "[metrics]") {
    SECTION("Kernel size and normalization") {
        auto kernel = generate_gaussian_kernel(11, 1.5);
        
        REQUIRE(kernel.size() == 11);
        
        // Check normalization (sum should be approximately 1)
        double sum = 0.0;
        for (double val : kernel) {
            sum += val;
        }
        REQUIRE(sum == Approx(1.0).epsilon(1e-10));
        
        // Check symmetry
        REQUIRE(kernel[0] == Approx(kernel[10]).epsilon(1e-10));
        REQUIRE(kernel[1] == Approx(kernel[9]).epsilon(1e-10));
        REQUIRE(kernel[2] == Approx(kernel[8]).epsilon(1e-10));
    }
    
    SECTION("Center value is maximum") {
        auto kernel = generate_gaussian_kernel(11, 1.5);
        
        // Center value (index 5) should be maximum
        for (size_t i = 0; i < kernel.size(); ++i) {
            if (i != 5) {
                REQUIRE(kernel[5] >= kernel[i]);
            }
        }
    }
}

TEST_CASE("PSNR calculation", "[metrics]") {
    SECTION("Identical images") {
        std::vector<uint8_t> image(100 * 100, 128);  // All pixels = 128
        double psnr = psnr_y(image, image, 100, 100);
        REQUIRE(psnr == Approx(100.0));  // High value for identical images
    }
    
    SECTION("Different images") {
        std::vector<uint8_t> ref(100 * 100, 100);
        std::vector<uint8_t> dist(100 * 100, 150);  // Uniform difference of 50
        
        double psnr = psnr_y(ref, dist, 100, 100);
        
        // MSE = 50^2 = 2500
        // PSNR = 20 * log10(255 / sqrt(2500)) = 20 * log10(255/50) = 20 * log10(5.1) ≈ 14.15 dB
        REQUIRE(psnr == Approx(14.15).epsilon(0.01));
    }
    
    SECTION("Size mismatch throws exception") {
        std::vector<uint8_t> ref(100, 128);
        std::vector<uint8_t> dist(50, 128);
        
        REQUIRE_THROWS_AS(psnr_y(ref, dist, 10, 10), std::invalid_argument);
    }
}

TEST_CASE("Image downsampling", "[metrics]") {
    SECTION("2x2 downsampling preserves averages") {
        // Create a simple 4x4 image with known values
        std::vector<uint8_t> image = {
            100, 100, 200, 200,  // Row 0
            100, 100, 200, 200,  // Row 1  
            50,  50,  150, 150,  // Row 2
            50,  50,  150, 150   // Row 3
        };
        
        int new_width, new_height;
        auto downsampled = downsample_2x2(image, 4, 4, new_width, new_height);
        
        REQUIRE(new_width == 2);
        REQUIRE(new_height == 2);
        REQUIRE(downsampled.size() == 4);
        
        // Check averaged values
        REQUIRE(downsampled[0] == 100);  // Average of (100,100,100,100)
        REQUIRE(downsampled[1] == 200);  // Average of (200,200,200,200)
        REQUIRE(downsampled[2] == 50);   // Average of (50,50,50,50)
        REQUIRE(downsampled[3] == 150);  // Average of (150,150,150,150)
    }
    
    SECTION("Too small image throws exception") {
        std::vector<uint8_t> image(1, 128);
        int new_width, new_height;
        
        REQUIRE_THROWS_AS(downsample_2x2(image, 1, 1, new_width, new_height), std::invalid_argument);
    }
}

TEST_CASE("SSIM calculation", "[metrics]") {
    SECTION("Identical images return 1.0") {
        std::vector<uint8_t> image(64 * 64, 128);  // Uniform image
        double ssim = ssim_y(image, image, 64, 64);
        REQUIRE(ssim == Approx(1.0).epsilon(1e-6));
    }
    
    SECTION("Completely different images return low SSIM") {
        std::vector<uint8_t> ref(64 * 64, 0);     // Black image
        std::vector<uint8_t> dist(64 * 64, 255);  // White image
        
        double ssim = ssim_y(ref, dist, 64, 64);
        REQUIRE(ssim < 0.1);  // Should be very low
    }
    
    SECTION("Similar images return high SSIM") {
        std::vector<uint8_t> ref(64 * 64, 128);
        std::vector<uint8_t> dist(64 * 64, 130);  // Slight difference
        
        double ssim = ssim_y(ref, dist, 64, 64);
        REQUIRE(ssim > 0.8);  // Should be high
    }
}

TEST_CASE("MS-SSIM calculation", "[metrics]") {
    SECTION("Identical images return 1.0") {
        // Use minimum size for 5 scales (32x32)
        std::vector<uint8_t> image(64 * 64, 128);
        double msssim = msssim_y(image, image, 64, 64);
        REQUIRE(msssim == Approx(1.0).epsilon(1e-6));
    }
    
    SECTION("Image too small throws exception") {
        std::vector<uint8_t> image(16 * 16, 128);
        
        REQUIRE_THROWS_AS(msssim_y(image, image, 16, 16), std::invalid_argument);
    }
    
    SECTION("Different images return reasonable MS-SSIM") {
        // Create a gradient image for testing
        std::vector<uint8_t> ref(64 * 64);
        std::vector<uint8_t> dist(64 * 64);
        
        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 64; ++x) {
                ref[y * 64 + x] = static_cast<uint8_t>((x + y) % 256);
                dist[y * 64 + x] = static_cast<uint8_t>(((x + y) % 256 + 10) % 256);  // Slight shift
            }
        }
        
        double msssim = msssim_y(ref, dist, 64, 64);
        REQUIRE(msssim > 0.5);  // Should be reasonable for similar patterns
        REQUIRE(msssim < 1.0);  // But not perfect
    }
    
    SECTION("MS-SSIM is in valid range [0,1]") {
        // Create random-like pattern
        std::vector<uint8_t> ref(64 * 64);
        std::vector<uint8_t> dist(64 * 64);
        
        for (int i = 0; i < 64 * 64; ++i) {
            ref[i] = static_cast<uint8_t>(i % 256);
            dist[i] = static_cast<uint8_t>((i * 7) % 256);  // Different pattern
        }
        
        double msssim = msssim_y(ref, dist, 64, 64);
        REQUIRE(msssim >= 0.0);
        REQUIRE(msssim <= 1.0);
    }
}

TEST_CASE("Gaussian filter application", "[metrics]") {
    SECTION("Filtering preserves image dimensions") {
        std::vector<uint8_t> image(32 * 32, 128);
        auto kernel = generate_gaussian_kernel(11, 1.5);
        
        auto filtered = apply_gaussian_filter(image, 32, 32, kernel);
        REQUIRE(filtered.size() == image.size());
    }
    
    SECTION("Filtering uniform image preserves value") {
        std::vector<uint8_t> image(32 * 32, 100);  // Uniform value
        auto kernel = generate_gaussian_kernel(11, 1.5);
        
        auto filtered = apply_gaussian_filter(image, 32, 32, kernel);
        
        // All values should be approximately 100
        for (double val : filtered) {
            REQUIRE(val == Approx(100.0).epsilon(1e-6));
        }
    }
}