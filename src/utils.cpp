#include "utils.hpp"
#include <fstream>

std::streamsize getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return -1;
    }
    std::streamsize size = file.tellg();
    file.close();
    return size;
}

double calculateCompressionRatio(std::streamsize original_size, std::streamsize compressed_size) {
    if (original_size <= 0 || compressed_size <= 0) {
        return 0.0;
    }
    return static_cast<double>(original_size) / compressed_size;
}