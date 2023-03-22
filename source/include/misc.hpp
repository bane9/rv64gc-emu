#pragma once

#include <cstdint>
#include <type_traits>

class Fregister
{
  public:
    Fregister();
    Fregister(float val);
    Fregister(double val);

    Fregister(int32_t val);
    Fregister(uint32_t val);

    Fregister(int64_t val);
    Fregister(uint64_t val);

  public:
    Fregister& operator=(float val);
    Fregister& operator=(double val);

    Fregister& operator=(int32_t val);
    Fregister& operator=(uint32_t val);

    Fregister& operator=(int64_t val);
    Fregister& operator=(uint64_t val);

  public:
    operator float() const;
    operator double() const;

    operator int32_t() const;
    operator uint32_t() const;

    operator int64_t() const;
    operator uint64_t() const;

  public:
    void set_value(float val);
    void set_value(double val);

    void set_value(int32_t val);
    void set_value(uint32_t val);

    void set_value(int64_t val);
    void set_value(uint64_t val);

    float get_float() const;
    double get_double() const;

    int32_t get_i32() const;
    uint32_t get_u32() const;

    int64_t get_i64() const;
    uint64_t get_u64() const;

    template <typename T> T get_value() const
    {
        if constexpr (std::is_same_v<T, float>)
        {
            return get_float();
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return get_double();
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            return get_i32();
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            return get_u32();
        }
        else if constexpr (std::is_same_v<T, int64_t>)
        {
            return get_i64();
        }
        else if constexpr (std::is_same_v<T, uint64_t>)
        {
            return get_u64();
        }
    }

  public:
    uint64_t value;
};