#pragma once

template<typename T>
class Optional {
    bool hasValue;
    T value;
public:
    Optional() : hasValue(false) {}
    Optional(const T& val) : hasValue(true), value(val) {}

    bool has_value() const { return hasValue; }
    T& operator*() { return value; }
    const T& operator*() const { return value; }
    T take() { return value;}
};

constexpr struct nullopt_t {} nullopt;