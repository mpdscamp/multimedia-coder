#include "arithmetic_coder.hpp"
#include "constants.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <stdexcept>

ArithmeticEncoder::ArithmeticEncoder() 
    : low(0), high(TOP_VALUE), bits_to_follow(0), bit_io(nullptr), total_byte_count(0) {}

void ArithmeticEncoder::outputBitPlusFollow(int bit) {
    bit_io->writeBit(bit);
    while (bits_to_follow > 0) {
        bit_io->writeBit(!bit);
        bits_to_follow--;
    }
}

uint64_t ArithmeticEncoder::calculateByteFrequencyTables(std::istream& in,
                                      std::map<unsigned char, uint32_t>& frequency,
                                      std::map<unsigned char, uint64_t>& cumulative) {
    frequency.clear();
    cumulative.clear();
    uint64_t current_total_bytes = 0;
    std::map<unsigned char, uint64_t> freq64;

    char byte_char;
    while (in.get(byte_char)) {
        unsigned char byte_val = static_cast<unsigned char>(byte_char);
        freq64[byte_val]++;
        current_total_bytes++;
    }
    if (!in.eof()) {
        std::cerr << "Error reading input file during frequency calculation." << std::endl;
        return 0;
    }
    in.clear();
    in.seekg(0);

    for(const auto& pair : freq64) {
        if (pair.second > std::numeric_limits<uint32_t>::max()) {
            std::cerr << "Error: Frequency count for byte " << (int)pair.first << " exceeds uint32_t limit." << std::endl;
            return 0;
        }
        frequency[pair.first] = static_cast<uint32_t>(pair.second);
    }

    uint64_t cum = 0;
    for (auto const& pair : frequency) {
        unsigned char byte_val = pair.first;
        uint32_t freq = pair.second;
        if (freq == 0) continue;
        cumulative[byte_val] = cum;
        cum += freq;
    }

    if (cum != current_total_bytes) {
        std::cerr << "Internal Error: Cumulative frequency calculation mismatch (" << cum << " != " << current_total_bytes << ")" << std::endl;
        return 0;
    }

    return current_total_bytes;
}

bool ArithmeticEncoder::writeHeader(std::ostream& out,
                   uint64_t total_bytes,
                   const std::map<unsigned char, uint32_t>& frequency) {

    out.write(reinterpret_cast<const char*>(&total_bytes), sizeof(total_bytes));

    uint32_t num_symbols = frequency.size();
    out.write(reinterpret_cast<const char*>(&num_symbols), sizeof(num_symbols));

    for (auto const& pair : frequency) {
        unsigned char byte_val = pair.first;
        uint32_t freq = pair.second;
        out.write(reinterpret_cast<const char*>(&byte_val), sizeof(byte_val));
        out.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
    }

    out << std::flush;
    return out.good();
}

bool ArithmeticEncoder::encode(const std::string& input_filename, const std::string& output_filename) {
    std::ifstream infile(input_filename, std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error opening input file: " << input_filename << std::endl;
        return false;
    }

    std::ofstream outfile(output_filename, std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Error opening output file: " << output_filename << std::endl;
        infile.close();
        return false;
    }

    std::map<unsigned char, uint32_t> frequency;
    std::map<unsigned char, uint64_t> cumulative_frequency;
    total_byte_count = calculateByteFrequencyTables(infile, frequency, cumulative_frequency);

    if (total_byte_count == 0 && !frequency.empty()) {
        infile.close();
        outfile.close();
        std::remove(output_filename.c_str());
        return false;
    }

    if (total_byte_count == 0 && frequency.empty()) {
        std::cout << "Input file is empty. Writing minimal header." << std::endl;
        writeHeader(outfile, 0, frequency);
        infile.close();
        outfile.close();
        return true;
    }

    if (total_byte_count > MAX_FREQ_SUM) {
        std::cerr << "Error: Total byte count (" << total_byte_count
                  << ") exceeds maximum allowed (" << MAX_FREQ_SUM
                  << "). Cannot encode reliably." << std::endl;
        infile.close();
        outfile.close();
        std::remove(output_filename.c_str());
        return false;
    }

    if (!writeHeader(outfile, total_byte_count, frequency)) {
        std::cerr << "Error writing header to output file: " << output_filename << std::endl;
        infile.close();
        outfile.close();
        std::remove(output_filename.c_str());
        return false;
    }

    BitIO bit_io_obj(&outfile);
    bit_io = &bit_io_obj;

    low = 0;
    high = TOP_VALUE;
    bits_to_follow = 0;

    char byte_char;
    while (infile.get(byte_char)) {
        unsigned char byte_val = static_cast<unsigned char>(byte_char);

        uint64_t range = (uint64_t)high - low + 1;

        auto freq_it = frequency.find(byte_val);
        auto cum_freq_it = cumulative_frequency.find(byte_val);
        if (freq_it == frequency.end() || cum_freq_it == cumulative_frequency.end()) {
            std::cerr << "Internal Error: Byte " << (int)byte_val << " not found in frequency tables during encoding pass 2." << std::endl;
            infile.close();
            outfile.close();
            std::remove(output_filename.c_str());
            return false;
        }
        uint32_t sym_freq = freq_it->second;
        uint64_t sym_cum_freq = cum_freq_it->second;

        high = low + (uint32_t)((range * (sym_cum_freq + sym_freq)) / total_byte_count) - 1;
        low = low + (uint32_t)((range * sym_cum_freq) / total_byte_count);

        for (;;) {
            if (high < HALF) {
                outputBitPlusFollow(0);
            } else if (low >= HALF) {
                outputBitPlusFollow(1);
                low -= HALF;
                high -= HALF;
            } else if (low >= FIRST_QTR && high < THIRD_QTR) {
                bits_to_follow++;
                low -= FIRST_QTR;
                high -= FIRST_QTR;
            } else {
                break;
            }
            low <<= 1;
            high = (high << 1) + 1;
        }
    }
    if (!infile.eof()) {
        std::cerr << "Error reading input file during encoding pass 2." << std::endl;
        infile.close();
        outfile.close();
        std::remove(output_filename.c_str());
        return false;
    }

    bits_to_follow++;
    if (low < FIRST_QTR) {
        outputBitPlusFollow(0);
    } else {
        outputBitPlusFollow(1);
    }
    bit_io->flush();

    infile.close();
    outfile.close();
    if (!outfile) {
        std::cerr << "Error occurred during final write/close of file: " << output_filename << std::endl;
        return false;
    }
    return true;
}

