#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <iostream>

std::streamsize getFileSize(const std::string& filename);
double calculateCompressionRatio(std::streamsize original_size, std::streamsize compressed_size);

#endif