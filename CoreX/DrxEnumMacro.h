// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <type_traits>

// Примечание: Этот макрос используется только в отношении перечней,
// расположенных вне классов и структур!
#define DRX_CREATE_ENUM_FLAG_OPERATORS(ENUM)                                 \
  constexpr std::underlying_type<ENUM>::type IntegralValue(ENUM const t)     \
  {                                                                          \
    return static_cast<std::underlying_type<ENUM>::type>(t);                 \
  }                                                                          \
                                                                             \
  constexpr ENUM operator|(ENUM const lhs, ENUM const rhs)                   \
  {                                                                          \
    return static_cast<ENUM>(IntegralValue(lhs) | IntegralValue(rhs));       \
  }                                                                          \
                                                                             \
  constexpr ENUM operator&(ENUM const lhs, ENUM const rhs)                   \
  {                                                                          \
    return static_cast<ENUM>(IntegralValue(lhs) & IntegralValue(rhs));       \
  }                                                                          \
                                                                             \
  constexpr ENUM operator~(ENUM const rhs)                                   \
  {                                                                          \
    return static_cast<ENUM>(~IntegralValue(rhs));                           \
  }                                                                          \
                                                                             \
  inline void operator|=(ENUM& lhs, ENUM const rhs)                          \
  {                                                                          \
    lhs = static_cast<ENUM>(IntegralValue(lhs) | IntegralValue(rhs));        \
  }                                                                          \
                                                                             \
  inline void operator&=(ENUM& lhs, ENUM const rhs)                          \
  {                                                                          \
    lhs = static_cast<ENUM>(IntegralValue(lhs) & IntegralValue(rhs));        \
  }                                                                          \
                                                                             \
  template<typename TLiteral>                                                \
  constexpr                                                                  \
  typename std::enable_if<std::is_integral<TLiteral>::value, TLiteral>::type \
  operator&(TLiteral const lhs, ENUM const rhs)                              \
  {                                                                          \
    return lhs & IntegralValue(rhs);                                         \
  }                                                                          \
                                                                             \
  template<typename TLiteral>                                                \
  constexpr                                                                  \
  typename std::enable_if<std::is_integral<TLiteral>::value, bool>::type     \
  operator==(ENUM const lhs, TLiteral const rhs)                             \
  {                                                                          \
    return IntegralValue(lhs) == rhs;                                        \
  }                                                                          \
                                                                             \
  template<typename TLiteral>                                                \
  constexpr                                                                  \
  typename std::enable_if<std::is_integral<TLiteral>::value, bool>::type     \
  operator==(TLiteral const lhs, ENUM const rhs)                             \
  {                                                                          \
    return lhs == IntegralValue(rhs);                                        \
  }                                                                          \
                                                                             \
  template<typename TLiteral>                                                \
  constexpr                                                                  \
  typename std::enable_if<std::is_integral<TLiteral>::value, bool>::type     \
  operator!=(ENUM const lhs, TLiteral const rhs)                             \
  {                                                                          \
    return IntegralValue(lhs) != rhs;                                        \
  }                                                                          \
                                                                             \
  template<typename TLiteral>                                                \
  constexpr                                                                  \
  typename std::enable_if<std::is_integral<TLiteral>::value, bool>::type     \
  operator>(ENUM const lhs, TLiteral const rhs)                              \
  {                                                                          \
    return IntegralValue(lhs) > rhs;                                         \
  }                                                                          \
                                                                             \
  template<typename TLiteral>                                                \
  constexpr                                                                  \
  typename std::enable_if<std::is_integral<TLiteral>::value, TLiteral>::type \
  operator-(ENUM const lhs, TLiteral const rhs)                              \
  {                                                                          \
    return IntegralValue(lhs) - rhs;                                         \
  }                                                                          \

