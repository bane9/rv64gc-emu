#include "helper.hpp"
#include "cpu_config.hpp"
#include <chrono>
#include <cstring>
#include <fmt/core.h>
#include <fstream>

void helper::dump(std::ostream& stream, const uint8_t* data, size_t data_len)
{
    for (const uint8_t* byte = data; byte < data + data_len; byte++)
    {
        stream << *byte;
    }
}

void helper::dump_hex(std::ostream& stream, const uint8_t* data, size_t data_len, int increment)
{
    for (size_t row = 0; row < data_len; row += increment)
    {
        stream << fmt::format("{:0>8X}: ", row);

        for (size_t byte_idx = row; byte_idx < row + increment; byte_idx++)
        {
            stream << fmt::format("{:0>2x}", data[byte_idx]);

            if (byte_idx < row + increment - 1)
            {
                stream << ' ';
            }
        }

        stream << '\n';
    }
}

void helper::store8(uint8_t* data, uint64_t value)
{
    data[0] = value;
}

void helper::store16(uint8_t* data, uint64_t value)
{
    memcpy(data, &value, 2);
}

void helper::store32(uint8_t* data, uint64_t value)
{
    memcpy(data, &value, 4);
}

void helper::store64(uint8_t* data, uint64_t value)
{
    memcpy(data, &value, 8);
}

uint64_t helper::load8(uint8_t* data)
{
    return data[0];
}

uint64_t helper::load16(uint8_t* data)
{
    uint64_t ret = 0;
    memcpy(&ret, data, 2);

    return ret;
}

uint64_t helper::load32(uint8_t* data)
{
    uint64_t ret = 0;
    memcpy(&ret, data, 4);

    return ret;
}

uint64_t helper::load64(uint8_t* data)
{
    uint64_t ret = 0;
    memcpy(&ret, data, 8);

    return ret;
}

uint64_t helper::read_bit(uint64_t value, uint64_t offset)
{
    return (value & (1ULL << offset)) != 0ULL;
}

uint64_t helper::read_bits(uint64_t value, uint64_t upper_offset, uint64_t lower_offset)
{
    uint64_t mask = (1ULL << (upper_offset - lower_offset + 1ULL)) - 1ULL;
    return (value >> lower_offset) & mask;
}

uint64_t helper::write_bit(uint64_t value, uint64_t offset, uint64_t write_value)
{
    if (write_value == 0)
    {
        return value & ~(1ULL << offset);
    }
    else
    {
        return value | (1ULL << offset);
    }
}

uint64_t helper::write_bits(uint64_t value, uint64_t upper_offset, uint64_t lower_offset,
                            uint64_t write_value)
{
    uint64_t mask = (1ULL << (upper_offset - lower_offset + 1ULL)) - 1ULL;

    mask <<= lower_offset;
    write_value <<= lower_offset;
    value &= ~mask;
    value |= write_value;

    return value;
}

uint64_t helper::get_milliseconds()
{
    static const auto start_time = std::chrono::high_resolution_clock::now();

    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}

std::vector<uint8_t> helper::load_file(const char* filename)
{
    std::ifstream input(filename, std::ios::binary);
    return std::vector<uint8_t>{std::istreambuf_iterator<char>(input),
                                std::istreambuf_iterator<char>()};
}