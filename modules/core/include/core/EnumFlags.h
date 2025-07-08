//core/EnumFlags.h
#pragma once

#include <type_traits>

#define ENABLE_ENUM_FLAG_OPERATORS(T) \
inline T operator|(T lhs, T rhs) { \
using Underlying = std::underlying_type_t<T>; \
return static_cast<T>(static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs)); \
} \
inline T& operator|=(T& lhs, T rhs) { \
lhs = lhs | rhs; \
return lhs; \
} \
inline T operator&(T lhs, T rhs) { \
using Underlying = std::underlying_type_t<T>; \
return static_cast<T>(static_cast<Underlying>(lhs) & static_cast<Underlying>(rhs)); \
} \
inline bool has_flag(T flags, T flag) { \
return (flags & flag) == flag; \
}
