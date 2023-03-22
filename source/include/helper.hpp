#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <utility>
#include <vector>

#define SIZE_KIB(size) (1024UL * (size))
#define SIZE_MIB(size) ((1024UL * 1024UL) * (size))

#define SIGNEXTEND_CAST2(val, upcast_from) (static_cast<int64_t>(static_cast<upcast_from>(val)))

#define SIGNEXTEND_CAST(val, upcast_from)                                                          \
    (static_cast<uint64_t>(SIGNEXTEND_CAST2(val, upcast_from)))

namespace helper
{
template <class To, class From> To bit_cast(const From& src)
{
    To dst;
    memcpy(&dst, &src, sizeof(To));
    return dst;
}

void dump(std::ostream& stream, const uint8_t* data, size_t data_len);
void dump_hex(std::ostream& stream, const uint8_t* data, size_t data_len, int increment = 32);

void store8(uint8_t* data, uint64_t value);
void store16(uint8_t* data, uint64_t value);
void store32(uint8_t* data, uint64_t value);
void store64(uint8_t* data, uint64_t value);

uint64_t load8(uint8_t* data);
uint64_t load16(uint8_t* data);
uint64_t load32(uint8_t* data);
uint64_t load64(uint8_t* data);

uint64_t read_bit(uint64_t value, uint64_t offset);
uint64_t read_bits(uint64_t value, uint64_t upper_offset, uint64_t lower_offset);

uint64_t write_bit(uint64_t value, uint64_t offset, uint64_t write_value);
uint64_t write_bits(uint64_t value, uint64_t upper_offset, uint64_t lower_offset,
                    uint64_t write_value);

uint64_t get_milliseconds();

std::vector<uint8_t> load_file(const char* filename);

template <typename T> bool value_in_range(T value, T lower_range, T upper_range)
{
    return (value > lower_range) && (value <= upper_range);
}
} // namespace helper