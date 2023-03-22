#include "misc.hpp"
#include "helper.hpp"
#include <cstring>

Fregister::Fregister() : value(0)
{
}

Fregister::Fregister(float val)
{
    set_value(val);
}

Fregister::Fregister(double val)
{
    set_value(val);
}

Fregister::Fregister(int32_t val)
{
    set_value(val);
}

Fregister::Fregister(uint32_t val)
{
    set_value(val);
}

Fregister::Fregister(int64_t val)
{
    set_value(val);
}

Fregister::Fregister(uint64_t val)
{
    set_value(val);
}

Fregister& Fregister::operator=(float val)
{
    set_value(val);
    return *this;
}

Fregister& Fregister::operator=(double val)
{
    set_value(val);
    return *this;
}

Fregister& Fregister::operator=(int32_t val)
{
    set_value(val);
    return *this;
}

Fregister& Fregister::operator=(uint32_t val)
{
    set_value(val);
    return *this;
}

Fregister& Fregister::operator=(int64_t val)
{
    set_value(val);
    return *this;
}

Fregister& Fregister::operator=(uint64_t val)
{
    set_value(val);
    return *this;
}

Fregister::operator float() const
{
    return get_float();
}

Fregister::operator double() const
{
    return get_double();
}

Fregister::operator int32_t() const
{
    return get_i32();
}

Fregister::operator uint32_t() const
{
    return get_u32();
}

Fregister::operator int64_t() const
{
    return get_i64();
}

Fregister::operator uint64_t() const
{
    return get_u64();
}

void Fregister::set_value(float val)
{
    value = 0;
    memcpy(&value, &val, sizeof(val));
}

void Fregister::set_value(double val)
{
    value = 0;
    memcpy(&value, &val, sizeof(val));
}

void Fregister::set_value(int32_t val)
{
    value = 0;
    memcpy(&value, &val, sizeof(val));
}

void Fregister::set_value(uint32_t val)
{
    value = 0;
    memcpy(&value, &val, sizeof(val));
}

void Fregister::set_value(int64_t val)
{
    value = 0;
    memcpy(&value, &val, sizeof(val));
}

void Fregister::set_value(uint64_t val)
{
    value = val;
}

float Fregister::get_float() const
{
    float val = 0.0f;
    memcpy(&val, &value, sizeof(val));

    return val;
}

double Fregister::get_double() const
{
    double val = 0.0;
    memcpy(&val, &value, sizeof(val));

    return val;
}

int32_t Fregister::get_i32() const
{
    return value;
}

uint32_t Fregister::get_u32() const
{
    return value;
}

int64_t Fregister::get_i64() const
{
    return value;
}

uint64_t Fregister::get_u64() const
{
    return value;
}