ArithmeticDecoder::ArithmeticDecoder() 
    : low(0), high(TOP_VALUE), value(0), bit_io(nullptr), total_freq_sum(0) {}

int ArithmeticDecoder::inputBit() {
    return bit_io->readBit();
}

bool ArithmeticDecoder::readHeader(std::istream& in,
                 uint64_t& total_bytes_to_decode,
                 std::map<unsigned char, uint32_t>& frequency,
                 std::map<unsigned char, uint64_t>& cumulative,
                 std::map<uint64_t, unsigned char>& lookup_symbol)
{
    frequency.clear();
    cumulative.clear();
    lookup_symbol.clear();

    if (!in.read(reinterpret_cast<char*>(&total_bytes_to_decode), sizeof(total_bytes_to_decode))) {
        std::cerr << "Error reading total byte count from header." << std::endl;
        return false;
    }

    uint32_t num_symbols;
    if (!in.read(reinterpret_cast<char*>(&num_symbols), sizeof(num_symbols))) {
        std::cerr << "Error reading number of symbols from header." << std::endl;
        return false;
    }

    uint64_t current_total_freq = 0;
    for (uint32_t i = 0; i < num_symbols; ++i) {
        unsigned char byte_val;
        uint32_t freq;
        if (!in.read(reinterpret_cast<char*>(&byte_val), sizeof(byte_val)) ||
            !in.read(reinterpret_cast<char*>(&freq), sizeof(freq))) {
            std::cerr << "Error reading frequency table entry " << i << "." << std::endl;
            return false;
        }
        if (freq == 0) {
            std::cerr << "Warning: Symbol " << (int)byte_val << " has zero frequency in header." << std::endl;
            continue;
        }
        frequency[byte_val] = freq;
        current_total_freq += freq;
    }

    if (current_total_freq != total_bytes_to_decode && total_bytes_to_decode != 0) {
        std::cerr << "Warning: Sum of frequencies from header (" << current_total_freq
                  << ") does not match total byte count (" << total_bytes_to_decode << ")." << std::endl;
        if (current_total_freq == 0) {
            std::cerr << "Error: Frequency sum is zero but total bytes is non-zero." << std::endl;
            return false;
        }
    } else if (total_bytes_to_decode == 0 && current_total_freq != 0) {
        std::cerr << "Error: Total bytes is zero but frequency sum is non-zero." << std::endl;
        return false;
    }

    total_freq_sum = current_total_freq;

    if (total_freq_sum > MAX_FREQ_SUM && total_freq_sum != 0) {
        std::cerr << "Error: Total frequency sum (" << total_freq_sum
                  << ") read from header exceeds maximum allowed (" << MAX_FREQ_SUM
                  << "). Cannot decode reliably." << std::endl;
        return false;
    }

    uint64_t cum = 0;
    for (auto const& pair : frequency) {
        unsigned char byte_val = pair.first;
        uint32_t freq = pair.second;
        cumulative[byte_val] = cum;
        lookup_symbol[cum] = byte_val;
        cum += freq;
    }

    return in.good();
}

