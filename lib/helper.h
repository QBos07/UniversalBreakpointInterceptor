#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // IWYU pragma: export
#include <stdint.h> // IWYU pragma: export

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus // C++

#include <type_traits> // IWYU pragma: export

#else // C

#if __STDC_VERSION__ < 202311L // C23
#include <assert.h>            // IWYU pragma: export
#include <stdbool.h>           // IWYU pragma: export
#endif

#endif

#ifdef __cplusplus
#define _PtrCast(ptr, to) reinterpret_cast<to *>((ptr))
#define _GetType(expr)                                                         \
  std::remove_reference_t<std::remove_cv_t<decltype((expr))>>
#else
#define _PtrCast(ptr, to) (to *)((ptr))
#define _GetType(expr) typeof((expr))
#endif

#ifdef __has_builtin
#if __has_builtin(__builtin_bit_cast)
#define _TypePun(into_var, from_var)                                           \
  do {                                                                         \
    static_assert(sizeof((into_var)) == sizeof((from_var)),                    \
                  "type punning not possible, sizes do not match");            \
    *(&(into_var)) = __builtin_bit_cast(_GetType(into_var), (from_var));       \
  } while (false)
#endif
#endif

#ifndef _TypePun
#define _TypePun(into_var, from_var)                                           \
  do {                                                                         \
    static_assert(sizeof((into_var)) == sizeof((from_var)),                    \
                  "type punning not possible, sizes do not match");            \
    for (size_t _type_punning_offset = 0;                                      \
         _type_punning_offset < sizeof((into_var)); _type_punning_offset++)    \
      _PtrCast(&(into_var), uint8_t)[_type_punning_offset] =                   \
          _PtrCast(&(from_var), uint8_t)[_type_punning_offset];                \
  } while (false)
#endif

#define regstruct struct __attribute__((packed, aligned(alignof(uint32_t))))
#define HW_REG_T(name, address, type)                                          \
  static volatile type *const name = _PtrCast(address, type)
#define HW_REG32(name, address) HW_REG_T(name, address, uint32_t)

#define AS_STRUCT_TYPE(name) AS_STRUCT_TYPE_##name
#define AS_STRUCT_GET(name, into)                                              \
  do {                                                                         \
    _GetType(*name) _tmp = *name;                                              \
    _TypePun(into, _tmp);                                                      \
  } while (false)
#if defined(__GNUC__) || defined(__clang__)
#define AS_STRUCT_EXPR(name)                                                   \
  (__extension__({                                                             \
    AS_STRUCT_TYPE(name) _tmp;                                                 \
    AS_STRUCT_GET(name, _tmp);                                                 \
    _tmp;                                                                      \
  }))
#endif
#define AS_STRUCT_SET(name, ...)                                               \
  do {                                                                         \
    _GetType(*name) _tmp1;                                                     \
    AS_STRUCT_TYPE(name) _tmp2 = __VA_ARGS__;                                  \
    _TypePun(_tmp1, _tmp2);                                                    \
    *name = _tmp1;                                                             \
  } while (false)
#define AS_STRUCT_MODIFY(name, ...)                                            \
  do {                                                                         \
    AS_STRUCT_TYPE(name) _s##name;                                             \
    AS_STRUCT_GET(name, _s##name);                                             \
    (_s##name) __VA_ARGS__;                                                    \
    AS_STRUCT_SET(name, _s##name);                                             \
  } while (false);
