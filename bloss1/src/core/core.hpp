#pragma once

/**
 * @brief The heart and soul of the project (plus some type definitions).
 */

#include "pch.hpp"

// Unsigned integers
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Integers
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// Floats
typedef float  f32;
typedef double f64;

// Strings
typedef std::string str;

// Shorthand to bind event callbacks
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
