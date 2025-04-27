#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

const uint32_t CODE_VALUE_BITS = 32;
const uint64_t TOP_VALUE_CALC = ((uint64_t)1 << CODE_VALUE_BITS) - 1;
const uint32_t TOP_VALUE = (uint32_t)TOP_VALUE_CALC;
const uint32_t FIRST_QTR = (TOP_VALUE / 4) + 1;
const uint32_t HALF = 2 * FIRST_QTR;
const uint32_t THIRD_QTR = 3 * FIRST_QTR;
const uint64_t MAX_FREQ_SUM = ((uint64_t)1 << 28);

#endif