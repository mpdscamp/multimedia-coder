#ifndef BIT_IO_HPP
#define BIT_IO_HPP

#include <iostream>
#include <cstdint>

class BitIO {
private:
    std::ostream* out_stream;
    std::istream* in_stream;
    unsigned char buffer;
    int bits_in_buffer;
    bool is_writing;
    uint64_t bits_processed;

public:
    BitIO(std::ostream* os);
    BitIO(std::istream* is);
    ~BitIO();

    void writeBit(int bit);
    int readBit();
    void flush();
    uint64_t getBitsProcessed() const;
};

#endif