unsigned char ArithmeticDecoder::findSymbol(uint64_t scaled_value, const std::map<uint64_t, unsigned char>& lookup_symbol) {
    auto it = lookup_symbol.upper_bound(scaled_value);
    if (it == lookup_symbol.begin()) {
        if (lookup_symbol.empty() || lookup_symbol.begin()->first != 0 || scaled_value != 0) {
            std::cerr << "Error: Scaled value " << scaled_value << " is below the lowest cumulative frequency." << std::endl;
            throw std::runtime_error("Symbol lookup failed");
        }
        it = lookup_symbol.begin();
    } else {
        --it;
    }
    return it->second;
}

bool ArithmeticDecoder::initializeDecoder() {
    value = 0;
    for (int i = 0; i < CODE_VALUE_BITS; i++) {
        int bit = inputBit();
        if (bit == -1) {
            std::cerr << "Error: Premature EOF encountered while initializing decoder value (read " << i << " bits)." << std::endl;
            return false;
        }
        value = (value << 1) | bit;
    }
    return true;
}

bool ArithmeticDecoder::decode(const std::string& input_filename, const std::string& output_filename) {
    std::ifstream infile(input_filename, std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error opening input file: " << input_filename << std::endl;
        return false;
    }

    std::ofstream outfile(output_filename, std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Error opening output file: " << output_filename << std::endl;
        infile.close();
        return false;
    }

    uint64_t total_bytes_to_decode;
    std::map<unsigned char, uint32_t> frequency;
    std::map<unsigned char, uint64_t> cumulative_frequency;
    std::map<uint64_t, unsigned char> lookup_symbol;

    if (!readHeader(infile, total_bytes_to_decode, frequency,
                  cumulative_frequency, lookup_symbol)) {
        std::cerr << "Failed to read or validate header from: " << input_filename << std::endl;
        infile.close();
        outfile.close();
        std::remove(output_filename.c_str());
        return false;
    }

    if (total_bytes_to_decode == 0) {
        std::cout << "Header indicates empty file (0 bytes). Creating empty output file." << std::endl;
        infile.close();
        outfile.close();
        return true;
    }

    BitIO bit_io_obj(&infile);
    bit_io = &bit_io_obj;

    low = 0;
    high = TOP_VALUE;
    if (!initializeDecoder()) {
        infile.close();
        outfile.close();
        std::remove(output_filename.c_str());
        return false;
    }

    uint64_t bytes_decoded = 0;
    try {
        for (; bytes_decoded < total_bytes_to_decode; ++bytes_decoded) {
            uint64_t range = (uint64_t)high - low + 1;
            if (range == 0) {
                std::cerr << "Error: Decoder range became zero at byte " << bytes_decoded << "." << std::endl;
                infile.close(); outfile.close(); std::remove(output_filename.c_str()); return false;
            }

            uint64_t scaled_value = (((uint64_t)value - low + 1) * total_freq_sum - 1) / range;
            unsigned char decoded_byte = findSymbol(scaled_value, lookup_symbol);

            outfile.put(static_cast<char>(decoded_byte));

            auto freq_it = frequency.find(decoded_byte);
            auto cum_freq_it = cumulative_frequency.find(decoded_byte);
            if (freq_it == frequency.end() || cum_freq_it == cumulative_frequency.end()) {
                std::cerr << "Internal Error: Decoded byte " << (int)decoded_byte << " not found in frequency tables." << std::endl;
                infile.close(); outfile.close(); std::remove(output_filename.c_str()); return false;
            }
            uint32_t sym_freq = freq_it->second;
            uint64_t sym_cum_freq = cum_freq_it->second;

            high = low + (uint32_t)((range * (sym_cum_freq + sym_freq)) / total_freq_sum) - 1;
            low = low + (uint32_t)((range * sym_cum_freq) / total_freq_sum);

            for (;;) {
                if (high < HALF) {
                } else if (low >= HALF) {
                    low -= HALF; high -= HALF; value -= HALF;
                } else if (low >= FIRST_QTR && high < THIRD_QTR) {
                    low -= FIRST_QTR; high -= FIRST_QTR; value -= FIRST_QTR;
                } else {
                    break;
                }
                low <<= 1;
                high = (high << 1) + 1;
                int bit = inputBit();
                value = (value << 1) | (bit == -1 ? 0 : bit);
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error during decoding: " << e.what() << std::endl;
        infile.close(); outfile.close(); std::remove(output_filename.c_str()); return false;
    }

    infile.close();
    outfile.close();

    if (bytes_decoded != total_bytes_to_decode) {
        std::cerr << "Error: Decoded " << bytes_decoded << " bytes, but expected " << total_bytes_to_decode << " from header." << std::endl;
        return false;
    }
    if (!outfile) {
        std::cerr << "Error occurred during final write/close of file: " << output_filename << std::endl;
        return false;
    }

    return true;
}