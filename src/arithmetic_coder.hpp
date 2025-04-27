#ifndef ARITHMETIC_CODER_HPP
#define ARITHMETIC_CODER_HPP

#include <string>
#include <map>
#include <cstdint>
#include "bit_io.hpp"

class ArithmeticEncoder {
private:
    uint32_t low;
    uint32_t high;
    int bits_to_follow;
    BitIO* bit_io;
    uint64_t total_byte_count;

    void outputBitPlusFollow(int bit);
    uint64_t calculateByteFrequencyTables(std::istream& in,
                                          std::map<unsigned char, uint32_t>& frequency,
                                          std::map<unsigned char, uint64_t>& cumulative);
    bool writeHeader(std::ostream& out,
                     uint64_t total_bytes,
                     const std::map<unsigned char, uint32_t>& frequency);

public:
    ArithmeticEncoder();
    bool encode(const std::string& input_filename, const std::string& output_filename);
};

class ArithmeticDecoder {
private:
    uint32_t low;
    uint32_t high;
    uint32_t value;
    BitIO* bit_io;
    uint64_t total_freq_sum;

    int inputBit();
    bool readHeader(std::istream& in,
                   uint64_t& total_bytes_to_decode,
                   std::map<unsigned char, uint32_t>& frequency,
                   std::map<unsigned char, uint64_t>& cumulative,
                   std::map<uint64_t, unsigned char>& lookup_symbol);
    unsigned char findSymbol(uint64_t scaled_value, const std::map<uint64_t, unsigned char>& lookup_symbol);
    bool initializeDecoder();

public:
    ArithmeticDecoder();
    bool decode(const std::string& input_filename, const std::string& output_filename);
};

#endif