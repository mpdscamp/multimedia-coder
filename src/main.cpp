#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstdio>
#include <vector>
#include "arithmetic_coder.hpp"
#include "utils.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "  " << argv[0] << " encode <input_file> <output.codestream>" << std::endl;
        std::cerr << "  " << argv[0] << " decode <input.codestream> <output_file>" << std::endl;
        std::cerr << "  " << argv[0] << " encode_all" << std::endl;
        std::cerr << "  " << argv[0] << " decode_all" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    auto global_start_time = std::chrono::high_resolution_clock::now();

    if (mode == "encode") {
        if (argc < 4) {
            std::cerr << "Usage for encoding: " << argv[0] << " encode <input_file> <output.codestream>" << std::endl;
            return 1;
        }
        std::string input_file = argv[2];
        std::string output_file = argv[3];

        std::cout << "Encoding " << input_file << " to " << output_file << "..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();

        ArithmeticEncoder encoder;
        if (!encoder.encode(input_file, output_file)) {
            std::cerr << "Failed to encode file." << std::endl;
            std::remove(output_file.c_str());
            return 1;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        std::streamsize orig_size = getFileSize(input_file);
        std::streamsize comp_size = getFileSize(output_file);
        double ratio = calculateCompressionRatio(orig_size, comp_size);

        std::cout << "Successfully encoded file." << std::endl;
        if (orig_size >= 0) std::cout << "Original size:     " << orig_size << " bytes" << std::endl;
        if (comp_size >= 0) std::cout << "Compressed size:   " << comp_size << " bytes" << std::endl;
        if (ratio > 0.0) std::cout << "Compression ratio: " << std::fixed << std::setprecision(2) << ratio << ":1" << std::endl;
        std::cout << "Encoding time:     " << std::fixed << std::setprecision(3) << duration.count() << " seconds" << std::endl;

    } else if (mode == "decode") {
        if (argc < 4) {
            std::cerr << "Usage for decoding: " << argv[0] << " decode <input.codestream> <output_file>" << std::endl;
            return 1;
        }
        std::string input_file = argv[2];
        std::string output_file = argv[3];

        std::cout << "Decoding " << input_file << " to " << output_file << "..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();

        ArithmeticDecoder decoder;
        if (!decoder.decode(input_file, output_file)) {
            std::cerr << "Failed to decode file." << std::endl;
            std::remove(output_file.c_str());
            return 1;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        std::cout << "Successfully decoded file." << std::endl;
        std::cout << "Output saved to:   " << output_file << std::endl;
        std::streamsize decoded_size = getFileSize(output_file);
        if (decoded_size >= 0) std::cout << "Decoded size:      " << decoded_size << " bytes" << std::endl;
        std::cout << "Decoding time:     " << std::fixed << std::setprecision(3) << duration.count() << " seconds" << std::endl;

    } else if (mode == "encode_all") {
        const std::vector<std::pair<std::string, std::string>> files = {
            {"input/lena_ascii.pgm", "lena_ascii.codestream"},
            {"input/baboon_ascii.pgm", "baboon_ascii.codestream"},
            {"input/quadrado_ascii.pgm", "quadrado_ascii.codestream"}
        };
        
        std::cout << "Encoding All Specified Files..." << std::endl;
        std::cout << "-----------------------------------------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(25) << "Input File"
                  << std::right << std::setw(15) << "Original Size"
                  << std::setw(15) << "Comp. Size"
                  << std::setw(12) << "Ratio"
                  << std::setw(12) << "Time (s)" << std::endl;
        std::cout << "-----------------------------------------------------------------------------" << std::endl;

        bool all_successful = true;
        for (const auto& pair : files) {
            const std::string& input_file = pair.first;
            const std::string& output_file = pair.second;

            std::cout << std::left << std::setw(25) << input_file.substr(input_file.find_last_of("/\\") + 1);

            auto start_time = std::chrono::high_resolution_clock::now();
            ArithmeticEncoder encoder;
            bool success = false;
            std::streamsize orig_size = getFileSize(input_file);
            std::streamsize comp_size = -1;

            if (orig_size < 0) {
                std::cerr << "\n  Error reading " << input_file << std::endl;
                all_successful = false;
            } else if (!encoder.encode(input_file, output_file)) {
                std::cerr << "\n  Error encoding " << input_file << std::endl;
                std::remove(output_file.c_str());
                all_successful = false;
            } else {
                success = true;
                comp_size = getFileSize(output_file);
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end_time - start_time;

            if (success) {
                double ratio = calculateCompressionRatio(orig_size, comp_size);
                std::cout << std::right << std::setw(15) << orig_size
                          << std::setw(15) << comp_size
                          << std::setw(6) << std::fixed << std::setprecision(2) << ratio << ":1   "
                          << std::setw(10) << std::fixed << std::setprecision(3) << duration.count() << std::endl;
            } else {
                std::cout << std::right << std::setw(15) << (orig_size >= 0 ? std::to_string(orig_size) : "N/A")
                          << std::setw(15) << "FAIL"
                          << std::setw(12) << "N/A"
                          << std::setw(12) << std::fixed << std::setprecision(3) << duration.count() << std::endl;
            }
        }
        std::cout << "-----------------------------------------------------------------------------" << std::endl;
        if (!all_successful) return 1;

    } else if (mode == "decode_all") {
        const std::vector<std::pair<std::string, std::string>> files = {
            {"lena_ascii.codestream", "lena_ascii-rec.pgm"},
            {"baboon_ascii.codestream", "baboon_ascii-rec.pgm"},
            {"quadrado_ascii.codestream", "quadrado_ascii-rec.pgm"}
        };
        std::cout << "Decoding All Specified Codestreams..." << std::endl;
        std::cout << "------------------------------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(28) << "Input Codestream"
                  << std::right << std::setw(15) << "Decoded Size"
                  << std::setw(15) << "Output File"
                  << std::setw(12) << "Time (s)" << std::endl;
        std::cout << "------------------------------------------------------------------" << std::endl;

        bool all_successful = true;
        for (const auto& pair : files) {
            const std::string& input_file = pair.first;
            const std::string& output_file = pair.second;

            std::cout << std::left << std::setw(28) << input_file.substr(input_file.find_last_of("/\\") + 1);

            auto start_time = std::chrono::high_resolution_clock::now();
            ArithmeticDecoder decoder;
            bool success = false;
            std::streamsize decoded_size = -1;

            if (getFileSize(input_file) < 0) {
                std::cerr << "\n  Error: Input file not found: " << input_file << std::endl;
                all_successful = false;
            }
            else if (!decoder.decode(input_file, output_file)) {
                std::cerr << "\n  Error decoding " << input_file << std::endl;
                std::remove(output_file.c_str());
                all_successful = false;
            } else {
                success = true;
                decoded_size = getFileSize(output_file);
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end_time - start_time;

            if (success) {
                std::cout << std::right << std::setw(15) << decoded_size
                          << std::setw(1) << " " << std::left << std::setw(14) << output_file.substr(output_file.find_last_of("/\\") + 1)
                          << std::right << std::setw(10) << std::fixed << std::setprecision(3) << duration.count() << std::endl;
            } else {
                std::cout << std::right << std::setw(15) << "FAIL"
                          << std::setw(1) << " " << std::left << std::setw(14) << output_file.substr(output_file.find_last_of("/\\") + 1)
                          << std::right << std::setw(10) << std::fixed << std::setprecision(3) << duration.count() << std::endl;
            }
        }
        std::cout << "------------------------------------------------------------------" << std::endl;
        if (!all_successful) return 1;

    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }

    auto global_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> global_duration = global_end_time - global_start_time;
    std::cout << "Total execution time: " << std::fixed << std::setprecision(3) << global_duration.count() << " seconds" << std::endl;

    return 0;
}