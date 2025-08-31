#include "third_party/CLI/CLI11.hpp"
#include "third_party/json.hpp"
#include "yuv_reader.hpp"
#include "metrics.hpp"
#include "threading.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <chrono>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    CLI::App app{"rdmeter: High-performance video codec analysis tool for computing RD metrics"};

    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "Enable verbose output");

    auto compute_cmd = app.add_subcommand("compute", "Compute RD metrics between reference and distorted videos");
    std::string ref_file;
    std::string dist_file;
    std::string output_file = "results/results.json";
    int width = 0;
    int height = 0;
    int max_frames = -1;  // all frames

    compute_cmd->add_option("-r,--ref", ref_file, "Path to reference YUV file")->required();
    compute_cmd->add_option("-d,--dist", dist_file, "Path to distorted YUV file")->required();
    compute_cmd->add_option("-o,--output", output_file, "Output JSON file path");
    compute_cmd->add_option("--width", width, "Video width in pixels")->required();
    compute_cmd->add_option("--height", height, "Video height in pixels")->required();
    compute_cmd->add_option("-f,--frames", max_frames, "Maximum number of frames to process (-1 for all)");

    auto bdrate_cmd = app.add_subcommand("bdrate", "Calculate BD-Rate from two CSV files");
    std::string ref_csv;
    std::string test_csv;
    std::string bdrate_output = "results/bdrate_results.json";

    bdrate_cmd->add_option("--ref-csv", ref_csv, "Path to reference CSV file")->required();
    bdrate_cmd->add_option("--test-csv", test_csv, "Path to test CSV file")->required();
    bdrate_cmd->add_option("-o,--output", bdrate_output, "Output JSON file path");

    CLI11_PARSE(app, argc, argv);

    try {
        if (*compute_cmd) {
            // Validate
            if (!fs::exists(ref_file)) {
                throw std::runtime_error("Reference file does not exist: " + ref_file);
            }
            if (!fs::exists(dist_file)) {
                throw std::runtime_error("Distorted file does not exist: " + dist_file);
            }
            if (width <= 0 || height <= 0) {
                throw std::runtime_error("Width and height must be positive");
            }

            // Open files
            std::ifstream ref_stream(ref_file, std::ios::binary);
            std::ifstream dist_stream(dist_file, std::ios::binary);
            if (!ref_stream) {
                throw std::runtime_error("Failed to open reference file: " + ref_file);
            }
            if (!dist_stream) {
                throw std::runtime_error("Failed to open distorted file: " + dist_file);
            }

            // for printing processing time
            auto start_time = std::chrono::high_resolution_clock::now();

            // Read the frames
            std::vector<rdmeter::YUVFrame> ref_frames;
            std::vector<rdmeter::YUVFrame> dist_frames;
            int frame_count = 0;

            while ((max_frames == -1 || frame_count < max_frames) &&
                   ref_stream && dist_stream) {
                try {
                    auto ref_frame = rdmeter::read_yuv420p_frame(ref_stream, width, height);
                    auto dist_frame = rdmeter::read_yuv420p_frame(dist_stream, width, height);
                    ref_frames.push_back(std::move(ref_frame));
                    dist_frames.push_back(std::move(dist_frame));
                    ++frame_count;
                } catch (const std::runtime_error&) {
                    break;
                }
            }

            // Compute metrics (psnr) for each frame
            double total_psnr = 0.0;
            int valid_frames = 0;
            for (int i = 0; i < frame_count; ++i) {
                try {
                    double psnr = rdmeter::psnr_y(ref_frames[i].y, dist_frames[i].y, width, height);
                    total_psnr += psnr;
                    ++valid_frames;
                } catch (const std::invalid_argument& e) {
                    if (verbose) {
                        std::cerr << "Skipping frame " << i << ": " << e.what() << std::endl;
                    }
                }
            }

            double avg_psnr = (valid_frames > 0) ? total_psnr / valid_frames : 0.0;

            // end timer and print results
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            std::cout << "Processed " << frame_count << " frames" << std::endl;
            std::cout << "Average PSNR (Y): " << avg_psnr << " dB" << std::endl;
            std::cout << "Processing time: " << duration.count() << " ms" << std::endl;
            if (verbose) {
                std::cout << "Results written to " << output_file << std::endl;
            }

            nlohmann::json results = {
                {"frame_count", frame_count},
                {"width", width},
                {"height", height},
                {"metrics", {
                    {"psnr_y", avg_psnr}
                }}
            };

            // Create output directory and write output
            fs::path output_path(output_file);
            if (output_path.has_parent_path()) {
                fs::create_directories(output_path.parent_path());
            }

            std::ofstream out_stream(output_file);
            if (!out_stream) {
                throw std::runtime_error("Failed to open output file: " + output_file);
            }
            out_stream << results.dump(4);
            if (verbose) {
                std::cout << "Results written to " << output_file << std::endl;
            }

        } else if (*bdrate_cmd) {
            // BD-Rate calculation (implement later)
            if (!fs::exists(ref_csv)) {
                throw std::runtime_error("Reference CSV does not exist: " + ref_csv);
            }
            if (!fs::exists(test_csv)) {
                throw std::runtime_error("Test CSV does not exist: " + test_csv);
            }

            nlohmann::json bdrate_results = {
                {"bd_rate", 0.0},  // Placeholder
                {"bd_psnr", 0.0}   // Placeholder
            };

            // Create output directory and write output
            fs::path bdrate_path(bdrate_output);
            if (bdrate_path.has_parent_path()) {
                fs::create_directories(bdrate_path.parent_path());
            }

            std::ofstream out_stream(bdrate_output);
            if (!out_stream) {
                throw std::runtime_error("Failed to open output file: " + bdrate_output);
            }
            out_stream << bdrate_results.dump(4);
            if (verbose) {
                std::cout << "BD-Rate results written to " << bdrate_output << std::endl;
            }

        } else {
            std::cout << app.help() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}