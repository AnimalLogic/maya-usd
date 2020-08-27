//
// Copyright 2017 Animal Logic
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef MAYAUSDUTILS_SIMD_H
#define MAYAUSDUTILS_SIMD_H

#ifdef _WIN32
# define ALIGN16(X) __declspec(align(16)) X
# define ALIGN32(X) __declspec(align(32)) X
#else
# define ALIGN16(X) X __attribute__((aligned(16)))
# define ALIGN32(X) X __attribute__((aligned(32)))
#endif

#include <stdint.h>
#include <cmath>
#include <cstring>

#ifdef __AVX2__
# include <immintrin.h>
#endif

#ifdef __SSE__
# include <mmintrin.h>
#endif

#ifdef __SSE2__
# include <emmintrin.h>
#endif

#ifdef __SSE3__
# include <pmmintrin.h>
#endif

#ifdef __SSE4_1__
# include <smmintrin.h>
#endif

#if !defined(__F16C__)
# include <pxr/base/gf/half.h>
# include <pxr/base/gf/ilmbase_half.h>
PXR_NAMESPACE_USING_DIRECTIVE
#endif


namespace MayaUsdUtils {

#if defined(__SSE2__)

typedef __m128 f128;
typedef __m128i i128;
typedef __m128d d128;

# define shiftBytesLeft(reg, count) _mm_slli_si128(reg, count)
# define shiftBytesRight(reg, count) _mm_srli_si128(reg, count)

template<int X, int Y> inline d128 permute2d(const d128 a) { return _mm_shuffle_pd(a, a, X | (Y << 1)); }
template<int X, int Y, int Z, int W> inline f128 permute4f(const f128 a) { return _mm_shuffle_ps(a, a, X | (Y << 2) | (Z << 4) | (W << 6)); }
template<int X, int Y, int Z, int W> inline f128 shuffle4f(const f128 a, const f128 b) { return _mm_shuffle_ps(a, b, X | (Y << 2) | (Z << 4) | (W << 6)); }

template<int numBits> inline i128 lshift32(const i128 X) { return _mm_slli_epi32(X, numBits); }
template<int numBits> inline i128 rshift32(const i128 X) { return _mm_srli_epi32(X, numBits); }
template<int numBits> inline i128 rshift32i(const i128 X) { return _mm_srai_epi32(X, numBits); }
template<int numBits> inline i128 lshift64(const i128 X) { return _mm_slli_epi64(X, numBits); }
template<int numBits> inline i128 rshift64(const i128 X) { return _mm_srli_epi64(X, numBits); }

template<int numBytes> inline i128 lbyte_shift(const i128 X) { return _mm_slli_si128(X, numBytes); }
template<int numBytes> inline i128 rbyte_shift(const i128 X) { return _mm_srli_si128(X, numBytes); }

inline d128 zero2d() { return _mm_setzero_pd(); }
inline f128 zero4f() { return _mm_setzero_ps(); }
inline i128 zero4i() { return _mm_setzero_si128(); }

inline double cvtsd(const d128 x) { return _mm_cvtsd_f64(x); }
inline float cvtss(const f128 x) { return _mm_cvtss_f32(x); }

inline f128 loadu4f(const void* const ptr) { return _mm_loadu_ps((const float*)ptr); }
inline i128 loadu4i(const void* const ptr) { return _mm_loadu_si128((const i128*)ptr); }
inline d128 loadu2d(const void* const ptr) { return _mm_loadu_pd((const double*)ptr); }

inline f128 load4f(const void* const ptr) { return _mm_loadu_ps((const float*)ptr); }
inline i128 load4i(const void* const ptr) { return _mm_loadu_si128((const i128*)ptr); }
inline d128 load2d(const void* const ptr) { return _mm_loadu_pd((const double*)ptr); }

inline void storeu4f(void* const ptr, const f128 reg) { _mm_storeu_ps((float*)ptr, reg); }
inline void storeu4i(void* const ptr, const i128 reg) { _mm_storeu_si128((i128*)ptr, reg); }
inline void storeu2d(void* const ptr, const d128 reg) { _mm_storeu_pd((double*)ptr, reg); }

inline void store4f(void* const ptr, const f128 reg) { _mm_storeu_ps((float*)ptr, reg); }
inline void store4i(void* const ptr, const i128 reg) { _mm_storeu_si128((i128*)ptr, reg); }
inline void store2d(void* const ptr, const d128 reg) { _mm_storeu_pd((double*)ptr, reg); }

inline f128 cast4f(const d128 reg) { return _mm_castpd_ps(reg); }
inline f128 cast4f(const i128 reg) { return _mm_castsi128_ps(reg); }
inline i128 cast4i(const d128 reg) { return _mm_castpd_si128(reg); }
inline i128 cast4i(const f128 reg) { return _mm_castps_si128(reg); }
inline d128 cast2d(const f128 reg) { return _mm_castps_pd(reg); }
inline d128 cast2d(const i128 reg) { return _mm_castsi128_pd(reg); }

inline f128 load1f(const void* const ptr) { return _mm_load_ss((const float*)ptr); }
inline f128 load2f(const void* const ptr) { return cast4f(_mm_load_sd((const double*)ptr)); }
inline i128 load2i(const void* const ptr) { return cast4i(_mm_load_sd((const double*)ptr)); }

inline int32_t movemask16i8(const i128 reg) { return _mm_movemask_epi8(reg); }
inline int32_t movemask4i(const i128 reg) { return _mm_movemask_ps(cast4f(reg)); }
inline int32_t movemask4f(const f128 reg) { return _mm_movemask_ps(reg); }
inline int32_t movemask2d(const d128 reg) { return _mm_movemask_pd(reg); }
inline int32_t movemask2i64(const i128 reg) { return _mm_movemask_pd(cast2d(reg)); }

inline i128 cmpeq4i(const i128 a, const i128 b) { return _mm_cmpeq_epi32(a, b); }
inline i128 cmpeq16i8(const i128 a, const i128 b) { return _mm_cmpeq_epi8(a, b); }
inline i128 cmplt16i8(const i128 a, const i128 b) { return _mm_cmplt_epi8(a, b); }
inline i128 cmpgt16i8(const i128 a, const i128 b) { return _mm_cmpgt_epi8(a, b); }

inline f128 cmpgt4f(const f128 a, const f128 b) { return _mm_cmpgt_ps(a, b); }
inline d128 cmpgt2d(const d128 a, const d128 b) { return _mm_cmpgt_pd(a, b); }
inline f128 cmpne4f(const f128 a, const f128 b) { return _mm_cmpneq_ps(a, b); }
inline d128 cmpne2d(const d128 a, const d128 b) { return _mm_cmpneq_pd(a, b); }
inline i128 cmpeq8i16(const i128 a, const i128 b) { return _mm_cmpeq_epi16(a, b); }

# if !defined(__SSE4_1__) && !defined(__SSE4_2__) && !defined(__AVX__) && !defined(__AVX2__)
inline i128 cmpeq2i64(const i128 a, const i128 b) {
  alignas(16) int64_t A[2], B[2];
  store4i(A, a);
  store4i(B, b);
  A[0] = A[0] == B[0] ? 0xFFFFFFFFFFFFFFFFULL : 0;
  A[1] = A[1] == B[1] ? 0xFFFFFFFFFFFFFFFFULL : 0;
  return load4i(A);
}
#else
inline i128 cmpeq2i64(const i128 a, const i128 b) { return _mm_cmpeq_epi64(a, b); }
#endif

inline f128 set4f(const float a, const float b, const float c, const float d) {return _mm_setr_ps(a, b, c, d); }
inline i128 set4i(const int32_t a, const int32_t b, const int32_t c, const int32_t d) {return _mm_setr_epi32(a, b, c, d); }
inline d128 set2d(const double a, const double b) {return _mm_setr_pd(a, b); }

inline i128 set16i8(
    const int8_t a0, const int8_t b0, const int8_t c0, const int8_t d0,
    const int8_t a1, const int8_t b1, const int8_t c1, const int8_t d1,
    const int8_t a2, const int8_t b2, const int8_t c2, const int8_t d2,
    const int8_t a3, const int8_t b3, const int8_t c3, const int8_t d3)
{return _mm_setr_epi8(a0, b0, c0, d0, a1, b1, c1, d1, a2, b2, c2, d2, a3, b3, c3, d3); }

inline d128 cvt2f_to_2d(const f128 reg) { return _mm_cvtps_pd(reg); }
inline f128 cvt2d_to_2f(const d128 reg) { return _mm_cvtpd_ps(reg); }

inline f128 movehl4f(const f128 a, const f128 b) { return _mm_movehl_ps(a, b); }
inline f128 movelh4f(const f128 a, const f128 b) { return _mm_movelh_ps(a, b); }
inline i128 movehl4i(const i128 a, const i128 b) { return cast4i(_mm_movehl_ps(cast4f(a), cast4f(b))); }
inline i128 movelh4i(const i128 a, const i128 b) { return cast4i(_mm_movelh_ps(cast4f(a), cast4f(b))); }

inline d128 or2d(const d128 a, const d128 b) { return _mm_or_pd(a, b); }
inline f128 or4f(const f128 a, const f128 b) { return _mm_or_ps(a, b); }
inline i128 or4i(const i128 a, const i128 b) { return _mm_or_si128(a, b); }

inline d128 and2d(const d128 a, const d128 b) { return _mm_and_pd(a, b); }
inline f128 and4f(const f128 a, const f128 b) { return _mm_and_ps(a, b); }
inline i128 and4i(const i128 a, const i128 b) { return _mm_and_si128(a, b); }

inline d128 andnot2d(const d128 a, const d128 b) { return _mm_andnot_pd(a, b); }
inline f128 andnot4f(const f128 a, const f128 b) { return _mm_andnot_ps(a, b); }
inline i128 andnot4i(const i128 a, const i128 b) { return _mm_andnot_si128(a, b); }

inline d128 xor2d(const d128 a, const d128 b) { return _mm_xor_pd(a, b); }
inline f128 xor4f(const f128 a, const f128 b) { return _mm_xor_ps(a, b); }
inline i128 xor4i(const i128 a, const i128 b) { return _mm_xor_si128(a, b); }

inline f128 mul4f(const f128 a, const f128 b) { return _mm_mul_ps(a, b); }
inline d128 mul2d(const d128 a, const d128 b) { return _mm_mul_pd(a, b); }

inline f128 div4f(const f128 a, const f128 b) { return _mm_div_ps(a, b); }
inline d128 div2d(const d128 a, const d128 b) { return _mm_div_pd(a, b); }

inline d128 add2d(const d128 a, const d128 b) { return _mm_add_pd(a, b); }
inline f128 add4f(const f128 a, const f128 b) { return _mm_add_ps(a, b); }
inline i128 add4i(const i128 a, const i128 b) { return _mm_add_epi32(a, b); }
inline i128 add2i64(const i128 a, const i128 b) { return _mm_add_epi64(a, b); }

inline d128 sub2d(const d128 a, const d128 b) { return _mm_sub_pd(a, b); }
inline f128 sub4f(const f128 a, const f128 b) { return _mm_sub_ps(a, b); }
inline i128 sub4i(const i128 a, const i128 b) { return _mm_sub_epi32(a, b); }
inline i128 sub2i64(const i128 a, const i128 b) { return _mm_sub_epi64(a, b); }

inline d128 splat2d(double f) { return _mm_set1_pd(f); }
inline f128 splat4f(float f) { return _mm_set1_ps(f); }
inline i128 splat4i(int32_t f) { return _mm_set1_epi32(f); }
inline i128 splat2i64(const int64_t f) { return _mm_set1_epi64x(f); }

inline f128 unpacklo4f(const f128 a, const f128 b) { return _mm_unpacklo_ps(a, b); }
inline f128 unpackhi4f(const f128 a, const f128 b) { return _mm_unpackhi_ps(a, b); }
inline d128 unpacklo2d(const d128 a, const d128 b) { return _mm_unpacklo_pd(a, b); }
inline d128 unpackhi2d(const d128 a, const d128 b) { return _mm_unpackhi_pd(a, b); }

# if !defined(__SSE4__) && !defined(__SSE4_1__) && !defined(__SSE4_2__) && !defined(__AVX__) && !defined(__AVX2__)
inline __m128 _mm_blendv_ps(__m128 a, __m128 b, __m128 c)
{
  return or4f(and4f(c, b), andnot4f(c, a));
}
inline __m128d _mm_blendv_pd(__m128d a, __m128d b, __m128d c)
{
  return or2d(and2d(c, b), andnot2d(c, a));
}
# endif

inline f128 select4f(const f128 falseResult, const f128 trueResult, const f128 cmp) { return _mm_blendv_ps(falseResult, trueResult, cmp); }
inline d128 select2d(const d128 falseResult, const d128 trueResult, const d128 cmp) { return _mm_blendv_pd(falseResult, trueResult, cmp); }

inline d128 abs2d(const d128 v) { return andnot2d(splat2d(-0.0), v); }
inline f128 abs4f(const f128 v) { return andnot4f(splat4f(-0.0f), v); }
inline d128 sqrt2d(const d128 v) { return _mm_sqrt_pd(v); }
inline f128 sqrt4f(const f128 v) { return _mm_sqrt_ps(v); }

# if !defined(__SSE4_1__) && !defined(__SSE4_2__) && !defined(__AVX__) && !defined(__AVX2__)
template<bool x, bool y>
inline d128 select2d(const d128 a, const d128 b)
{
  alignas(16) double A[2];
  alignas(16) double B[2];
  store2d(A, a);
  store2d(B, b);
  if(x) B[0] = A[0];
  if(y) B[1] = A[1];
  return load2d(B);
}
inline d128 round2d(const d128 a)
{
  alignas(16) double A[2];
  store2d(A, a);
  A[0] = std::round(A[0]);
  A[1] = std::round(A[1]);
  return load2d(A);
}
# else
template<bool x, bool y>
inline d128 select2d(const d128 a, const d128 b)
{
  return _mm_blend_pd(b, a, x | (y << 1));
}
inline d128 round2d(const d128 a)
{
  return _mm_round_pd(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
# endif

#else
struct f128 { float v[4]; };
struct i128 { 
  union {
    int64_t i64[2]; 
    int32_t i32[4]; 
    int16_t i16[8]; 
    int8_t i8[16]; 
    uint64_t u64[2]; 
    uint32_t u32[4]; 
    uint16_t u16[8]; 
    uint8_t u8[16]; 
  } v;
};
struct d128 { double v[2]; };

# define shiftBytesLeft(reg, count) _mm_slli_si128(reg, count)
# define shiftBytesRight(reg, count) _mm_srli_si128(reg, count)

template<int X, int Y> inline d128 permute2d(const d128 a) { return d128 { a.v[X], a.v[Y] }; }
template<int X, int Y, int Z, int W> inline f128 permute4f(const f128 a) { return f128 { a.v[X], a.v[Y], a.v[Z], a.v[W] }; }
template<int X, int Y, int Z, int W> inline f128 shuffle4f(const f128 a, const f128 b) { return f128 { a.v[X], a.v[Y], b.v[Z], b.v[W] }; }

template<int numBits> inline i128 lshift32(i128 X) { for(int i = 0; i < 4; ++i) { X.v.u32[i] = X.v.u32[i] << numBits; } return X; }
template<int numBits> inline i128 rshift32(i128 X) { for(int i = 0; i < 4; ++i) { X.v.u32[i] = X.v.u32[i] >> numBits; } return X; }
template<int numBits> inline i128 rshift32i(i128 X) { for(int i = 0; i < 4; ++i) { X.v.i32[i] = X.v.i32[i] >> numBits; } return X; }
template<int numBits> inline i128 lshift64(i128 X) { for(int i = 0; i < 2; ++i) { X.v.u64[i] = X.v.u64[i] << numBits; } return X; }
template<int numBits> inline i128 rshift64(i128 X) { for(int i = 0; i < 2; ++i) { X.v.u64[i] = X.v.u64[i] >> numBits; } return X; }
template<int numBits> inline i128 rshift64i(i128 X) { for(int i = 0; i < 2; ++i) { X.v.i64[i] = X.v.i64[i] >> numBits; } return X; }

template<int numBytes> i128 lbyte_shift(i128 X)
{
  for(int i = 7; i >= numBytes; --i) { X.v.i32[i] = X.v.i32[i - numBytes]; }
  for(int i = 0; i < numBytes; ++i) { X.v.i32[i] = 0; } 
  return X;
}
template<int numBytes> i128 rbyte_shift(i128 X)
{
  for(int i = 0; i < numBytes; ++i) { X.v.i32[i] = X.v.i32[i + numBytes]; }
  for(int i = 7; i > numBytes; --i) { X.v.i32[i] = 0; } 
  return X;
}

inline d128 zero2d() { return d128{0, 0}; }
inline f128 zero4f() { return f128{0, 0, 0, 0}; }
inline i128 zero4i() { return i128{0, 0}; }

inline double cvtsd(const d128 x) { return x.v[0]; }
inline float cvtss(const f128 x) { return x.v[0]; }

inline f128 cast4f(const d128 reg) {
  f128 r;
  std::memcpy(&r, &reg, sizeof(f128));
  return r; 
}
inline f128 cast4f(const i128 reg) {
  f128 r;
  std::memcpy(&r, &reg, sizeof(f128));
  return r; 
}
inline i128 cast4i(const d128 reg) {
  i128 r;
  std::memcpy(&r, &reg, sizeof(i128));
  return r; 
}
inline i128 cast4i(const f128 reg) {
  i128 r;
  std::memcpy(&r, &reg, sizeof(i128));
  return r; 
}
inline d128 cast2d(const f128 reg) {
  d128 r;
  std::memcpy(&r, &reg, sizeof(d128));
  return r; 
}
inline d128 cast2d(const i128 reg) {
  d128 r;
  std::memcpy(&r, &reg, sizeof(d128));
  return r; 
}
inline f128 load1f(const void* const ptr) { const float* fp = (const float*)ptr; f128 f; f.v[0] = fp[0]; f.v[3] = f.v[2] = f.v[1] = 0; return f; }
inline f128 load2f(const void* const ptr) { const float* fp = (const float*)ptr; f128 f; f.v[0] = fp[0]; f.v[1] = fp[1]; f.v[3] = f.v[2] = 0; return f; }
inline i128 load2i(const void* const ptr) { const int* fp = (const int*)ptr; i128 f; f.v.i32[0] = fp[0]; f.v.i32[1] = fp[1]; f.v.i32[3] = f.v.i32[2] = 0; return f; }

inline int32_t movemask16i8(const i128 reg) { 
  return ((reg.v.i8[0] >> 7) & 0x01) | 
         ((reg.v.i8[1] >> 6) & 0x02) | 
         ((reg.v.i8[2] >> 5) & 0x04) | 
         ((reg.v.i8[3] >> 4) & 0x08) |
         ((reg.v.i8[4] >> 3) & 0x10) | 
         ((reg.v.i8[5] >> 2) & 0x20) | 
         ((reg.v.i8[6] >> 1) & 0x40) | 
         ((reg.v.i8[7] >> 0) & 0x80) |
         ((reg.v.i8[8] << 1) & 0x0100) | 
         ((reg.v.i8[9] << 2) & 0x0200) | 
         ((reg.v.i8[10] << 3) & 0x0400) | 
         ((reg.v.i8[11] << 4) & 0x0800) |
         ((reg.v.i8[12] << 5) & 0x1000) | 
         ((reg.v.i8[13] << 6) & 0x2000) | 
         ((reg.v.i8[14] << 7) & 0x4000) | 
         ((reg.v.i8[15] << 8) & 0x8000);
}
inline int32_t movemask4i(const i128 reg) { 
  return ((reg.v.i32[0] >> 31) & 0x1) | 
         ((reg.v.i32[1] >> 30) & 0x2) | 
         ((reg.v.i32[2] >> 29) & 0x4) | 
         ((reg.v.i32[3] >> 28) & 0x8);
}
inline int32_t movemask4f(const f128 reg) { 
  const uint32_t* ptr = (const uint32_t*)reg.v;
  return ((ptr[0] >> 31) & 0x1) | 
         ((ptr[1] >> 30) & 0x2) | 
         ((ptr[2] >> 29) & 0x4) | 
         ((ptr[3] >> 28) & 0x8);
}
inline int32_t movemask2d(const d128 reg) { 
  const uint64_t* ptr = (const uint64_t*)reg.v;
  return ((ptr[0] >> 63) & 0x1) | 
         ((ptr[1] >> 62) & 0x2);
}
inline int32_t movemask2i64(const i128 reg) { 
  return ((reg.v.u64[0] >> 63) & 0x1) | 
         ((reg.v.u64[1] >> 62) & 0x2);
}

inline i128 cmpeq4i(const i128 a, const i128 b) {
  i128 r;
  for(int i = 0; i < 4; ++i)
    r.v.i32[i] = a.v.i32[i] == b.v.i32[i] ? 0xFFFFFFFF : 0;
  return r;
}
inline i128 cmpeq16i8(const i128 a, const i128 b) {
  i128 r;
  for(int i = 0; i < 16; ++i)
    r.v.i8[i] = a.v.i8[i] == b.v.i8[i] ? 0xFF : 0x00;
  return r;
}
inline i128 cmplt16i8(const i128 a, const i128 b) {
  i128 r;
  for(int i = 0; i < 16; ++i)
    r.v.i8[i] = a.v.i8[i] < b.v.i8[i] ? 0xFF : 0x00;
  return r;
}
inline i128 cmpgt16i8(const i128 a, const i128 b) {
  i128 r;
  for(int i = 0; i < 16; ++i)
    r.v.i8[i] = a.v.i8[i] > b.v.i8[i] ? 0xFF : 0x00;
  return r;
}

inline f128 cmpgt4f(const f128 a, const f128 b) {
  const union {
    uint32_t i;
    float f;
  } _true = { 0xFFFFFFFF };
  const float _false = 0;
  return f128 {
    a.v[0] > b.v[0] ? _true.f : _false,
    a.v[1] > b.v[1] ? _true.f : _false,
    a.v[2] > b.v[2] ? _true.f : _false,
    a.v[3] > b.v[3] ? _true.f : _false
  };
}
inline d128 cmpgt2d(const d128 a, const d128 b) {
  const union {
    uint64_t i;
    double f;
  } _true = { 0xFFFFFFFFFFFFFFFFULL };
  const double _false = 0;
  return d128 {
    a.v[0] > b.v[0] ? _true.f : _false,
    a.v[1] > b.v[1] ? _true.f : _false
  };
}
inline f128 cmpne4f(const f128 a, const f128 b) {
  const union {
    uint32_t i;
    float f;
  } _true = { 0xFFFFFFFF };
  const float _false = 0;
  return f128 {
    a.v[0] != b.v[0] ? _true.f : _false,
    a.v[1] != b.v[1] ? _true.f : _false,
    a.v[2] != b.v[2] ? _true.f : _false,
    a.v[3] != b.v[3] ? _true.f : _false
  };
}
inline d128 cmpne2d(const d128 a, const d128 b) {
  const union {
    uint64_t i;
    double f;
  } _true = { 0xFFFFFFFFFFFFFFFFULL };
  const double _false = 0;
  return d128 {
    a.v[0] != b.v[0] ? _true.f : _false,
    a.v[1] != b.v[1] ? _true.f : _false
  };
}
inline i128 cmpeq8i16(const i128 a, const i128 b) {
  i128 r;
  for(int i = 0; i < 8; ++i)
    r.v.i16[i] = a.v.i16[i] == b.v.i16[i] ? 0xFFFF : 0;
  return r;
}
inline i128 cmpeq2i64(const i128 a, const i128 b) {
  i128 r;
  for(int i = 0; i < 2; ++i)
    r.v.i64[i] = a.v.i64[i] == b.v.i64[i] ? 0xFFFFFFFFFFFFFFFFULL : 0;
  return r;
}

inline f128 set4f(const float a, const float b, const float c, const float d) {
  return f128 {a, b, c, d};
}
inline i128 set4i(const int32_t a, const int32_t b, const int32_t c, const int32_t d) {
  i128 r;
  r.v.i32[0] = a;
  r.v.i32[1] = b;
  r.v.i32[2] = c;
  r.v.i32[3] = d;
  return r;
}
inline d128 set2d(const double a, const double b) {
  return d128 {a, b};
}

inline i128 set16i8(
    const int8_t a0, const int8_t b0, const int8_t c0, const int8_t d0,
    const int8_t a1, const int8_t b1, const int8_t c1, const int8_t d1,
    const int8_t a2, const int8_t b2, const int8_t c2, const int8_t d2,
    const int8_t a3, const int8_t b3, const int8_t c3, const int8_t d3) {
  i128 r;
  r.v.i8[0]  = a0;
  r.v.i8[1]  = b0;
  r.v.i8[2]  = c0;
  r.v.i8[3]  = d0;
  r.v.i8[4]  = a1;
  r.v.i8[5]  = b1;
  r.v.i8[6]  = c1;
  r.v.i8[7]  = d1;
  r.v.i8[8]  = a2;
  r.v.i8[9]  = b2;
  r.v.i8[10] = c2;
  r.v.i8[11] = d2;
  r.v.i8[12] = a3;
  r.v.i8[13] = b3;
  r.v.i8[14] = c3;
  r.v.i8[15] = d3;
  return r;
}

inline f128 loadu4f(const void* const ptr) {
  f128 r;
  std::memcpy(&r, ptr, sizeof(f128));
  return r;
}
inline i128 loadu4i(const void* const ptr) {
  i128 r;
  std::memcpy(&r, ptr, sizeof(i128));
  return r;
}
inline d128 loadu2d(const void* const ptr) {
  d128 r;
  std::memcpy(&r, ptr, sizeof(d128));
  return r;
}

inline f128 load4f(const void* const ptr) { 
  return loadu4f(ptr);
}
inline i128 load4i(const void* const ptr) {
  return loadu4i(ptr);
}
inline d128 load2d(const void* const ptr) {
  return loadu2d(ptr);
}

inline void storeu4f(void* const ptr, const f128 reg) {
  std::memcpy(ptr, &reg, sizeof(f128));
}
inline void storeu4i(void* const ptr, const i128 reg) {
  std::memcpy(ptr, &reg, sizeof(f128));
}
inline void storeu2d(void* const ptr, const d128 reg) {
  std::memcpy(ptr, &reg, sizeof(f128));
}

inline void store4f(void* const ptr, const f128 reg) {
  storeu4f(ptr, reg);
}
inline void store4i(void* const ptr, const i128 reg) { 
  storeu4i(ptr, reg);
}
inline void store2d(void* const ptr, const d128 reg) {
  storeu2d(ptr, reg);
}

inline d128 cvt2f_to_2d(const f128 reg) { 
  return d128 {reg.v[0], reg.v[1]};
}
inline f128 cvt2d_to_2f(const d128 reg) { 
  return f128 {float(reg.v[0]), float(reg.v[1]), 0, 0};
}

inline f128 movehl4f(f128 a, const f128 b) { return f128{b.v[2], b.v[3], a.v[2], a.v[3]}; }
inline f128 movelh4f(f128 a, const f128 b) { return f128{a.v[0], a.v[1], b.v[0], b.v[1]}; }
inline i128 movehl4i(i128 a, const i128 b) { 
  i128 r;
  r.v.i32[0] = b.v.i32[2];
  r.v.i32[1] = b.v.i32[3];
  r.v.i32[2] = a.v.i32[2];
  r.v.i32[3] = a.v.i32[3];
  return r;
}
inline i128 movelh4i(i128 a, const i128 b) { 
  i128 r;
  r.v.i32[0] = a.v.i32[0];
  r.v.i32[1] = a.v.i32[1];
  r.v.i32[2] = b.v.i32[0];
  r.v.i32[3] = b.v.i32[1];
  return r;
}

inline d128 xor2d(d128 a, const d128 b) { 
  uint64_t* pa = (uint64_t*)a.v;
  const uint64_t* pb = (const uint64_t*)b.v;
  for(int i = 0; i < 2; ++i)
    pa[i] = pa[i] ^ pb[i]; 
  return a; 
}
inline f128 xor4f(f128 a, const f128 b) {
  uint32_t* pa = (uint32_t*)a.v;
  const uint32_t* pb = (const uint32_t*)b.v;
  for(int i = 0; i < 4; ++i)
    pa[i] = pa[i] ^ pb[i]; 
  return a; 
}
inline i128 xor4i(i128 a, const i128 b) {
  for(int i = 0; i < 4; ++i)
    a.v.u32[i] = a.v.u32[i] ^ b.v.u32[i]; 
  return a; 
}

inline d128 or2d(d128 a, const d128 b) { 
  uint64_t* pa = (uint64_t*)a.v;
  const uint64_t* pb = (const uint64_t*)b.v;
  for(int i = 0; i < 2; ++i)
  {
    pa[i] = pa[i] | pb[i]; 
  }
  return a; 
}
inline f128 or4f(f128 a, const f128 b) {
  uint32_t* pa = (uint32_t*)a.v;
  const uint32_t* pb = (const uint32_t*)b.v;
  for(int i = 0; i < 4; ++i)
    pa[i] = pa[i] | pb[i]; 
  return a; 
}
inline i128 or4i(i128 a, const i128 b) {
  for(int i = 0; i < 4; ++i)
    a.v.u32[i] = a.v.u32[i] | b.v.u32[i]; 
  return a; 
}

inline d128 and2d(d128 a, const d128 b) { 
  uint64_t* pa = (uint64_t*)a.v;
  const uint64_t* pb = (const uint64_t*)b.v;
  for(int i = 0; i < 2; ++i)
    pa[i] = pa[i] & pb[i]; 
  return a; 
}
inline f128 and4f(f128 a, const f128 b) { 
  uint32_t* pa = (uint32_t*)a.v;
  const uint32_t* pb = (const uint32_t*)b.v;
  for(int i = 0; i < 4; ++i)
    pa[i] = pa[i] & pb[i]; 
  return a; 
}
inline i128 and4i(i128 a, const i128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v.u32[i] = a.v.u32[i] & b.v.u32[i]; 
  return a; 
}

inline d128 andnot2d(d128 a, const d128 b) { 
  uint64_t* pa = (uint64_t*)a.v;
  const uint64_t* pb = (const uint64_t*)b.v;
  for(int i = 0; i < 2; ++i)
    pa[i] = (~pa[i]) & pb[i]; 
  return a; 
}
inline f128 andnot4f(f128 a, const f128 b) { 
  uint32_t* pa = (uint32_t*)a.v;
  const uint32_t* pb = (const uint32_t*)b.v;
  for(int i = 0; i < 4; ++i)
    pa[i] = (~pa[i]) & pb[i]; 
  return a; 
}
inline i128 andnot4i(i128 a, const i128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v.u32[i] = (~a.v.u32[i]) & b.v.u32[i]; 
  return a; 
}

inline f128 mul4f(f128 a, const f128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v[i] *= b.v[i]; 
  return a; 
}
inline d128 mul2d(d128 a, const d128 b) { 
  for(int i = 0; i < 2; ++i)
    a.v[i] *= b.v[i]; 
  return a; 
}

inline f128 div4f(f128 a, const f128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v[i] /= b.v[i]; 
  return a; 
}
inline d128 div2d(d128 a, const d128 b) { 
  for(int i = 0; i < 2; ++i)
    a.v[i] /= b.v[i]; 
  return a; 
}

inline d128 add2d(d128 a, const d128 b) { 
  for(int i = 0; i < 2; ++i)
    a.v[i] += b.v[i]; 
  return a; 
}
inline f128 add4f(f128 a, const f128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v[i] += b.v[i]; 
  return a; 
}
inline i128 add4i(i128 a, const i128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v.i32[i] += b.v.i32[i]; 
  return a; 
}
inline i128 add2i64(i128 a, const i128 b) { 
  for(int i = 0; i < 2; ++i)
    a.v.i64[i] += b.v.i64[i]; 
  return a; 
}

inline d128 sub2d(d128 a, const d128 b) { 
  for(int i = 0; i < 2; ++i)
    a.v[i] -= b.v[i]; 
  return a; 
}
inline f128 sub4f(f128 a, const f128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v[i] -= b.v[i]; 
  return a; 
}
inline i128 sub4i(i128 a, const i128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v.i32[i] -= b.v.i32[i]; 
  return a; 
}
inline i128 sub2i64(i128 a, const i128 b) { 
  for(int i = 0; i < 4; ++i)
    a.v.i64[i] -= b.v.i64[i]; 
  return a; 
}

inline d128 splat2d(double f) { return d128{f, f}; }
inline f128 splat4f(float f) { return f128{f, f, f, f}; }
inline i128 splat4i(int32_t f) { i128 r; r.v.i32[3] = r.v.i32[2] = r.v.i32[1] = r.v.i32[0] = f; return r; }
inline i128 splat2i64(const int64_t f) { i128 r; r.v.i64[1] = r.v.i64[0] = f; return r; }

inline f128 unpacklo4f(const f128 a, const f128 b) { return f128{a.v[0], b.v[0], a.v[1], b.v[1]}; }
inline f128 unpackhi4f(const f128 a, const f128 b) { return f128{a.v[2], b.v[2], a.v[3], b.v[3]}; }
inline d128 unpacklo2d(const d128 a, const d128 b) { return d128{a.v[0], b.v[0]}; }
inline d128 unpackhi2d(const d128 a, const d128 b) { return d128{a.v[1], b.v[1]}; }

template<bool x, bool y>
inline d128 select2d(const d128 a, const d128 b)
{
  alignas(16) double A[2];
  alignas(16) double B[2];
  store2d(A, a);
  store2d(B, b);
  if(x) B[0] = A[0];
  if(y) B[1] = A[1];
  return load2d(B);
}

inline f128 select4f(const f128 falseResult, const f128 trueResult, const f128 cmp) { 
  f128 r;
  const uint32_t* ptr = (const uint32_t*)cmp.v;
  for(int i = 0; i < 4; ++i) 
  {
    r.v[i] = ptr[i] & 0x80000000 ? trueResult.v[i] : falseResult.v[i];
  }
  return r;
}
inline d128 select2d(const d128 falseResult, const d128 trueResult, const d128 cmp) { 
  d128 r;
  const uint64_t* ptr = (const uint64_t*)cmp.v;
  for(int i = 0; i < 2; ++i) 
  {
    r.v[i] = ptr[i] & 0x8000000000000000ULL ? trueResult.v[i] : falseResult.v[i];
  }
  return r;
}

inline d128 abs2d(d128 v) { for(int i = 0; i < 2; ++i) { v.v[i] = std::abs(v.v[i]); } return v; }
inline f128 abs4f(f128 v) { for(int i = 0; i < 4; ++i) { v.v[i] = std::abs(v.v[i]); } return v; }
inline d128 sqrt2d(d128 v) { for(int i = 0; i < 2; ++i) { v.v[i] = std::sqrt(v.v[i]); } return v; }
inline f128 sqrt4f(f128 v) { for(int i = 0; i < 4; ++i) { v.v[i] = std::sqrt(v.v[i]); } return v; }

inline d128 round2d(d128 x)
{
  x.v[0] = std::round(x.v[0]);
  x.v[1] = std::round(x.v[1]);
  return x;
}
#endif


#if defined(__AVX2__)
typedef __m256 f256;
typedef __m256i i256;
typedef __m256d d256;

template<uint32_t X, uint32_t Y, uint32_t Z, uint32_t W>
inline f256 shuffle8f(const f256 a, const f256 b) { return _mm256_shuffle_ps(a, b, _MM_SHUFFLE(W, Z, Y, X)); }

inline f256 zero8f() { return _mm256_setzero_ps(); }
inline i256 zero8i() { return _mm256_setzero_si256(); }
inline d256 zero4d() { return _mm256_setzero_pd(); }

inline f256 cast8f(const d256 reg) { return _mm256_castpd_ps(reg); }
inline f256 cast8f(const i256 reg) { return _mm256_castsi256_ps(reg); }
inline f256 cast8f(const f128 reg) { return _mm256_castps128_ps256(reg); }
inline i256 cast8i(const d256 reg) { return _mm256_castpd_si256(reg); }
inline i256 cast8i(const f256 reg) { return _mm256_castps_si256(reg); }
inline i256 cast8i(const i128 reg) { return _mm256_castsi128_si256(reg); }
inline d128 cast2d(const d256 reg) { return _mm256_castpd256_pd128(reg); }
inline d256 cast4d(const f256 reg) { return _mm256_castps_pd(reg); }
inline d256 cast4d(const f128 reg) { return cast4d(cast8f(reg)); }
inline d256 cast4d(const i256 reg) { return _mm256_castsi256_pd(reg); }
inline d256 cast4d(const i128 reg) { return cast4d(cast8i(reg)); }
inline d256 cast4d(const d128 reg) { return _mm256_castpd128_pd256(reg); }
inline f128 cast4f(const f256 reg) { return _mm256_castps256_ps128(reg); }

inline int32_t movemask32i8(const i256 reg) { return _mm256_movemask_epi8(reg); }
inline int32_t movemask8i(const i256 reg) { return _mm256_movemask_ps(cast8f(reg)); }
inline int32_t movemask8f(const f256 reg) { return _mm256_movemask_ps(reg); }
inline int32_t movemask4d(const d256 reg) { return _mm256_movemask_pd(reg); }

inline i256 cmpeq8i(const i256 a, const i256 b) { return _mm256_cmpeq_epi32(a, b); }

template<uint32_t mask> 
inline f256 permute2f128(const f256 a, const f256 b) { return _mm256_permute2f128_ps(a, b, mask); }

template<uint8_t X, uint8_t Y>
inline f256 permute128f(const f256 a, const f256 b) { return _mm256_permute2f128_ps(a, b, X | (Y << 4)); }

inline d256 set4d(const d128 a, const d128 b) { return _mm256_insertf128_pd(_mm256_castpd128_pd256(a), b, 1); }
inline f256 set8f(const f128 a, const f128 b) { return _mm256_insertf128_ps(_mm256_castps128_ps256(a), b, 1); }
inline i256 set8i(const i128 a, const i128 b) { return _mm256_inserti128_si256(_mm256_castsi128_si256(a), b, 1); }

inline d256 set4d(const double a, const double b, const double c, const double d) { return _mm256_setr_pd(a, b, c, d); }
inline f256 set8f(const float a, const float b, const float c, const float d,
                                  const float e, const float f, const float g, const float h)
  {return _mm256_setr_ps(a,b,c,d,e,f,g,h); }
inline i256 set8i(const int32_t a, const int32_t b, const int32_t c, const int32_t d,
                                const int32_t e, const int32_t f, const int32_t g, const int32_t h)
  {return _mm256_setr_epi32(a,b,c,d,e,f,g,h); }

inline f256 loadu8f(const void* const ptr) { return _mm256_loadu_ps((const float*)ptr); }
inline i256 loadu8i(const void* const ptr) { return _mm256_loadu_si256((const i256*)ptr); }
inline d256 loadu4d(const void* const ptr) { return _mm256_loadu_pd((const double*)ptr); }

inline f256 load8f(const void* const ptr) { return _mm256_load_ps((const float*)ptr); }
inline i256 load8i(const void* const ptr) { return _mm256_load_si256((const i256*)ptr); }
inline d256 load4d(const void* const ptr) { return _mm256_load_pd((const double*)ptr); }

inline void storeu8f(void* const ptr, const f256 reg) { _mm256_storeu_ps((float*)ptr, reg); }
inline void storeu8i(void* const ptr, const i256 reg) { _mm256_storeu_si256((i256*)ptr, reg); }
inline void storeu4d(void* const ptr, const d256 reg) { _mm256_storeu_pd((double*)ptr, reg); }

inline void store8f(void* const ptr, const f256 reg) { _mm256_store_ps((float*)ptr, reg); }
inline void store8i(void* const ptr, const i256 reg) { _mm256_store_si256((i256*)ptr, reg); }
inline void store4d(void* const ptr, const d256 reg) { _mm256_store_pd((double*)ptr, reg); }

inline d256 cvt4f_to_4d(const f128 reg) { return _mm256_cvtps_pd(reg); }
inline f128 cvt4d_to_4f(const d256 reg) { return _mm256_cvtpd_ps(reg); }
inline i256 cvt4i32_to_4i64(const i128 reg) { return _mm256_cvtepi32_epi64(reg); }

inline d256 xor4d(const d256 a, const d256 b) { return _mm256_xor_pd(a, b); }
inline d256 or4d(const d256 a, const d256 b) { return _mm256_or_pd(a, b); }
inline f256 or8f(const f256 a, const f256 b) { return _mm256_or_ps(a, b); }
inline f256 and8f(const f256 a, const f256 b) { return _mm256_and_ps(a, b); }
inline f256 andnot8f(const f256 a, const f256 b) { return _mm256_andnot_ps(a, b); }

inline i256 or8i(const i256 a, const i256 b) { return _mm256_or_si256(a, b); }
inline i256 and8i(const i256 a, const i256 b) { return _mm256_and_si256(a, b); }
inline i256 andnot8i(const i256 a, const i256 b) { return _mm256_andnot_si256(a, b); }

inline f256 mul8f(const f256 a, const f256 b) { return _mm256_mul_ps(a, b); }
inline d256 mul4d(const d256 a, const d256 b) { return _mm256_mul_pd(a, b); }
inline f256 div8f(const f256 a, const f256 b) { return _mm256_div_ps(a, b); }
inline d256 div4d(const d256 a, const d256 b) { return _mm256_div_pd(a, b); }

inline f256 add8f(const f256 a, const f256 b) { return _mm256_add_ps(a, b); }
inline i256 add8i(const i256 a, const i256 b) { return _mm256_add_epi32(a, b); }
inline d256 add4d(const d256 a, const d256 b) { return _mm256_add_pd(a, b); }
inline i256 add4i64(const i256 a, const i256 b) { return _mm256_add_epi64(a, b); }

inline f256 sub8f(const f256 a, const f256 b) { return _mm256_sub_ps(a, b); }
inline i256 sub8i(const i256 a, const i256 b) { return _mm256_sub_epi32(a, b); }
inline d256 sub4d(const d256 a, const d256 b) { return _mm256_sub_pd(a, b); }
inline i256 sub4i64(const i256 a, const i256 b) { return _mm256_sub_epi64(a, b); }

inline f256 select8f(const f256 falseResult, const f256 trueResult, const f256 cmp) { return _mm256_blendv_ps(falseResult, trueResult, cmp); }

template<bool x, bool y, bool z, bool w>
inline d256 select4d(const d256 a, const d256 b)
{
  return _mm256_blend_pd(b, a, x | (y<<1) | (z<<2) | (w<<3));
}

inline f256 permutevar8x32f(const f256 a, const i256 b) { return _mm256_permutevar8x32_ps(a, b); }

inline f256 unpacklo8f(const f256 a, const f256 b) { return _mm256_unpacklo_ps(a, b); }
inline f256 unpackhi8f(const f256 a, const f256 b) { return _mm256_unpackhi_ps(a, b); }

template<uint32_t index>
inline f128 extract4f(const f256 reg) { return _mm256_extractf128_ps(reg, index); }
template<uint32_t index>
inline d128 extract2d(const d256 reg) { return _mm256_extractf128_pd(reg, index); }
template<uint32_t index>
inline i128 extract256i64(const i256 reg) { return _mm256_extract_epi64(reg, index); }

inline f256 splat8f(const float f) { return _mm256_set1_ps(f); }
inline d256 splat4d(const double f) { return _mm256_set1_pd(f); }
inline i256 splat8i(const int32_t f) { return _mm256_set1_epi32(f); }
inline i256 splat4i64(const int64_t f) { return _mm256_set1_epi64x(f); }

inline f128 i32gather4f(const float* const ptr, const i128 indices) { return _mm_i32gather_ps(ptr, indices, 4); }
inline f256 i32gather8f(const float* const ptr, const i256 indices) { return _mm256_i32gather_ps(ptr, indices, 4); }
inline i128 i32gather4i(const int32_t* const ptr, const i128 indices) { return _mm_i32gather_epi32(ptr, indices, 4); }
inline i256 i32gather8i(const int32_t* const ptr, const i256 indices) { return _mm256_i32gather_epi32(ptr, indices, 4); }

inline f256 set2f128(const f128 lo, const f128 hi) { return _mm256_insertf128_ps(_mm256_castps128_ps256(lo), hi, 1); }

inline f256 cmpgt8f(const f256 a, const f256 b) { return _mm256_cmp_ps(a, b, _CMP_GT_OQ); }
inline d256 cmpgt4d(const d256 a, const d256 b) { return _mm256_cmp_pd(a, b, _CMP_GT_OQ); }
inline f256 cmpne8f(const f256 a, const f256 b) { return _mm256_cmp_ps(a, b, _CMP_NEQ_OQ); }
inline d256 cmpne4d(const d256 a, const d256 b) { return _mm256_cmp_pd(a, b, _CMP_NEQ_OQ); }
inline i256 cmpeq4i64(const i256 a, const i256 b) { return _mm256_cmpeq_epi64(a, b); }
inline i256 cmpeq16i16(const i256 a, const i256 b) { return _mm256_cmpeq_epi16(a, b); }
inline i256 cmpeq32i8(const i256 a, const i256 b) { return _mm256_cmpeq_epi8(a, b); }

inline f256 abs8f(const f256 v) { return _mm256_andnot_ps(splat8f(-0.0f), v); }
inline d256 abs4d(const d256 v) { return _mm256_andnot_pd(splat4d(-0.0), v); }

inline f256 sqrt8f(const f256 a) { return _mm256_sqrt_ps(a); }
inline d256 sqrt4d(const d256 a) { return _mm256_sqrt_pd(a); }

/// \brief  loads up to 7 floating point values from ptr, and sets the other elements to zero.
inline f256 loadmask7f(const void* const ptr, const size_t count)
{
  // mask_offset = 7 - (count % 8)
  //
  // This gives us an index into the array of masks which we can pass into
  // _mm256_maskload_ps later on.
  const size_t mask_offset = (~count) & 0x7;
  const int32_t masks[] = {
    -1, -1, -1, -1,
    -1, -1, -1, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
  };
  const i256 loadmask = loadu8i(masks + mask_offset);
  return _mm256_maskload_ps((const float*)ptr, loadmask);
}

/// \brief  loads up to 7 integer values from ptr, and sets the other elements to zero.
inline i256 loadmask7i(const void* const ptr, const size_t count)
{
    return cast8i(loadmask7f(ptr, count));
}

inline d256 loadmask3d(const void* const ptr, const size_t count)
{
  // mask_offset = 3 - (count % 3)
  //
  // This gives us an index into the array of masks which we can pass into
  // _mm256_maskload_ps later on.
  const size_t mask_offset = (~count) & 0x3;
  const int64_t masks[] = {
    -1, -1, -1, 0,
    0, 0, 0, 0
  };
  const i256 loadmask = loadu8i(masks + mask_offset);
  return _mm256_maskload_pd((const double*)ptr, loadmask);
}
inline i256 loadmask3i64(const void* const ptr, const size_t count)
{
  return cast8i(loadmask3d(ptr, count));
}

template<int X, int Y, int Z, int W>
inline d256 permute4d(const d256 a)
{
    return _mm256_permute4x64_pd(a, X | (Y << 2) | (Z << 4) | (W << 6));
}

inline d256 round4d(const d256 x)
{
  return _mm256_round_pd(x, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

template<uint32_t index> double get(const d256 reg);
template<> inline double get<0>(const d256 reg) { return cvtsd(cast2d(reg)); }
template<> inline double get<1>(const d256 reg) { return cvtsd(unpackhi2d(cast2d(reg), cast2d(reg))); }
template<> inline double get<2>(const d256 reg) { return cvtsd(extract2d<1>(reg)); }
template<> inline double get<3>(const d256 reg) { return cvtsd(unpackhi2d(extract2d<1>(reg), extract2d<1>(reg))); }

#else
struct f256 { f128 a, b; };
struct d256 { d128 a, b; };
struct i256 { i128 a, b; };

template<uint32_t X, uint32_t Y, uint32_t Z, uint32_t W>
inline f256 shuffle8f(const f256 a, const f256 b) { return f256 { shuffle4f<X, Y, Z, W>(a.a, b.a), shuffle4f<X, Y, Z, W>(a.b, b.b) }; }

inline f256 zero8f() { return f256 {zero4f(), zero4f()}; }
inline i256 zero8i() { return i256 {zero4i(), zero4i()}; }
inline d256 zero4d() { return d256 {zero2d(), zero2d()}; }

inline int32_t movemask32i8(const i256 reg) { return movemask16i8(reg.a) | (movemask16i8(reg.b) << 16); } 
inline int32_t movemask8i(const i256 reg) { return movemask4i(reg.a) | (movemask4i(reg.b) << 4); }
inline int32_t movemask8f(const f256 reg) { return movemask4f(reg.a) | (movemask4f(reg.b) << 4); }
inline int32_t movemask4d(const d256 reg) { return movemask2d(reg.a) | (movemask2d(reg.b) << 2); }

inline i256 cmpeq8i(const i256 a, const i256 b) { return i256 {cmpeq4i(a.a, b.a), cmpeq4i(a.b, b.b)}; }

template<uint32_t mask> 
inline f256 permute2f128(const f256 a, const f256 b) { 
  f256 R;
  switch(mask & 0xF)
  {
  case 0: R.a = a.a; break;
  case 1: R.a = a.b; break;
  case 2: R.a = b.a; break;
  case 3: R.a = b.b; break;
  }
  switch((mask >> 4) & 0xF)
  {
  case 0: R.b = a.a; break;
  case 1: R.b = a.b; break;
  case 2: R.b = b.a; break;
  case 3: R.b = b.b; break;
  }
  return R;
}

template<uint8_t X, uint8_t Y>
inline f256 permute128f(const f256 a, const f256 b) { 
  f256 R;
  switch(X)
  {
  case 0: R.a = a.a; break;
  case 1: R.a = a.b; break;
  case 2: R.a = b.a; break;
  case 3: R.a = b.b; break;
  }
  switch(Y)
  {
  case 0: R.b = a.a; break;
  case 1: R.b = a.b; break;
  case 2: R.b = b.a; break;
  case 3: R.b = b.b; break;
  }
  return R;
}

inline d256 set4d(const d128 a, const d128 b) { return d256{a, b}; }
inline f256 set8f(const f128 a, const f128 b) { return f256{a, b}; }
inline i256 set8i(const i128 a, const i128 b) { return i256{a, b}; }

inline d256 set4d(const double a, const double b, const double c, const double d) { 
  return d256{ set2d(a, b), set2d(c, d) }; 
}
inline f256 set8f(const float a, const float b, const float c, const float d,
                                  const float e, const float f, const float g, const float h) {
  return f256{ set4f(a, b, c, d), set4f(e, f, g, h) };
}
inline i256 set8i(const int32_t a, const int32_t b, const int32_t c, const int32_t d,
                                const int32_t e, const int32_t f, const int32_t g, const int32_t h) {
  return i256{ set4i(a, b, c, d), set4i(e, f, g, h) }; 
}

inline f256 loadu8f(const void* const ptr) { 
  const uint8_t* const f = (const uint8_t*)ptr;
  return f256{ loadu4f(f), loadu4f(f + 16) };
}
inline i256 loadu8i(const void* const ptr) { 
  const uint8_t* const f = (const uint8_t*)ptr;
  return i256{ loadu4i(f), loadu4i(f + 16) };
}
inline d256 loadu4d(const void* const ptr) { 
  const uint8_t* const f = (const uint8_t*)ptr;
  return d256{ loadu2d(f), loadu2d(f + 16) };
}

inline f256 load8f(const void* const ptr) { 
  const uint8_t* const f = (const uint8_t*)ptr;
  return f256{ load4f(f), load4f(f + 16) };
}
inline i256 load8i(const void* const ptr) { 
  const uint8_t* const f = (const uint8_t*)ptr;
  return i256{ load4i(f), load4i(f + 16) };
}
inline d256 load4d(const void* const ptr) { 
  const uint8_t* const f = (const uint8_t*)ptr;
  return d256{ load2d(f), load2d(f + 16) };
}

inline void storeu8f(void* const inPtr, const f256 reg) { 
  uint8_t* const ptr = (uint8_t*)inPtr;
  storeu4f(ptr, reg.a);
  storeu4f(ptr + 16, reg.b);
}
inline void storeu8i(void* const inPtr, const i256 reg) { 
  uint8_t* const ptr = (uint8_t*)inPtr;
  storeu4i(ptr, reg.a);
  storeu4i(ptr + 16, reg.b);
}
inline void storeu4d(void* const inPtr, const d256 reg) { 
  uint8_t* const ptr = (uint8_t*)inPtr;
  storeu2d(ptr, reg.a);
  storeu2d(ptr + 16, reg.b);
}

inline void store8f(void* const inPtr, const f256 reg) { 
  uint8_t* const ptr = (uint8_t*)inPtr;
  store4f(ptr, reg.a);
  store4f(ptr + 16, reg.b);
}
inline void store8i(void* const inPtr, const i256 reg) { 
  uint8_t* const ptr = (uint8_t*)inPtr;
  store4i(ptr, reg.a);
  store4i(ptr + 16, reg.b);
}
inline void store4d(void* const inPtr, const d256 reg) { 
  uint8_t* const ptr = (uint8_t*)inPtr;
  store2d(ptr, reg.a);
  store2d(ptr + 16, reg.b);
}

inline f256 cast8f(const d256 reg) { alignas(32) uint8_t f[32]; store4d(f, reg); return load8f(f); }
inline f256 cast8f(const i256 reg) { alignas(32) uint8_t f[32]; store8i(f, reg); return load8f(f); }
inline i256 cast8i(const d256 reg) { alignas(32) uint8_t f[32]; store4d(f, reg); return load8i(f); }
inline i256 cast8i(const f256 reg) { alignas(32) uint8_t f[32]; store8f(f, reg); return load8i(f); }
inline d256 cast4d(const f256 reg) { alignas(32) uint8_t f[32]; store8f(f, reg); return load4d(f); }
inline d256 cast4d(const i256 reg) { alignas(32) uint8_t f[32]; store8i(f, reg); return load4d(f); }
inline d128 cast2d(const d256 reg) { return reg.a; }
inline f128 cast4f(const f256 reg) { return reg.a; }
inline i128 cast4i(const i256 reg) { return reg.a; }
inline f256 cast8f(const f128 reg) { return f256{reg, zero4f()}; }
inline i256 cast8i(const i128 reg) { return i256{reg, zero4i()}; }
inline d256 cast4d(const d128 reg) { return d256{reg, zero2d()}; }
inline d256 cast4d(const f128 reg) { return cast4d(cast8f(reg)); }
inline d256 cast4d(const i128 reg) { return cast4d(cast8i(reg)); }

inline d256 cvt4f_to_4d(const f128 reg) { 
  alignas(16) float f[4];
  store4f(f, reg);
  alignas(32) double d[4];
  d[0] = f[0];
  d[1] = f[1];
  d[2] = f[2];
  d[3] = f[3];
  return load4d(d);
 }
inline f128 cvt4d_to_4f(const d256 reg) { 
  alignas(32) double f[4];
  store4d(f, reg);
  alignas(16) float d[4];
  d[0] = f[0];
  d[1] = f[1];
  d[2] = f[2];
  d[3] = f[3];
  return load4f(d);
}
inline i256 cvt4i32_to_4i64(const i128 reg) { 
  alignas(16) int32_t f[4];
  store4i(f, reg);
  alignas(32) int64_t d[4];
  d[0] = f[0];
  d[1] = f[1];
  d[2] = f[2];
  d[3] = f[3];
  return load8i(d);
}

inline d256 xor4d(const d256 a, const d256 b) { 
  return d256{ xor2d(a.a, b.a), xor2d(a.b, b.b) };
}
inline d256 or4d(const d256 a, const d256 b) {
  return d256{ or2d(a.a, b.a), or2d(a.b, b.b) };
}
inline f256 or8f(const f256 a, const f256 b) { 
  return f256{ or4f(a.a, b.a), or4f(a.b, b.b) };
}
inline f256 and8f(const f256 a, const f256 b) { 
  return f256{ and4f(a.a, b.a), and4f(a.b, b.b) };
}
inline f256 andnot8f(const f256 a, const f256 b) { 
  return f256{ andnot4f(a.a, b.a), andnot4f(a.b, b.b) };
}

inline i256 or8i(const i256 a, const i256 b) { 
  return i256{ or4i(a.a, b.a), or4i(a.b, b.b) };
}
inline i256 and8i(const i256 a, const i256 b) { 
  return i256{ and4i(a.a, b.a), and4i(a.b, b.b) };
}
inline i256 andnot8i(const i256 a, const i256 b) { 
  return i256{ andnot4i(a.a, b.a), andnot4i(a.b, b.b) };
}

inline f256 mul8f(const f256 a, const f256 b) { 
  return f256{ mul4f(a.a, b.a), mul4f(a.b, b.b) };
}
inline d256 mul4d(const d256 a, const d256 b) { 
  return d256{ mul2d(a.a, b.a), mul2d(a.b, b.b) };
}
inline f256 div8f(const f256 a, const f256 b) { 
  return f256{ div4f(a.a, b.a), div4f(a.b, b.b) };
}
inline d256 div4d(const d256 a, const d256 b) { 
  return d256{ div2d(a.a, b.a), div2d(a.b, b.b) };
}

inline f256 add8f(const f256 a, const f256 b) { 
  return f256{ add4f(a.a, b.a), add4f(a.b, b.b) };
}
inline i256 add8i(const i256 a, const i256 b) { 
  return i256{ add4i(a.a, b.a), add4i(a.b, b.b) };
}
inline d256 add4d(const d256 a, const d256 b) { 
  return d256{ add2d(a.a, b.a), add2d(a.b, b.b) };
}
inline i256 add4i64(const i256 a, const i256 b) { 
  return i256{ add2i64(a.a, b.a), add2i64(a.b, b.b) };
}

inline f256 sub8f(const f256 a, const f256 b) { 
  return f256{ sub4f(a.a, b.a), sub4f(a.b, b.b) };
}
inline i256 sub8i(const i256 a, const i256 b) { 
  return i256{ sub4i(a.a, b.a), sub4i(a.b, b.b) };
}
inline d256 sub4d(const d256 a, const d256 b) { 
  return d256{ sub2d(a.a, b.a), sub2d(a.b, b.b) };
}
inline i256 sub4i64(const i256 a, const i256 b) { 
  return i256{ sub2i64(a.a, b.a), sub2i64(a.b, b.b) };
}

inline f256 select8f(const f256 falseResult, const f256 trueResult, const f256 cmp) { 
  return f256{ select4f(falseResult.a, trueResult.a, cmp.a), select4f(falseResult.b, trueResult.b, cmp.b) };
}
template<bool X, bool Y, bool Z, bool W>
inline d256 select4d(const d256 a, const d256 b)
{
  return d256{ select2d<X, Y>(a.a, b.a), select2d<Z, W>(a.b, b.b) };
}

template<int X, int Y, int Z, int W>
inline d256 permute4d(const d256 a)
{
  alignas(32) double D[4];
  alignas(32) double R[4];
  store4d(D, a);
  R[0] = D[X];
  R[1] = D[Y];
  R[2] = D[Z];
  R[3] = D[W];
  return load4d(R);
}

inline f256 permutevar8x32f(const f256 a, const i256 b) {
  alignas(32) float f[8];
  alignas(32) float r[8];
  alignas(32) int32_t i[8];
  store8i(i, b);
  store8f(f, a);
  r[0] = f[i[0]];
  r[1] = f[i[1]];
  r[2] = f[i[2]];
  r[3] = f[i[3]];
  r[4] = f[i[4]];
  r[5] = f[i[5]];
  r[6] = f[i[6]];
  r[7] = f[i[7]];
   return load8f(r); 
}

inline f256 unpacklo8f(const f256 a, const f256 b) { return f256{unpacklo4f(a.a, b.a), unpacklo4f(a.a, b.a)}; }
inline f256 unpackhi8f(const f256 a, const f256 b) { return f256{unpackhi4f(a.a, b.a), unpackhi4f(a.a, b.a)}; }

template<uint32_t index> inline f128 extract4f(const f256 reg) { return reg.a; }
template<> inline f128 extract4f<1>(const f256 reg) { return reg.b; }
template<uint32_t index> inline d128 extract2d(const d256 reg) { return reg.a; }
template<> inline d128 extract2d<1>(const d256 reg) { return reg.b; }
template<uint32_t index> inline i128 extract256i64(const i256 reg) { return reg.a; }
template<> inline i128 extract256i64<1>(const i256 reg) { return reg.b; }

inline f256 splat8f(const float f) { 
  return f256{ splat4f(f), splat4f(f) };
}
inline d256 splat4d(const double f) { 
  return d256{ splat2d(f), splat2d(f) };
}
inline i256 splat8i(const int32_t f)   { 
  return i256{ splat4i(f), splat4i(f) };
}
inline i256 splat4i64(const int64_t f)  { 
  return i256{ splat2i64(f), splat2i64(f) };
}
inline f128 i32gather4f(const float* const ptr, const i128 indices) { 
  alignas(16) int i[4];
  store4i(i, indices);
  return set4f( ptr[i[0]], ptr[i[1]], ptr[i[2]], ptr[i[3]] );
}
inline f256 i32gather8f(const float* const ptr, const i256 indices) { 
  return f256{i32gather4f(ptr, indices.a), i32gather4f(ptr, indices.b)};
}
inline i128 i32gather4i(const int32_t* const ptr, const i128 indices) { 
  alignas(16) int i[4];
  store4i(i, indices);
  return set4i( ptr[i[0]], ptr[i[1]], ptr[i[2]], ptr[i[3]] );
}
inline i256 i32gather8i(const int32_t* const ptr, const i256 indices) { 
  return i256{i32gather4i(ptr, indices.a), i32gather4i(ptr, indices.b)};
}
inline f256 set2f128(const f128 lo, const f128 hi) { return f256{lo, hi}; }

inline f256 cmpgt8f(const f256 a, const f256 b) { 
  return f256{ cmpgt4f(a.a, b.a), cmpgt4f(a.b, b.b) };
}
inline d256 cmpgt4d(const d256 a, const d256 b) { 
  return d256{ cmpgt2d(a.a, b.a), cmpgt2d(a.b, b.b) };
}
inline f256 cmpne8f(const f256 a, const f256 b) { 
  return f256{ cmpne4f(a.a, b.a), cmpne4f(a.b, b.b) };
}
inline d256 cmpne4d(const d256 a, const d256 b) { 
  return d256{ cmpne2d(a.a, b.a), cmpne2d(a.b, b.b) };
}
inline i256 cmpeq4i64(const i256 a, const i256 b) { 
  return i256{ cmpeq2i64(a.a, b.a), cmpeq2i64(a.b, b.b) };
}
inline i256 cmpeq16i16(const i256 a, const i256 b) { 
  return i256{ cmpeq8i16(a.a, b.a), cmpeq8i16(a.b, b.b) };
}
inline i256 cmpeq32i8(const i256 a, const i256 b){ 
  return i256{ cmpeq16i8(a.a, b.a), cmpeq16i8(a.b, b.b) };
}

inline f256 abs8f(const f256 v) { 
  return f256{ abs4f(v.a), abs4f(v.b) };
}
inline d256 abs4d(const d256 v) { 
  return d256{ abs2d(v.a), abs2d(v.b) };
}

inline f256 sqrt8f(const f256 v) { 
  return f256{ sqrt4f(v.a), sqrt4f(v.b) };
}
inline d256 sqrt4d(const d256 v) { 
  return d256{ sqrt2d(v.a), sqrt2d(v.b) };
}

/// \brief  loads up to 7 floating point values from ptr, and sets the other elements to zero.
inline f256 loadmask7f(const void* const ptr, const size_t count)
{
  alignas(32) float f[8] = {0};
  std::memcpy(f, ptr, count * sizeof(float));
  return load8f(f);
}

/// \brief  loads up to 7 integer values from ptr, and sets the other elements to zero.
inline i256 loadmask7i(const void* const ptr, const size_t count)
{
    return cast8i(loadmask7f(ptr, count));
}

inline d256 loadmask3d(const void* const ptr, const size_t count)
{
  alignas(32) double d[4] = {0};
  std::memcpy(d, ptr, count * sizeof(double));
  return load4d(d);
}
inline i256 loadmask3i64(const void* const ptr, const size_t count)
{
  return cast8i(loadmask3d(ptr, count));
}
inline d256 round4d(const d256 v)
{
  return d256{ round2d(v.a), round2d(v.b) };
}

template<uint32_t index> double get(const d256 reg);
template<> inline double get<0>(const d256 reg) { return cvtsd(reg.a); }
template<> inline double get<1>(const d256 reg) { return cvtsd(unpackhi2d(reg.a, reg.a)); }
template<> inline double get<2>(const d256 reg) { return cvtsd(reg.b); }
template<> inline double get<3>(const d256 reg) { return cvtsd(unpackhi2d(reg.b, reg.b)); }
#endif

// If enabled, make use of the half-float F16C processor ops to convert between half <-> float
#if defined(__F16C__)
inline f256 cvtph8(const i128 a) { return _mm256_cvtph_ps(a); }
inline i128 cvtph8(const f256 a) { return _mm256_cvtps_ph(a, _MM_FROUND_CUR_DIRECTION); }
inline f128 cvtph4(const i128 a) { return _mm_cvtph_ps(a); }
inline i128 cvtph4(const f128 a) { return _mm_cvtps_ph(a, _MM_FROUND_CUR_DIRECTION); }
inline float cvtph1(const uint16_t x) { return  _cvtsh_ss(x); }
inline uint16_t cvtph1(const float x) { return  _cvtss_sh(x, _MM_FROUND_CUR_DIRECTION); }
#else
inline f256 cvtph8(const i128 a) { 
  alignas(16) GfHalf A[8];
  alignas(32) float F[8];
  store4i(A, a);
  for(int i = 0; i < 8; ++i) F[i] = A[i];
  return load8f(F);
}
inline i128 cvtph8(const f256 a) { 
  alignas(16) GfHalf A[8];
  alignas(32) float F[8];
  store8f(F, a);
  for(int i = 0; i < 8; ++i) A[i] = F[i];
  return load4i(A);
}
inline f128 cvtph4(const i128 a) {
  alignas(16) GfHalf A[8];
  alignas(16) float F[4];
  store4i(A, a);
  for(int i = 0; i < 4; ++i) F[i] = A[i];
  return load4f(F);
}
inline i128 cvtph4(const f128 a) { 
  alignas(16) GfHalf A[8];
  alignas(16) float F[4];
  store4f(F, a);
  for(int i = 0; i < 4; ++i) A[i] = F[i];
  return load4i(A);
}
inline float cvtph1(const uint16_t x) { GfHalf h; h.setBits(x); return float(h); }
inline uint16_t cvtph1(const float x)  { GfHalf h(x); return h.bits(); }
#endif

// If enabled, make use of the fused-multiply-add ops
#if defined(__FMA__)
inline d256 fmadd4d(const d256 a, const d256 b, const d256 c) { return _mm256_fmadd_pd(a, b, c); }
inline d256 fmsub4d(const d256 a, const d256 b, const d256 c) { return _mm256_fmsub_pd(a, b, c); }
inline d256 fnmadd4d(const d256 a, const d256 b, const d256 c) { return _mm256_fnmadd_pd(a, b, c); }
inline d256 fnmsub4d(const d256 a, const d256 b, const d256 c) { return _mm256_fnmsub_pd(a, b, c); }
# else
inline d256 fmadd4d(const d256 a, const d256 b, const d256 c) { return add4d(mul4d(a, b), c); }
inline d256 fmsub4d(const d256 a, const d256 b, const d256 c) { return sub4d(mul4d(a, b), c); }
inline d256 fnmadd4d(const d256 a, const d256 b, const d256 c) { return sub4d(c, mul4d(a, b)); }
inline d256 fnmsub4d(const d256 a, const d256 b, const d256 c) { return sub4d(zero4d(), add4d(c, mul4d(a, b))); }
#endif

#ifdef __AVX2__
/// \brief  loads up to 3 floating point values from ptr, and sets the other elements to zero.
inline f128 loadmask3f(const void* const ptr, const size_t count)
{
  // mask_offset = 3 - (count % 3)
  //
  // This gives us an index into the array of masks which we can pass into
  // _mm256_maskload_ps later on.
  const size_t mask_offset = (~count) & 0x3;
  const int32_t masks[] = {
    -1, -1, -1, 0,
    0, 0, 0, 0
  };
  const i128 loadmask = loadu4i(masks + mask_offset);
  return _mm_maskload_ps((const float*)ptr, loadmask);
}
#else
/// \brief  loads up to 3 floating point values from ptr, and sets the other elements to zero.
inline f128 loadmask3f(const void* const ptr, size_t count)
{
  alignas(16) float p[4] = {0};
  const float* const fp = (const float*)ptr;
  switch(count & 3)
  {
  case 3: p[2] = fp[2]; /* break; */
  case 2: p[1] = fp[1]; /* break; */
  case 1: p[0] = fp[0]; /* break; */
  default: break;
  }
  return load4f(p);
}
#endif

} // MayaUsdUtils

#endif
