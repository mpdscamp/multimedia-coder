#include "bit_io.hpp"

BitIO::BitIO(std::ostream* os) 
    : out_stream(os), in_stream(nullptr), buffer(0), bits_in_buffer(0), is_writing(true), bits_processed(0) {}

BitIO::BitIO(std::istream* is) 
    : out_stream(nullptr), in_stream(is), buffer(0), bits_in_buffer(0), is_writing(false), bits_processed(0) {}

BitIO::~BitIO() {
    if (is_writing && bits_in_buffer > 0) {
        flush();
    }
}

void BitIO::writeBit(int bit) {
    if (!is_writing || !out_stream || !out_stream->good()) return;
    buffer = (buffer << 1) | (bit & 1);
    bits_in_buffer++;
    bits_processed++;
    if (bits_in_buffer == 8) {
        out_stream->put(buffer);
        buffer = 0;
        bits_in_buffer = 0;
    }
}

int BitIO::readBit() {
    if (is_writing || !in_stream || in_stream->eof()) return -1;
    if (bits_in_buffer == 0) {
        char c;
        if (!in_stream->get(c)) {
            return -1;
        }
        buffer = static_cast<unsigned char>(c);
        bits_in_buffer = 8;
    }
    bits_in_buffer--;
    int bit = (buffer >> bits_in_buffer) & 1;
    bits_processed++;
    return bit;
}

void BitIO::flush() {
    if (!is_writing || !out_stream || !out_stream->good() || bits_in_buffer == 0) return;
    buffer <<= (8 - bits_in_buffer);
    out_stream->put(buffer);
    buffer = 0;
    bits_in_buffer = 0;
}

uint64_t BitIO::getBitsProcessed() const { 
    return bits_processed; 
}