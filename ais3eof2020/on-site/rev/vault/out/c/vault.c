#include <math.h>
#include <string.h>

#include "vault.h"
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)

#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#define FUNC_PROLOGUE                                            \
  if (++wasm_rt_call_stack_depth > WASM_RT_MAX_CALL_STACK_DEPTH) \
    TRAP(EXHAUSTION)

#define FUNC_EPILOGUE --wasm_rt_call_stack_depth

#define UNREACHABLE TRAP(UNREACHABLE)

#define CALL_INDIRECT(table, t, ft, x, ...)          \
  (LIKELY((x) < table.size && table.data[x].func &&  \
          table.data[x].func_type == func_types[ft]) \
       ? ((t)table.data[x].func)(__VA_ARGS__)        \
       : TRAP(CALL_INDIRECT))

#define MEMCHECK(mem, a, t)  \
  if (UNLIKELY((a) + sizeof(t) > mem->size)) TRAP(OOB)

#define DEFINE_LOAD(name, t1, t2, t3)              \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {   \
    MEMCHECK(mem, addr, t1);                       \
    t1 result;                                     \
    memcpy(&result, &mem->data[addr], sizeof(t1)); \
    return (t3)(t2)result;                         \
  }

#define DEFINE_STORE(name, t1, t2)                           \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    MEMCHECK(mem, addr, t1);                                 \
    t1 wrapped = (t1)value;                                  \
    memcpy(&mem->data[addr], &wrapped, sizeof(t1));          \
  }

DEFINE_LOAD(i32_load, u32, u32, u32);
DEFINE_LOAD(i64_load, u64, u64, u64);
DEFINE_LOAD(f32_load, f32, f32, f32);
DEFINE_LOAD(f64_load, f64, f64, f64);
DEFINE_LOAD(i32_load8_s, s8, s32, u32);
DEFINE_LOAD(i64_load8_s, s8, s64, u64);
DEFINE_LOAD(i32_load8_u, u8, u32, u32);
DEFINE_LOAD(i64_load8_u, u8, u64, u64);
DEFINE_LOAD(i32_load16_s, s16, s32, u32);
DEFINE_LOAD(i64_load16_s, s16, s64, u64);
DEFINE_LOAD(i32_load16_u, u16, u32, u32);
DEFINE_LOAD(i64_load16_u, u16, u64, u64);
DEFINE_LOAD(i64_load32_s, s32, s64, u64);
DEFINE_LOAD(i64_load32_u, u32, u64, u64);
DEFINE_STORE(i32_store, u32, u32);
DEFINE_STORE(i64_store, u64, u64);
DEFINE_STORE(f32_store, f32, f32);
DEFINE_STORE(f64_store, f64, f64);
DEFINE_STORE(i32_store8, u8, u32);
DEFINE_STORE(i32_store16, u16, u32);
DEFINE_STORE(i64_store8, u8, u64);
DEFINE_STORE(i64_store16, u16, u64);
DEFINE_STORE(i64_store32, u32, u64);

#define I32_CLZ(x) ((x) ? __builtin_clz(x) : 32)
#define I64_CLZ(x) ((x) ? __builtin_clzll(x) : 64)
#define I32_CTZ(x) ((x) ? __builtin_ctz(x) : 32)
#define I64_CTZ(x) ((x) ? __builtin_ctzll(x) : 64)
#define I32_POPCNT(x) (__builtin_popcount(x))
#define I64_POPCNT(x) (__builtin_popcountll(x))

#define DIV_S(ut, min, x, y)                                 \
   ((UNLIKELY((y) == 0)) ?                TRAP(DIV_BY_ZERO)  \
  : (UNLIKELY((x) == min && (y) == -1)) ? TRAP(INT_OVERFLOW) \
  : (ut)((x) / (y)))

#define REM_S(ut, min, x, y)                                \
   ((UNLIKELY((y) == 0)) ?                TRAP(DIV_BY_ZERO) \
  : (UNLIKELY((x) == min && (y) == -1)) ? 0                 \
  : (ut)((x) % (y)))

#define I32_DIV_S(x, y) DIV_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_DIV_S(x, y) DIV_S(u64, INT64_MIN, (s64)x, (s64)y)
#define I32_REM_S(x, y) REM_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_REM_S(x, y) REM_S(u64, INT64_MIN, (s64)x, (s64)y)

#define DIVREM_U(op, x, y) \
  ((UNLIKELY((y) == 0)) ? TRAP(DIV_BY_ZERO) : ((x) op (y)))

#define DIV_U(x, y) DIVREM_U(/, x, y)
#define REM_U(x, y) DIVREM_U(%, x, y)

#define ROTL(x, y, mask) \
  (((x) << ((y) & (mask))) | ((x) >> (((mask) - (y) + 1) & (mask))))
#define ROTR(x, y, mask) \
  (((x) >> ((y) & (mask))) | ((x) << (((mask) - (y) + 1) & (mask))))

#define I32_ROTL(x, y) ROTL(x, y, 31)
#define I64_ROTL(x, y) ROTL(x, y, 63)
#define I32_ROTR(x, y) ROTR(x, y, 31)
#define I64_ROTR(x, y) ROTR(x, y, 63)

#define FMIN(x, y)                                          \
   ((UNLIKELY((x) != (x))) ? NAN                            \
  : (UNLIKELY((y) != (y))) ? NAN                            \
  : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? x : y) \
  : (x < y) ? x : y)

#define FMAX(x, y)                                          \
   ((UNLIKELY((x) != (x))) ? NAN                            \
  : (UNLIKELY((y) != (y))) ? NAN                            \
  : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? y : x) \
  : (x > y) ? x : y)

#define TRUNC_S(ut, st, ft, min, max, maxop, x)                             \
   ((UNLIKELY((x) != (x))) ? TRAP(INVALID_CONVERSION)                       \
  : (UNLIKELY((x) < (ft)(min) || (x) maxop (ft)(max))) ? TRAP(INT_OVERFLOW) \
  : (ut)(st)(x))

#define I32_TRUNC_S_F32(x) TRUNC_S(u32, s32, f32, INT32_MIN, INT32_MAX, >=, x)
#define I64_TRUNC_S_F32(x) TRUNC_S(u64, s64, f32, INT64_MIN, INT64_MAX, >=, x)
#define I32_TRUNC_S_F64(x) TRUNC_S(u32, s32, f64, INT32_MIN, INT32_MAX, >,  x)
#define I64_TRUNC_S_F64(x) TRUNC_S(u64, s64, f64, INT64_MIN, INT64_MAX, >=, x)

#define TRUNC_U(ut, ft, max, maxop, x)                                    \
   ((UNLIKELY((x) != (x))) ? TRAP(INVALID_CONVERSION)                     \
  : (UNLIKELY((x) <= (ft)-1 || (x) maxop (ft)(max))) ? TRAP(INT_OVERFLOW) \
  : (ut)(x))

#define I32_TRUNC_U_F32(x) TRUNC_U(u32, f32, UINT32_MAX, >=, x)
#define I64_TRUNC_U_F32(x) TRUNC_U(u64, f32, UINT64_MAX, >=, x)
#define I32_TRUNC_U_F64(x) TRUNC_U(u32, f64, UINT32_MAX, >,  x)
#define I64_TRUNC_U_F64(x) TRUNC_U(u64, f64, UINT64_MAX, >=, x)

#define DEFINE_REINTERPRET(name, t1, t2)  \
  static inline t2 name(t1 x) {           \
    t2 result;                            \
    memcpy(&result, &x, sizeof(result));  \
    return result;                        \
  }

DEFINE_REINTERPRET(f32_reinterpret_i32, u32, f32)
DEFINE_REINTERPRET(i32_reinterpret_f32, f32, u32)
DEFINE_REINTERPRET(f64_reinterpret_i64, u64, f64)
DEFINE_REINTERPRET(i64_reinterpret_f64, f64, u64)


static u32 func_types[14];

static void init_func_types(void) {
  func_types[0] = wasm_rt_register_func_type(3, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[1] = wasm_rt_register_func_type(1, 1, WASM_RT_I32, WASM_RT_I32);
  func_types[2] = wasm_rt_register_func_type(1, 0, WASM_RT_I32);
  func_types[3] = wasm_rt_register_func_type(3, 0, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[4] = wasm_rt_register_func_type(2, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[5] = wasm_rt_register_func_type(2, 0, WASM_RT_I32, WASM_RT_I32);
  func_types[6] = wasm_rt_register_func_type(4, 1, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[7] = wasm_rt_register_func_type(2, 1, WASM_RT_I64, WASM_RT_I32, WASM_RT_I32);
  func_types[8] = wasm_rt_register_func_type(0, 0);
  func_types[9] = wasm_rt_register_func_type(5, 0, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[10] = wasm_rt_register_func_type(0, 1, WASM_RT_I32);
  func_types[11] = wasm_rt_register_func_type(6, 1, WASM_RT_I32, WASM_RT_F64, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[12] = wasm_rt_register_func_type(3, 1, WASM_RT_I64, WASM_RT_I32, WASM_RT_I32, WASM_RT_I32);
  func_types[13] = wasm_rt_register_func_type(3, 1, WASM_RT_I32, WASM_RT_I64, WASM_RT_I32, WASM_RT_I64);
}

static void f2(u32, u32, u32, u32, u32);
static void f3(u32, u32, u32);
static u32 f4(u32, u32, u32);
static u32 f5(u32, u32, u32, u32);
static void f6(u32);
static u32 f7_do_some_copy_on_buf(u32);
static void f8(u32, u32, u32);
static u32 f9_maybe_strlen(u32);
static u32 f10(u32, u32);
static void f11(u32, u32, u32);
static u32 f12(u32);
static u32 f13(u32, u32, u32);
static void f14(u32);
static u32 f15(u32, u32, u32);
static void f16_stackRestore(u32);
static u32 f17_stackAlloc(u32);
static u32 f18_stackSave(void);
static u32 f19_v(u32);
static void f20(u32, u32, u32);
static u32 f21(u32, u32);
static u32 f22(u32, u32);
static void f23(u32);
static u32 f24(u64, u32);
static u32 f25_e_k_check_passwd(u32);
static u32 f26(u64, u32, u32);
static u32 f27(u64, u32);
static void f28(u32, u32);
static u64 f29(u32, u64, u32);
static u32 f30(u32);
static u32 f31_main(u32, u32);
static void ___wasm_call_ctors(void);

static u32 g0_stack;

static void init_globals(void) {
  g0_stack = 0x2ec0;
  // g0 = 5246656u; // 0x500ec0
}

static void f2(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4) {
  u32 l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5;
  i0 = g0_stack;
  i1 = 256u;
  i0 -= i1;
  l5 = i0;
  g0_stack = i0;
  i0 = p2;
  i1 = p3;
  i0 = (u32)((s32)i0 <= (s32)i1);
  if (i0) {goto B0;}
  i0 = p4;
  i1 = 73728u;
  i0 &= i1;
  if (i0) {goto B0;}
  i0 = l5;
  i1 = p1;
  i2 = p2;
  i3 = p3;
  i2 -= i3;
  p4 = i2;
  i3 = 256u;
  i4 = p4;
  i5 = 256u;
  i4 = i4 < i5;
  p1 = i4;
  i2 = i4 ? i2 : i3;
  f8(i0, i1, i2);
  i0 = p0;
  i1 = l5;
  i2 = p1;
  if (i2) {
    i2 = p4;
  } else {
    i2 = p2;
    i3 = p3;
    i2 -= i3;
    p1 = i2;
    L2:
      i2 = p0;
      i3 = l5;
      i4 = 256u;
      f3(i2, i3, i4);
      i2 = p4;
      i3 = 4294967040u;
      i2 += i3;
      p4 = i2;
      i3 = 255u;
      i2 = i2 > i3;
      if (i2) {goto L2;}
    i2 = p1;
    i3 = 255u;
    i2 &= i3;
  }
  f3(i0, i1, i2);
  B0:;
  i0 = l5;
  i1 = 256u;
  i0 += i1;
  g0_stack = i0;
  FUNC_EPILOGUE;
}

static void f3(u32 p0, u32 p1, u32 p2) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  i1 = 32u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = p1;
    i1 = p2;
    i2 = p0;
    i0 = f4(i0, i1, i2);
  }
  FUNC_EPILOGUE;
}

static u32 f4(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = p2;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16)); // 1st: mem[1776+16]=0
  l3 = i0;
  if (i0) {
    i0 = l3;
  } else {
    i0 = p2;
    i0 = f7_do_some_copy_on_buf(i0);
    if (i0) {goto B0;}
    i0 = p2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16));
  }
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 20));
  l5 = i1;
  i0 -= i1;
  i1 = p1;
  i0 = i0 < i1;
  if (i0) {
    i0 = p2;
    i1 = p0;
    i2 = p1;
    i3 = p2;
    i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
    i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
    goto Bfunc;
  }
  i0 = p2;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 75));
  i1 = 0u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {goto B3;}
  i0 = p1;
  l4 = i0;
  L4:
    i0 = l4;
    l3 = i0;
    i0 = !(i0);
    if (i0) {goto B3;}
    i0 = p0;
    i1 = l3;
    i2 = 4294967295u;
    i1 += i2;
    l4 = i1;
    i0 += i1;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    i1 = 10u;
    i0 = i0 != i1;
    if (i0) {goto L4;}
  i0 = p2;
  i1 = p0;
  i2 = l3;
  i3 = p2;
  i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
  i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
  l4 = i0;
  i1 = l3;
  i0 = i0 < i1;
  if (i0) {goto B0;}
  i0 = p1;
  i1 = l3;
  i0 -= i1;
  p1 = i0;
  i0 = p0;
  i1 = l3;
  i0 += i1;
  p0 = i0;
  i0 = p2;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
  l5 = i0;
  i0 = l3;
  l6 = i0;
  B3:;
  i0 = l5;
  i1 = p0;
  i2 = p1;
  f20(i0, i1, i2);
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 20));
  i2 = p1;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = p1;
  i1 = l6;
  i0 += i1;
  l4 = i0;
  B0:;
  i0 = l4;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f5(u32 p0, u32 p1, u32 p2, u32 p3) {
  u32 l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0, l11 = 0,
      l12 = 0, l13 = 0, l14 = 0, l15 = 0, l16 = 0, l17 = 0, l18 = 0, l19 = 0;
  u64 l20 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5, i6;
  u64 j0, j1, j2;
  f64 d1;
  i0 = g0_stack;
  i1 = 80u;
  i0 -= i1;
  l5 = i0;
  g0_stack = i0;
  i0 = l5;
  i1 = 1230u;
  i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
  i0 = l5;
  i1 = 55u;
  i0 += i1;
  l19 = i0;
  i0 = l5;
  i1 = 56u;
  i0 += i1;
  l16 = i0;
  L1:
    i0 = l13;
    i1 = 0u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {goto B2;}
    i0 = l4;
    i1 = 2147483647u;
    i2 = l13;
    i1 -= i2;
    i0 = (u32)((s32)i0 > (s32)i1);
    if (i0) {
      i0 = 3192u;
      i1 = 61u;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = 4294967295u;
      l13 = i0;
      goto B2;
    }
    i0 = l4;
    i1 = l13;
    i0 += i1;
    l13 = i0;
    B2:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
    l9 = i0;
    l4 = i0;
    i0 = l9;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    l6 = i0;
    if (i0) {
      L19:
        i0 = l6;
        i1 = 255u;
        i0 &= i1;
        l7 = i0;
        i0 = !(i0);
        if (i0) {
          i0 = l4;
          l6 = i0;
          goto B22;
        }
        i0 = l7;
        i1 = 37u;
        i0 = i0 != i1;
        if (i0) {goto B21;}
        i0 = l4;
        l6 = i0;
        L24:
          i0 = l4;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 1));
          i1 = 37u;
          i0 = i0 != i1;
          if (i0) {goto B22;}
          i0 = l5;
          i1 = l4;
          i2 = 2u;
          i1 += i2;
          l7 = i1;
          i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
          i0 = l6;
          i1 = 1u;
          i0 += i1;
          l6 = i0;
          i0 = l4;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 2));
          l10 = i0;
          i0 = l7;
          l4 = i0;
          i0 = l10;
          i1 = 37u;
          i0 = i0 == i1;
          if (i0) {goto L24;}
        B22:;
        i0 = l6;
        i1 = l9;
        i0 -= i1;
        l4 = i0;
        i0 = p0;
        if (i0) {
          i0 = p0;
          i1 = l9;
          i2 = l4;
          f3(i0, i1, i2);
        }
        i0 = l4;
        if (i0) {goto L1;}
        i0 = 4294967295u;
        l15 = i0;
        i0 = 1u;
        l6 = i0;
        i0 = l5;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
        l4 = i0;
        i0 = l5;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
        i1 = 4294967248u;
        i0 += i1;
        i1 = 10u;
        i0 = i0 >= i1;
        if (i0) {goto B26;}
        i0 = l4;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 2));
        i1 = 36u;
        i0 = i0 != i1;
        if (i0) {goto B26;}
        i0 = l4;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
        i1 = 4294967248u;
        i0 += i1;
        l15 = i0;
        i0 = 1u;
        l17 = i0;
        i0 = 3u;
        l6 = i0;
        B26:;
        i0 = l5;
        i1 = l4;
        i2 = l6;
        i1 += i2;
        l4 = i1;
        i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
        i0 = 0u;
        l6 = i0;
        i0 = l4;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        l14 = i0;
        i1 = 4294967264u;
        i0 += i1;
        l10 = i0;
        i1 = 31u;
        i0 = i0 > i1;
        if (i0) {
          i0 = l4;
          l7 = i0;
          goto B27;
        }
        i0 = l4;
        l7 = i0;
        i0 = 1u;
        i1 = l10;
        i0 <<= (i1 & 31);
        l10 = i0;
        i1 = 75913u;
        i0 &= i1;
        i0 = !(i0);
        if (i0) {goto B27;}
        L29:
          i0 = l5;
          i1 = l4;
          i2 = 1u;
          i1 += i2;
          l7 = i1;
          i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
          i0 = l6;
          i1 = l10;
          i0 |= i1;
          l6 = i0;
          i0 = l4;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
          l14 = i0;
          i1 = 4294967264u;
          i0 += i1;
          l10 = i0;
          i1 = 31u;
          i0 = i0 > i1;
          if (i0) {goto B27;}
          i0 = l7;
          l4 = i0;
          i0 = 1u;
          i1 = l10;
          i0 <<= (i1 & 31);
          l10 = i0;
          i1 = 75913u;
          i0 &= i1;
          if (i0) {goto L29;}
        B27:;
        i0 = l14;
        i1 = 42u;
        i0 = i0 == i1;
        if (i0) {
          i0 = l5;
          i1 = l7;
          i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 1));
          i2 = 4294967248u;
          i1 += i2;
          i2 = 10u;
          i1 = i1 >= i2;
          if (i1) {goto B33;}
          i1 = l5;
          i1 = i32_load(Z_envZ_memory, (u64)(i1 + 76));
          l4 = i1;
          i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 2));
          i2 = 36u;
          i1 = i1 != i2;
          if (i1) {goto B33;}
          i1 = l4;
          i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 1));
          i2 = 2u;
          i1 <<= (i2 & 31);
          i2 = p3;
          i1 += i2;
          i2 = 4294967104u;
          i1 += i2;
          i2 = 10u;
          i32_store(Z_envZ_memory, (u64)(i1), i2);
          i1 = l4;
          i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 1));
          i2 = 3u;
          i1 <<= (i2 & 31);
          i2 = p2;
          i1 += i2;
          i2 = 4294966912u;
          i1 += i2;
          i1 = i32_load(Z_envZ_memory, (u64)(i1));
          l11 = i1;
          i1 = 1u;
          l17 = i1;
          i1 = l4;
          i2 = 3u;
          i1 += i2;
          goto B32;
          B33:;
          i1 = l17;
          if (i1) {goto B17;}
          i1 = 0u;
          l17 = i1;
          i1 = 0u;
          l11 = i1;
          i1 = p0;
          if (i1) {
            i1 = p1;
            i2 = p1;
            i2 = i32_load(Z_envZ_memory, (u64)(i2));
            l4 = i2;
            i3 = 4u;
            i2 += i3;
            i32_store(Z_envZ_memory, (u64)(i1), i2);
            i1 = l4;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            l11 = i1;
          }
          i1 = l5;
          i1 = i32_load(Z_envZ_memory, (u64)(i1 + 76));
          i2 = 1u;
          i1 += i2;
          B32:;
          l4 = i1;
          i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
          i0 = l11;
          i1 = 4294967295u;
          i0 = (u32)((s32)i0 > (s32)i1);
          if (i0) {goto B30;}
          i0 = 0u;
          i1 = l11;
          i0 -= i1;
          l11 = i0;
          i0 = l6;
          i1 = 8192u;
          i0 |= i1;
          l6 = i0;
          goto B30;
        }
        i0 = l5;
        i1 = 76u;
        i0 += i1;
        i0 = f12(i0);
        l11 = i0;
        i1 = 0u;
        i0 = (u32)((s32)i0 < (s32)i1);
        if (i0) {goto B17;}
        i0 = l5;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
        l4 = i0;
        B30:;
        i0 = 4294967295u;
        l8 = i0;
        i0 = l4;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
        i1 = 46u;
        i0 = i0 != i1;
        if (i0) {goto B35;}
        i0 = l4;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 1));
        i1 = 42u;
        i0 = i0 == i1;
        if (i0) {
          i0 = l4;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 2));
          i1 = 4294967248u;
          i0 += i1;
          i1 = 10u;
          i0 = i0 >= i1;
          if (i0) {goto B37;}
          i0 = l5;
          i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
          l4 = i0;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 3));
          i1 = 36u;
          i0 = i0 != i1;
          if (i0) {goto B37;}
          i0 = l4;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 2));
          i1 = 2u;
          i0 <<= (i1 & 31);
          i1 = p3;
          i0 += i1;
          i1 = 4294967104u;
          i0 += i1;
          i1 = 10u;
          i32_store(Z_envZ_memory, (u64)(i0), i1);
          i0 = l4;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 2));
          i1 = 3u;
          i0 <<= (i1 & 31);
          i1 = p2;
          i0 += i1;
          i1 = 4294966912u;
          i0 += i1;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l8 = i0;
          i0 = l5;
          i1 = l4;
          i2 = 4u;
          i1 += i2;
          l4 = i1;
          i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
          goto B35;
          B37:;
          i0 = l17;
          if (i0) {goto B17;}
          i0 = p0;
          if (i0) {
            i0 = p1;
            i1 = p1;
            i1 = i32_load(Z_envZ_memory, (u64)(i1));
            l4 = i1;
            i2 = 4u;
            i1 += i2;
            i32_store(Z_envZ_memory, (u64)(i0), i1);
            i0 = l4;
            i0 = i32_load(Z_envZ_memory, (u64)(i0));
          } else {
            i0 = 0u;
          }
          l8 = i0;
          i0 = l5;
          i1 = l5;
          i1 = i32_load(Z_envZ_memory, (u64)(i1 + 76));
          i2 = 2u;
          i1 += i2;
          l4 = i1;
          i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
          goto B35;
        }
        i0 = l5;
        i1 = l4;
        i2 = 1u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
        i0 = l5;
        i1 = 76u;
        i0 += i1;
        i0 = f12(i0);
        l8 = i0;
        i0 = l5;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
        l4 = i0;
        B35:;
        i0 = 0u;
        l7 = i0;
        L39:
          i0 = l7;
          l18 = i0;
          i0 = 4294967295u;
          l12 = i0;
          i0 = l4;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
          i1 = 4294967231u;
          i0 += i1;
          i1 = 57u;
          i0 = i0 > i1;
          if (i0) {goto B0;}
          i0 = l5;
          i1 = l4;
          i2 = 1u;
          i1 += i2;
          l14 = i1;
          i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
          i0 = l4;
          i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
          l7 = i0;
          i0 = l14;
          l4 = i0;
          i0 = l7;
          i1 = l18;
          i2 = 58u;
          i1 *= i2;
          i0 += i1;
          i1 = 1231u;
          i0 += i1;
          i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
          l7 = i0;
          i1 = 4294967295u;
          i0 += i1;
          i1 = 8u;
          i0 = i0 < i1;
          if (i0) {goto L39;}
        i0 = l7;
        i0 = !(i0);
        if (i0) {goto B0;}
        i0 = l7;
        i1 = 19u;
        i0 = i0 == i1;
        if (i0) {
          i0 = l15;
          i1 = 4294967295u;
          i0 = (u32)((s32)i0 <= (s32)i1);
          if (i0) {goto B42;}
          goto B0;
        }
        i0 = l15;
        i1 = 0u;
        i0 = (u32)((s32)i0 < (s32)i1);
        if (i0) {goto B41;}
        i0 = p3;
        i1 = l15;
        i2 = 2u;
        i1 <<= (i2 & 31);
        i0 += i1;
        i1 = l7;
        i32_store(Z_envZ_memory, (u64)(i0), i1);
        i0 = l5;
        i1 = p2;
        i2 = l15;
        i3 = 3u;
        i2 <<= (i3 & 31);
        i1 += i2;
        j1 = i64_load(Z_envZ_memory, (u64)(i1));
        i64_store(Z_envZ_memory, (u64)(i0 + 64), j1);
        B42:;
        i0 = 0u;
        l4 = i0;
        i0 = p0;
        i0 = !(i0);
        if (i0) {goto L1;}
        goto B40;
        B41:;
        i0 = p0;
        i0 = !(i0);
        if (i0) {goto B4;}
        i0 = l5;
        i1 = 4294967232u;
        i0 -= i1;
        i1 = l7;
        i2 = p1;
        f11(i0, i1, i2);
        i0 = l5;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
        l14 = i0;
        B40:;
        i0 = l6;
        i1 = 4294901759u;
        i0 &= i1;
        l10 = i0;
        i1 = l6;
        i2 = l6;
        i3 = 8192u;
        i2 &= i3;
        i0 = i2 ? i0 : i1;
        l6 = i0;
        i0 = 0u;
        l12 = i0;
        i0 = 1264u;
        l15 = i0;
        i0 = l16;
        l7 = i0;
        i0 = l14;
        i1 = 4294967295u;
        i0 += i1;
        i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
        l4 = i0;
        i1 = 4294967263u;
        i0 &= i1;
        i1 = l4;
        i2 = l4;
        i3 = 15u;
        i2 &= i3;
        i3 = 3u;
        i2 = i2 == i3;
        i0 = i2 ? i0 : i1;
        i1 = l4;
        i2 = l18;
        i0 = i2 ? i0 : i1;
        l4 = i0;
        i1 = 4294967208u;
        i0 += i1;
        l14 = i0;
        i1 = 32u;
        i0 = i0 <= i1;
        if (i0) {goto B20;}
        i0 = l4;
        i1 = 4294967231u;
        i0 += i1;
        l10 = i0;
        i1 = 6u;
        i0 = i0 > i1;
        if (i0) {
          i0 = l4;
          i1 = 83u;
          i0 = i0 != i1;
          if (i0) {goto B5;}
          i0 = l8;
          i0 = !(i0);
          if (i0) {goto B47;}
          i0 = l5;
          i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
          goto B45;
        }
        i0 = l10;
        i1 = 1u;
        i0 -= i1;
        switch (i0) {
          case 0: goto B5;
          case 1: goto B46;
          case 2: goto B5;
          default: goto B16;
        }
        B47:;
        i0 = 0u;
        l4 = i0;
        i0 = p0;
        i1 = 32u;
        i2 = l11;
        i3 = 0u;
        i4 = l6;
        f2(i0, i1, i2, i3, i4);
        goto B44;
        B46:;
        i0 = l5;
        i1 = 0u;
        i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
        i0 = l5;
        i1 = l5;
        j1 = i64_load(Z_envZ_memory, (u64)(i1 + 64));
        i64_store32(Z_envZ_memory, (u64)(i0 + 8), j1);
        i0 = l5;
        i1 = l5;
        i2 = 8u;
        i1 += i2;
        i32_store(Z_envZ_memory, (u64)(i0 + 64), i1);
        i0 = 4294967295u;
        l8 = i0;
        i0 = l5;
        i1 = 8u;
        i0 += i1;
        B45:;
        l7 = i0;
        i0 = 0u;
        l4 = i0;
        L50:
          i0 = l7;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l9 = i0;
          i0 = !(i0);
          if (i0) {goto B49;}
          i0 = l5;
          i1 = 4u;
          i0 += i1;
          i1 = l9;
          i0 = f10(i0, i1);
          l9 = i0;
          i1 = 0u;
          i0 = (u32)((s32)i0 < (s32)i1);
          l10 = i0;
          if (i0) {goto B51;}
          i0 = l9;
          i1 = l8;
          i2 = l4;
          i1 -= i2;
          i0 = i0 > i1;
          if (i0) {goto B51;}
          i0 = l7;
          i1 = 4u;
          i0 += i1;
          l7 = i0;
          i0 = l8;
          i1 = l4;
          i2 = l9;
          i1 += i2;
          l4 = i1;
          i0 = i0 > i1;
          if (i0) {goto L50;}
          goto B49;
          B51:;
        i0 = 4294967295u;
        l12 = i0;
        i0 = l10;
        if (i0) {goto B0;}
        B49:;
        i0 = p0;
        i1 = 32u;
        i2 = l11;
        i3 = l4;
        i4 = l6;
        f2(i0, i1, i2, i3, i4);
        i0 = l4;
        i0 = !(i0);
        if (i0) {
          i0 = 0u;
          l4 = i0;
          goto B44;
        }
        i0 = 0u;
        l10 = i0;
        i0 = l5;
        i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
        l7 = i0;
        L53:
          i0 = l7;
          i0 = i32_load(Z_envZ_memory, (u64)(i0));
          l9 = i0;
          i0 = !(i0);
          if (i0) {goto B44;}
          i0 = l5;
          i1 = 4u;
          i0 += i1;
          i1 = l9;
          i0 = f10(i0, i1);
          l9 = i0;
          i1 = l10;
          i0 += i1;
          l10 = i0;
          i1 = l4;
          i0 = (u32)((s32)i0 > (s32)i1);
          if (i0) {goto B44;}
          i0 = p0;
          i1 = l5;
          i2 = 4u;
          i1 += i2;
          i2 = l9;
          f3(i0, i1, i2);
          i0 = l7;
          i1 = 4u;
          i0 += i1;
          l7 = i0;
          i0 = l10;
          i1 = l4;
          i0 = i0 < i1;
          if (i0) {goto L53;}
        B44:;
        i0 = p0;
        i1 = 32u;
        i2 = l11;
        i3 = l4;
        i4 = l6;
        i5 = 8192u;
        i4 ^= i5;
        f2(i0, i1, i2, i3, i4);
        i0 = l11;
        i1 = l4;
        i2 = l11;
        i3 = l4;
        i2 = (u32)((s32)i2 > (s32)i3);
        i0 = i2 ? i0 : i1;
        l4 = i0;
        goto L1;
        B21:;
        i0 = l5;
        i1 = l4;
        i2 = 1u;
        i1 += i2;
        l7 = i1;
        i32_store(Z_envZ_memory, (u64)(i0 + 76), i1);
        i0 = l4;
        i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 1));
        l6 = i0;
        i0 = l7;
        l4 = i0;
        goto L19;
        B20:;
      i0 = l14;
      i1 = 1u;
      i0 -= i1;
      switch (i0) {
        case 0: goto B5;
        case 1: goto B5;
        case 2: goto B5;
        case 3: goto B5;
        case 4: goto B5;
        case 5: goto B5;
        case 6: goto B5;
        case 7: goto B5;
        case 8: goto B16;
        case 9: goto B5;
        case 10: goto B14;
        case 11: goto B13;
        case 12: goto B16;
        case 13: goto B16;
        case 14: goto B16;
        case 15: goto B5;
        case 16: goto B13;
        case 17: goto B5;
        case 18: goto B5;
        case 19: goto B5;
        case 20: goto B5;
        case 21: goto B9;
        case 22: goto B12;
        case 23: goto B11;
        case 24: goto B5;
        case 25: goto B5;
        case 26: goto B15;
        case 27: goto B5;
        case 28: goto B8;
        case 29: goto B5;
        case 30: goto B5;
        default: goto B10;
      }
    }
    i0 = l13;
    l12 = i0;
    i0 = p0;
    if (i0) {goto B0;}
    i0 = l17;
    i0 = !(i0);
    if (i0) {goto B4;}
    i0 = 1u;
    l4 = i0;
    L54:
      i0 = p3;
      i1 = l4;
      i2 = 2u;
      i1 <<= (i2 & 31);
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      p0 = i0;
      if (i0) {
        i0 = p2;
        i1 = l4;
        i2 = 3u;
        i1 <<= (i2 & 31);
        i0 += i1;
        i1 = p0;
        i2 = p1;
        f11(i0, i1, i2);
        i0 = 1u;
        l12 = i0;
        i0 = l4;
        i1 = 1u;
        i0 += i1;
        l4 = i0;
        i1 = 10u;
        i0 = i0 != i1;
        if (i0) {goto L54;}
        goto B0;
      }
    i0 = 1u;
    l12 = i0;
    i0 = l4;
    i1 = 10u;
    i0 = i0 >= i1;
    if (i0) {goto B0;}
    L56:
      i0 = p3;
      i1 = l4;
      i2 = 2u;
      i1 <<= (i2 & 31);
      i0 += i1;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      if (i0) {goto B17;}
      i0 = l4;
      i1 = 8u;
      i0 = i0 > i1;
      p0 = i0;
      i0 = l4;
      i1 = 1u;
      i0 += i1;
      l4 = i0;
      i0 = p0;
      i0 = !(i0);
      if (i0) {goto L56;}
    goto B0;
    B17:;
    i0 = 4294967295u;
    l12 = i0;
    goto B0;
    B16:;
    i0 = p0;
    i1 = l5;
    d1 = f64_load(Z_envZ_memory, (u64)(i1 + 64));
    i2 = l11;
    i3 = l8;
    i4 = l6;
    i5 = l4;
    i6 = 0u;
    i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, f64, u32, u32, u32, u32), 11, i6, i0, d1, i2, i3, i4, i5);
    l4 = i0;
    goto L1;
    B15:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    l4 = i0;
    i1 = 1274u;
    i2 = l4;
    i0 = i2 ? i0 : i1;
    l9 = i0;
    i1 = l8;
    i0 = f21(i0, i1);
    l4 = i0;
    i1 = l8;
    i2 = l9;
    i1 += i2;
    i2 = l4;
    i0 = i2 ? i0 : i1;
    l7 = i0;
    i0 = l10;
    l6 = i0;
    i0 = l4;
    i1 = l9;
    i0 -= i1;
    i1 = l8;
    i2 = l4;
    i0 = i2 ? i0 : i1;
    l8 = i0;
    goto B5;
    B14:;
    i0 = l5;
    i1 = l5;
    j1 = i64_load(Z_envZ_memory, (u64)(i1 + 64));
    i64_store8(Z_envZ_memory, (u64)(i0 + 55), j1);
    i0 = 1u;
    l8 = i0;
    i0 = l19;
    l9 = i0;
    i0 = l10;
    l6 = i0;
    goto B5;
    B13:;
    i0 = l5;
    j0 = i64_load(Z_envZ_memory, (u64)(i0 + 64));
    l20 = j0;
    j1 = 18446744073709551615ull;
    i0 = (u64)((s64)j0 <= (s64)j1);
    if (i0) {
      i0 = l5;
      j1 = 0ull;
      j2 = l20;
      j1 -= j2;
      l20 = j1;
      i64_store(Z_envZ_memory, (u64)(i0 + 64), j1);
      i0 = 1u;
      l12 = i0;
      i0 = 1264u;
      goto B7;
    }
    i0 = l6;
    i1 = 2048u;
    i0 &= i1;
    if (i0) {
      i0 = 1u;
      l12 = i0;
      i0 = 1265u;
      goto B7;
    }
    i0 = 1266u;
    i1 = 1264u;
    i2 = l6;
    i3 = 1u;
    i2 &= i3;
    l12 = i2;
    i0 = i2 ? i0 : i1;
    goto B7;
    B12:;
    i0 = l5;
    j0 = i64_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l16;
    i0 = f27(j0, i1);
    l9 = i0;
    i0 = l6;
    i1 = 8u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {goto B6;}
    i0 = l8;
    i1 = l16;
    i2 = l9;
    i1 -= i2;
    l4 = i1;
    i2 = 1u;
    i1 += i2;
    i2 = l8;
    i3 = l4;
    i2 = (u32)((s32)i2 > (s32)i3);
    i0 = i2 ? i0 : i1;
    l8 = i0;
    goto B6;
    B11:;
    i0 = l8;
    i1 = 8u;
    i2 = l8;
    i3 = 8u;
    i2 = i2 > i3;
    i0 = i2 ? i0 : i1;
    l8 = i0;
    i0 = l6;
    i1 = 8u;
    i0 |= i1;
    l6 = i0;
    i0 = 120u;
    l4 = i0;
    B10:;
    i0 = l5;
    j0 = i64_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l16;
    i2 = l4;
    i3 = 32u;
    i2 &= i3;
    i0 = f26(j0, i1, i2);
    l9 = i0;
    i0 = l6;
    i1 = 8u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {goto B6;}
    i0 = l5;
    j0 = i64_load(Z_envZ_memory, (u64)(i0 + 64));
    i0 = !(j0);
    if (i0) {goto B6;}
    i0 = l4;
    i1 = 4u;
    i0 >>= (i1 & 31);
    i1 = 1264u;
    i0 += i1;
    l15 = i0;
    i0 = 2u;
    l12 = i0;
    goto B6;
    B9:;
    i0 = 0u;
    l4 = i0;
    i0 = l18;
    i1 = 255u;
    i0 &= i1;
    l7 = i0;
    i1 = 7u;
    i0 = i0 > i1;
    if (i0) {goto L1;}
    i0 = l7;
    i1 = 1u;
    i0 -= i1;
    switch (i0) {
      case 0: goto B64;
      case 1: goto B63;
      case 2: goto B62;
      case 3: goto B61;
      case 4: goto L1;
      case 5: goto B60;
      case 6: goto B59;
      default: goto B65;
    }
    B65:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l13;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto L1;
    B64:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l13;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto L1;
    B63:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l13;
    j1 = (u64)(s64)(s32)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto L1;
    B62:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l13;
    i32_store16(Z_envZ_memory, (u64)(i0), i1);
    goto L1;
    B61:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l13;
    i32_store8(Z_envZ_memory, (u64)(i0), i1);
    goto L1;
    B60:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l13;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    goto L1;
    B59:;
    i0 = l5;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 64));
    i1 = l13;
    j1 = (u64)(s64)(s32)(i1);
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    goto L1;
    B8:;
    i0 = l5;
    j0 = i64_load(Z_envZ_memory, (u64)(i0 + 64));
    l20 = j0;
    i0 = 1264u;
    B7:;
    l15 = i0;
    j0 = l20;
    i1 = l16;
    i0 = f24(j0, i1);
    l9 = i0;
    B6:;
    i0 = l6;
    i1 = 4294901759u;
    i0 &= i1;
    i1 = l6;
    i2 = l8;
    i3 = 4294967295u;
    i2 = (u32)((s32)i2 > (s32)i3);
    i0 = i2 ? i0 : i1;
    l6 = i0;
    i0 = l5;
    j0 = i64_load(Z_envZ_memory, (u64)(i0 + 64));
    l20 = j0;
    i0 = l8;
    if (i0) {goto B67;}
    j0 = l20;
    i0 = !(j0);
    i0 = !(i0);
    if (i0) {goto B67;}
    i0 = l16;
    l9 = i0;
    i0 = 0u;
    goto B66;
    B67:;
    i0 = l8;
    j1 = l20;
    i1 = !(j1);
    i2 = l16;
    i3 = l9;
    i2 -= i3;
    i1 += i2;
    l4 = i1;
    i2 = l8;
    i3 = l4;
    i2 = (u32)((s32)i2 > (s32)i3);
    i0 = i2 ? i0 : i1;
    B66:;
    l8 = i0;
    B5:;
    i0 = p0;
    i1 = 32u;
    i2 = l12;
    i3 = l7;
    i4 = l9;
    i3 -= i4;
    l10 = i3;
    i4 = l8;
    i5 = l8;
    i6 = l10;
    i5 = (u32)((s32)i5 < (s32)i6);
    i3 = i5 ? i3 : i4;
    l14 = i3;
    i2 += i3;
    l7 = i2;
    i3 = l11;
    i4 = l11;
    i5 = l7;
    i4 = (u32)((s32)i4 < (s32)i5);
    i2 = i4 ? i2 : i3;
    l4 = i2;
    i3 = l7;
    i4 = l6;
    f2(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = l15;
    i2 = l12;
    f3(i0, i1, i2);
    i0 = p0;
    i1 = 48u;
    i2 = l4;
    i3 = l7;
    i4 = l6;
    i5 = 65536u;
    i4 ^= i5;
    f2(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = 48u;
    i2 = l14;
    i3 = l10;
    i4 = 0u;
    f2(i0, i1, i2, i3, i4);
    i0 = p0;
    i1 = l9;
    i2 = l10;
    f3(i0, i1, i2);
    i0 = p0;
    i1 = 32u;
    i2 = l4;
    i3 = l7;
    i4 = l6;
    i5 = 8192u;
    i4 ^= i5;
    f2(i0, i1, i2, i3, i4);
    goto L1;
    B4:;
  i0 = 0u;
  l12 = i0;
  B0:;
  i0 = l5;
  i1 = 80u;
  i0 += i1;
  g0_stack = i0;
  i0 = l12;
  FUNC_EPILOGUE;
  return i0;
}

static void f6(u32 p0_str) {
  u32 l1 = 0, l2 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5;
  i0 = 1260u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0; // l1 = 1776
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76)); // -1 ; data21
  i1 = 0u;
  i0 = (u32)((s32)i0 >= (s32)i1);
  if (i0) {
    i0 = 1u;
  } else {
    i0 = 0u;
  }
  i0 = 4294967295u; // = -1
  i1 = 0u;
  i2 = p0_str;
  i2 = f9_maybe_strlen(i2);
  l2 = i2;
  // first time:
  // l2 = f9(1024) = 75
  // second time:
  // l2 = f9(1100) = 88
  i3 = p0_str;
  i4 = l2;
  i5 = l1;
  i3 = f15(i3, i4, i5);
  // f15(s, strlen(s), buf=1776)
  // 1st: f15(1024, 75, 1776)
  i2 = i2 != i3;
  i0 = i2 ? i0 : i1;
  i1 = 0u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {goto B1;}
  i0 = l1;
  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 75)); // 10 ; data21
  i1 = 10u;
  i0 = i0 == i1;
  if (i0) {goto B2;}
  i0 = l1;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
  p0_str = i0;
  i1 = l1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 16));
  i0 = i0 >= i1;
  if (i0) {goto B2;}
  i0 = l1;
  i1 = p0_str;
  i2 = 1u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = p0_str;
  i1 = 10u;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  goto B1;
  B2:;
  i0 = l1;
  f14(i0); // f14(1776) ; f14(1100)
  B1:;
  FUNC_EPILOGUE;
}

static u32 f7_do_some_copy_on_buf(u32 p0) {
  // 1st: p0=1776
  u32 l1 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j1;
  i0 = p0;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 74)); // 1st: mem[1776+74]=0
  l1 = i1;
  i2 = 4294967295u;
  i1 += i2;
  i2 = l1;
  i1 |= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 74), i1);
  // mem[p0+74] |= (mem[p0+74] - 1)
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i1 = 8u;
  i0 &= i1;
  if (i0) {
    // if (uint32)mem[p0] & 8
    i0 = p0;
    i1 = l1;
    i2 = 32u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    // mem[p0] |= 32
    i0 = 4294967295u;
    goto Bfunc;
    // return -1
  }
  i0 = p0;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 4), j1);
  // mem[p0+4] = 0ull
  i0 = p0;
  i1 = p0;
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 44));
  l1 = i1;
  i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
  // mem[p0+28] = l1 = (uint32)mem[p0+44] = (1st) 2168
  i0 = p0;
  i1 = l1;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  // mem[p0+20] = mem[p0+44]
  i0 = p0;
  i1 = l1;
  i2 = p0;
  i2 = i32_load(Z_envZ_memory, (u64)(i2 + 48));
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
  // mem[p0+16] = mem[p0+44] + mem[p0+48] = (1st)2168 + 1024
  i0 = 0u;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static void f8(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0;
  u64 l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j0, j1;
  i0 = p2;
  i0 = !(i0);
  if (i0) {goto B0;}
  i0 = p0;
  i1 = p2;
  i0 += i1;
  l3 = i0;
  i1 = 4294967295u;
  i0 += i1;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p2;
  i1 = 3u;
  i0 = i0 < i1;
  if (i0) {goto B0;}
  i0 = l3;
  i1 = 4294967294u;
  i0 += i1;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
  i0 = l3;
  i1 = 4294967293u;
  i0 += i1;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0 + 2), i1);
  i0 = p2;
  i1 = 7u;
  i0 = i0 < i1;
  if (i0) {goto B0;}
  i0 = l3;
  i1 = 4294967292u;
  i0 += i1;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0 + 3), i1);
  i0 = p2;
  i1 = 9u;
  i0 = i0 < i1;
  if (i0) {goto B0;}
  i0 = p0;
  i1 = 0u;
  i2 = p0;
  i1 -= i2;
  i2 = 3u;
  i1 &= i2;
  l4 = i1;
  i0 += i1;
  l3 = i0;
  i1 = p1;
  i2 = 255u;
  i1 &= i2;
  i2 = 16843009u;
  i1 *= i2;
  p0 = i1;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l3;
  i1 = p2;
  i2 = l4;
  i1 -= i2;
  i2 = 4294967292u;
  i1 &= i2;
  p2 = i1;
  i0 += i1;
  p1 = i0;
  i1 = 4294967292u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p2;
  i1 = 9u;
  i0 = i0 < i1;
  if (i0) {goto B0;}
  i0 = l3;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
  i0 = l3;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
  i0 = p1;
  i1 = 4294967288u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = 4294967284u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p2;
  i1 = 25u;
  i0 = i0 < i1;
  if (i0) {goto B0;}
  i0 = l3;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
  i0 = l3;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = l3;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
  i0 = l3;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
  i0 = p1;
  i1 = 4294967280u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = 4294967276u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = 4294967272u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i1 = 4294967268u;
  i0 += i1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p2;
  i1 = l3;
  i2 = 4u;
  i1 &= i2;
  i2 = 24u;
  i1 |= i2;
  p1 = i1;
  i0 -= i1;
  p2 = i0;
  i1 = 32u;
  i0 = i0 < i1;
  if (i0) {goto B0;}
  i0 = p0;
  j0 = (u64)(i0);
  l5 = j0;
  j1 = 32ull;
  j0 <<= (j1 & 63);
  j1 = l5;
  j0 |= j1;
  l5 = j0;
  i0 = p1;
  i1 = l3;
  i0 += i1;
  p1 = i0;
  L1:
    i0 = p1;
    j1 = l5;
    i64_store(Z_envZ_memory, (u64)(i0 + 24), j1);
    i0 = p1;
    j1 = l5;
    i64_store(Z_envZ_memory, (u64)(i0 + 16), j1);
    i0 = p1;
    j1 = l5;
    i64_store(Z_envZ_memory, (u64)(i0 + 8), j1);
    i0 = p1;
    j1 = l5;
    i64_store(Z_envZ_memory, (u64)(i0), j1);
    i0 = p1;
    i1 = 32u;
    i0 += i1;
    p1 = i0;
    i0 = p2;
    i1 = 4294967264u;
    i0 += i1;
    p2 = i0;
    i1 = 31u;
    i0 = i0 > i1;
    if (i0) {goto L1;}
  B0:;
  FUNC_EPILOGUE;
}

static u32 f9_maybe_strlen(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  l1 = i0;
  i0 = p0;
  i1 = 3u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {goto B1;}
  // if not p0 & 3: break
  i0 = p0;
  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  i0 = !(i0);
  if (i0) {
    goto B0;
  }
  L3:
    i0 = l1;
    i1 = 1u;
    i0 += i1;
    l1 = i0;
    i1 = 3u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {goto B1;}
    i0 = l1;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    if (i0) {goto L3;}
    // while *p0 && (p0 & 3): p0++
  goto B0;
  B1:;
  L4:
    i0 = l1;
    l2 = i0;
    // l2 = l1
    i1 = 4u;
    i0 += i1;
    l1 = i0;
    // l1 += 4
    i0 = l2;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    l3 = i0;
    // l3 = (uint32)memory[i]
    // l3 = (1st time) memory[1024] = 2037149263
    i1 = 4294967295u; // -1
    i0 ^= i1;
    i1 = l3;
    i2 = 4278124287u; // 0xfefefeff
    i1 += i2;
    i0 &= i1;
    i1 = 2155905152u; // 0x80808080
    i0 &= i1;
    // i0 = ~l3 & (l3 + 0xfefefeff) & 0x80808080
    // = (1st time) 0
    i0 = !(i0);
    if (i0) {goto L4;}
    // 1st: l1=1100; l2=1096; l3=10000543
  i0 = l3;
  i1 = 255u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    // if not l3 & 0xff
    i0 = l2;
    l1 = i0;
    goto B0;
  }
  L6:
    i0 = l2;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 1));
    l3 = i0;
    // l3=mem[l2+1]
    i0 = l2;
    i1 = 1u;
    i0 += i1;
    l1 = i0;
    l2 = i0;
    // l1=l2=l2+1
    i0 = l3;
    if (i0) {goto L6;}
    // while mem[l2+1]
  B0:;
  i0 = l1;
  i1 = p0;
  i0 -= i1;
  FUNC_EPILOGUE;
  return i0;
  // return l1 - p0
  // 1st: 75
}

static u32 f10(u32 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = p0;
  i0 = !(i0);
  if (i0) {
    i0 = 0u;
    goto Bfunc;
  }
  i0 = p0;
  i1 = p1;
  i0 = f22(i0, i1);
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static void f11(u32 p0, u32 p1, u32 p2) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j1;
  i0 = p1;
  i1 = 20u;
  i0 = i0 > i1;
  if (i0) {goto B1;}
  i0 = p1;
  i1 = 4294967287u;
  i0 += i1;
  p1 = i0;
  i1 = 9u;
  i0 = i0 > i1;
  if (i0) {goto B1;}
  i0 = p1;
  i1 = 1u;
  i0 -= i1;
  switch (i0) {
    case 0: goto B8;
    case 1: goto B7;
    case 2: goto B0;
    case 3: goto B6;
    case 4: goto B5;
    case 5: goto B4;
    case 6: goto B3;
    case 7: goto B0;
    case 8: goto B2;
    default: goto B9;
  }
  B9:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  goto Bfunc;
  B8:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  j1 = i64_load32_s(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  goto Bfunc;
  B7:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  j1 = i64_load32_u(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  goto Bfunc;
  B6:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  j1 = i64_load16_s(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  goto Bfunc;
  B5:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  j1 = i32_load16_u(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  goto Bfunc;
  B4:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  j1 = i64_load8_s(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  goto Bfunc;
  B3:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  p1 = i1;
  i2 = 4u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  j1 = i64_load8_u(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  goto Bfunc;
  B2:;
  i0 = p0;
  i1 = p2;
  i2 = 0u;
  CALL_INDIRECT((*Z_envZ_table), void (*)(u32, u32), 5, i2, i0, i1);
  B1:;
  goto Bfunc;
  B0:;
  i0 = p2;
  i1 = p2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i2 = 7u;
  i1 += i2;
  i2 = 4294967288u;
  i1 &= i2;
  p1 = i1;
  i2 = 8u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p0;
  i1 = p1;
  j1 = i64_load(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  Bfunc:;
  FUNC_EPILOGUE;
}

static u32 f12(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
  i1 = 4294967248u;
  i0 += i1;
  i1 = 10u;
  i0 = i0 < i1;
  if (i0) {
    L1:
      i0 = p0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l1 = i0;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
      l3 = i0;
      i0 = p0;
      i1 = l1;
      i2 = 1u;
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = l3;
      i1 = l2;
      i2 = 10u;
      i1 *= i2;
      i0 += i1;
      i1 = 4294967248u;
      i0 += i1;
      l2 = i0;
      i0 = l1;
      i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
      i1 = 4294967248u;
      i0 += i1;
      i1 = 10u;
      i0 = i0 < i1;
      if (i0) {goto L1;}
  }
  i0 = l2;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f13(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0, l8 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4, i5, i6;
  u64 j1;
  i0 = g0_stack;
  i1 = 32u;
  i0 -= i1;
  l3 = i0;
  g0_stack = i0;
  i0 = l3;
  i1 = p0;
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 28));
  l5 = i1;
  i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
  l4 = i0;
  i0 = l3;
  i1 = p2;
  i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
  i0 = l3;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
  i0 = l3;
  i1 = l4;
  i2 = l5;
  i1 -= i2;
  p1 = i1;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = p1;
  i1 = p2;
  i0 += i1;
  l6 = i0;
  i0 = 2u;
  l5 = i0;
  i0 = l3;
  i1 = 16u;
  i0 += i1;
  p1 = i0;
  L0:
    i0 = l6;
    i1 = 0u;
    i2 = p0;
    i2 = i32_load(Z_envZ_memory, (u64)(i2 + 60));
    i3 = p1;
    i4 = l5;
    i5 = l3;
    i6 = 12u;
    i5 += i6;
    i2 = (*Z_wasi_snapshot_preview1Z_bZ_iiiii)(i2, i3, i4, i5);
    // call 1 === f1(1, 5246592, 2, 5246588)
    // i2 = 0
    // f1: may be console.log() ??
    l4 = i2;
    i2 = !(i2);
    if (i2) {goto B4;}
    i1 = 3192u;
    i2 = l4;
    i32_store(Z_envZ_memory, (u64)(i1), i2);
    i1 = 4294967295u;
    B4:;
    if (i1) {
      i1 = l3;
      i2 = 4294967295u;
      i32_store(Z_envZ_memory, (u64)(i1 + 12), i2);
      i1 = 4294967295u;
      goto B3;
    }
    i1 = l3;
    i1 = i32_load(Z_envZ_memory, (u64)(i1 + 12));
    B3:;
    l4 = i1;
    i0 = i0 == i1;
    if (i0) {
      i0 = p0;
      i1 = p0;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 44));
      p1 = i1;
      i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
      i0 = p0;
      i1 = p1;
      i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
      i0 = p0;
      i1 = p1;
      i2 = p0;
      i2 = i32_load(Z_envZ_memory, (u64)(i2 + 48));
      i1 += i2;
      i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
      i0 = p2;
      goto B2;
    }
    i0 = l4;
    i1 = 4294967295u;
    i0 = (u32)((s32)i0 > (s32)i1);
    if (i0) {goto B1;}
    i0 = p0;
    i1 = 0u;
    i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
    i0 = p0;
    j1 = 0ull;
    i64_store(Z_envZ_memory, (u64)(i0 + 16), j1);
    i0 = p0;
    i1 = p0;
    i1 = i32_load(Z_envZ_memory, (u64)(i1));
    i2 = 32u;
    i1 |= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 0u;
    i1 = l5;
    i2 = 2u;
    i1 = i1 == i2;
    if (i1) {goto B2;}
    i0 = p2;
    i1 = p1;
    i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
    i0 -= i1;
    B2:;
    p0 = i0;
    i0 = l3;
    i1 = 32u;
    i0 += i1;
    g0_stack = i0;
    i0 = p0;
    goto Bfunc;
    B1:;
    i0 = p1;
    i1 = 8u;
    i0 += i1;
    i1 = p1;
    i2 = l4;
    i3 = p1;
    i3 = i32_load(Z_envZ_memory, (u64)(i3 + 4));
    l7 = i3;
    i2 = i2 > i3;
    l8 = i2;
    i0 = i2 ? i0 : i1;
    p1 = i0;
    i1 = l4;
    i2 = l7;
    i3 = 0u;
    i4 = l8;
    i2 = i4 ? i2 : i3;
    i1 -= i2;
    l7 = i1;
    i2 = p1;
    i2 = i32_load(Z_envZ_memory, (u64)(i2));
    i1 += i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = p1;
    i1 = p1;
    i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
    i2 = l7;
    i1 -= i2;
    i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
    i0 = l6;
    i1 = l4;
    i0 -= i1;
    l6 = i0;
    i0 = l5;
    i1 = l8;
    i0 -= i1;
    l5 = i0;
    goto L0;
  UNREACHABLE;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static void f14(u32 p0) {
  u32 l1 = 0, l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = g0_stack;
  i1 = 16u;
  i0 -= i1;
  l1 = i0;
  g0_stack = i0;
  i0 = l1;
  i1 = 10u;
  i32_store8(Z_envZ_memory, (u64)(i0 + 15), i1);
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16));
  l2 = i0;
  i0 = !(i0);
  if (i0) {
    i0 = p0;
    i0 = f7_do_some_copy_on_buf(i0);
    if (i0) {goto B0;}
    i0 = p0;
    i0 = i32_load(Z_envZ_memory, (u64)(i0 + 16));
    l2 = i0;
  }
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
  l3 = i0;
  i1 = l2;
  i0 = i0 >= i1;
  if (i0) {goto B2;}
  i0 = p0;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 75));
  i1 = 10u;
  i0 = i0 == i1;
  if (i0) {goto B2;}
  i0 = p0;
  i1 = l3;
  i2 = 1u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = l3;
  i1 = 10u;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  goto B0;
  B2:;
  i0 = p0;
  i1 = l1;
  i2 = 15u;
  i1 += i2;
  i2 = 1u;
  i3 = p0;
  i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
  i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
  // call indirect === f13(1776, 5246623, 1, 2)
  i1 = 1u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = l1;
  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 15));
  B0:;
  i0 = l1;
  i1 = 16u;
  i0 += i1;
  g0_stack = i0;
  FUNC_EPILOGUE;
}

static u32 f15(u32 p0_str, u32 p1_len, u32 p2_buf) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = p1_len;
  i1 = p2_buf; // always = 1776
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 76)); // mem[1252]=-1
  i2 = 4294967295u; // -1
  i1 = (u32)((s32)i1 <= (s32)i2);
  if (i1) {
    i1 = p0_str;
    i2 = p1_len;
    i3 = p2_buf;
    i1 = f4(i1, i2, i3);
    // 1st: f4(1024, 75, 1776)
    goto B0;
  }
  i1 = p0_str;
  i2 = p1_len;
  i3 = p2_buf;
  i1 = f4(i1, i2, i3);
  B0:;
  p0_str = i1;
  i0 = i0 == i1;
  if (i0) {
    i0 = p1_len;
    goto Bfunc;
  }
  i0 = p0_str;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static void f16_stackRestore(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = p0;
  g0_stack = i0;
  FUNC_EPILOGUE;
}

static u32 f17_stackAlloc(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = g0_stack;
  i1 = p0;
  i0 -= i1;
  i1 = 4294967280u;
  i0 &= i1;
  p0 = i0;
  g0_stack = i0;
  i0 = p0;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f18_stackSave(void) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = g0_stack;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f19_v(u32 p0) {
  u32 l1 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j1;
  i0 = g0_stack;
  i1 = 48u;
  i0 -= i1;
  l1 = i0;
  g0_stack = i0;
  i0 = l1;
  i1 = 1222u;
  j1 = i64_load(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0 + 38), j1);
  i0 = l1;
  i1 = 1216u;
  j1 = i64_load(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0 + 32), j1);
  i0 = l1;
  i1 = 1208u;
  j1 = i64_load(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0 + 24), j1);
  i0 = l1;
  i1 = 1200u;
  j1 = i64_load(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0 + 16), j1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
  i2 = 5u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 16), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 1));
  i2 = 68u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 17), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 2));
  i2 = 61u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 18), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 3));
  i2 = 40u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 19), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 4));
  i2 = 7u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 5));
  i2 = 107u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 21), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 6));
  i2 = 22u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 22), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 7));
  i2 = 41u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 23), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 8));
  i2 = 22u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 24), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 9));
  i2 = 29u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 25), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 10));
  i2 = 61u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 26), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 11));
  i2 = 64u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 27), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 12));
  i2 = 53u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 28), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 13));
  i2 = 54u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 29), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 14));
  i32_store8(Z_envZ_memory, (u64)(i0 + 30), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 15));
  i2 = 66u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 31), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 16));
  i2 = 54u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 32), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 17));
  i2 = 49u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 33), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 18));
  i2 = 20u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 34), i1);
  i0 = l1;
  i1 = p0;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 19));
  i2 = 55u;
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 35), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 36));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 20));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 36), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 37));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 21));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 37), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 38));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 22));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 38), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 39));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 23));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 39), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 40));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 24));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 40), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 41));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 25));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 41), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 42));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 26));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 42), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 43));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 27));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 43), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 44));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 28));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 44), i1);
  i0 = l1;
  i1 = l1;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 45));
  i2 = p0;
  i2 = i32_load8_u(Z_envZ_memory, (u64)(i2 + 29));
  i1 ^= i2;
  i32_store8(Z_envZ_memory, (u64)(i0 + 45), i1);
  i0 = l1;
  i1 = l1;
  i2 = 16u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = l1;
  f23(i0);
  i0 = l1;
  i1 = 48u;
  i0 += i1;
  g0_stack = i0;
  i0 = 1247u;
  FUNC_EPILOGUE;
  return i0;
}

static void f20(u32 p0, u32 p1, u32 p2) {
  u32 l3 = 0, l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p2;
  i1 = 8192u;
  i0 = i0 >= i1;
  if (i0) {
    i0 = p0;
    i1 = p1;
    i2 = p2;
    i0 = (*Z_envZ_aZ_iiii)(i0, i1, i2);
    goto Bfunc;
  }
  i0 = p0;
  i1 = p2;
  i0 += i1;
  l3 = i0;
  i0 = p0;
  i1 = p1;
  i0 ^= i1;
  i1 = 3u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {
    i0 = p2;
    i1 = 1u;
    i0 = (u32)((s32)i0 < (s32)i1);
    if (i0) {
      i0 = p0;
      p2 = i0;
      goto B3;
    }
    i0 = p0;
    i1 = 3u;
    i0 &= i1;
    i0 = !(i0);
    if (i0) {
      i0 = p0;
      p2 = i0;
      goto B3;
    }
    i0 = p0;
    p2 = i0;
    L6:
      i0 = p2;
      i1 = p1;
      i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p1;
      i1 = 1u;
      i0 += i1;
      p1 = i0;
      i0 = p2;
      i1 = 1u;
      i0 += i1;
      p2 = i0;
      i1 = l3;
      i0 = i0 >= i1;
      if (i0) {goto B3;}
      i0 = p2;
      i1 = 3u;
      i0 &= i1;
      if (i0) {goto L6;}
    B3:;
    i0 = l3;
    i1 = 4294967292u;
    i0 &= i1;
    p0 = i0;
    i1 = 64u;
    i0 = i0 < i1;
    if (i0) {goto B7;}
    i0 = p2;
    i1 = p0;
    i2 = 4294967232u;
    i1 += i2;
    l4 = i1;
    i0 = i0 > i1;
    if (i0) {goto B7;}
    L8:
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 4));
      i32_store(Z_envZ_memory, (u64)(i0 + 4), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 8));
      i32_store(Z_envZ_memory, (u64)(i0 + 8), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 12));
      i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 16));
      i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 20));
      i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 24));
      i32_store(Z_envZ_memory, (u64)(i0 + 24), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 28));
      i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 32));
      i32_store(Z_envZ_memory, (u64)(i0 + 32), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 36));
      i32_store(Z_envZ_memory, (u64)(i0 + 36), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 40));
      i32_store(Z_envZ_memory, (u64)(i0 + 40), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 44));
      i32_store(Z_envZ_memory, (u64)(i0 + 44), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 48));
      i32_store(Z_envZ_memory, (u64)(i0 + 48), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 52));
      i32_store(Z_envZ_memory, (u64)(i0 + 52), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 56));
      i32_store(Z_envZ_memory, (u64)(i0 + 56), i1);
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1 + 60));
      i32_store(Z_envZ_memory, (u64)(i0 + 60), i1);
      i0 = p1;
      i1 = 4294967232u;
      i0 -= i1;
      p1 = i0;
      i0 = p2;
      i1 = 4294967232u;
      i0 -= i1;
      p2 = i0;
      i1 = l4;
      i0 = i0 <= i1;
      if (i0) {goto L8;}
    B7:;
    i0 = p2;
    i1 = p0;
    i0 = i0 >= i1;
    if (i0) {goto B1;}
    L9:
      i0 = p2;
      i1 = p1;
      i1 = i32_load(Z_envZ_memory, (u64)(i1));
      i32_store(Z_envZ_memory, (u64)(i0), i1);
      i0 = p1;
      i1 = 4u;
      i0 += i1;
      p1 = i0;
      i0 = p2;
      i1 = 4u;
      i0 += i1;
      p2 = i0;
      i1 = p0;
      i0 = i0 < i1;
      if (i0) {goto L9;}
    goto B1;
  }
  i0 = l3;
  i1 = 4u;
  i0 = i0 < i1;
  if (i0) {
    i0 = p0;
    p2 = i0;
    goto B1;
  }
  i0 = l3;
  i1 = 4294967292u;
  i0 += i1;
  l4 = i0;
  i1 = p0;
  i0 = i0 < i1;
  if (i0) {
    i0 = p0;
    p2 = i0;
    goto B1;
  }
  i0 = p0;
  p2 = i0;
  L12:
    i0 = p2;
    i1 = p1;
    i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
    i32_store8(Z_envZ_memory, (u64)(i0), i1);
    i0 = p2;
    i1 = p1;
    i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 1));
    i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
    i0 = p2;
    i1 = p1;
    i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 2));
    i32_store8(Z_envZ_memory, (u64)(i0 + 2), i1);
    i0 = p2;
    i1 = p1;
    i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 3));
    i32_store8(Z_envZ_memory, (u64)(i0 + 3), i1);
    i0 = p1;
    i1 = 4u;
    i0 += i1;
    p1 = i0;
    i0 = p2;
    i1 = 4u;
    i0 += i1;
    p2 = i0;
    i1 = l4;
    i0 = i0 <= i1;
    if (i0) {goto L12;}
  B1:;
  i0 = p2;
  i1 = l3;
  i0 = i0 < i1;
  if (i0) {
    L14:
      i0 = p2;
      i1 = p1;
      i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p1;
      i1 = 1u;
      i0 += i1;
      p1 = i0;
      i0 = p2;
      i1 = 1u;
      i0 += i1;
      p2 = i0;
      i1 = l3;
      i0 = i0 != i1;
      if (i0) {goto L14;}
  }
  Bfunc:;
  FUNC_EPILOGUE;
}

static u32 f21(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p1;
  i1 = 0u;
  i0 = i0 != i1;
  l2 = i0;
  i0 = p1;
  i0 = !(i0);
  if (i0) {goto B3;}
  i0 = p0;
  i1 = 3u;
  i0 &= i1;
  i0 = !(i0);
  if (i0) {goto B3;}
  L4:
    i0 = p0;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    i0 = !(i0);
    if (i0) {goto B2;}
    i0 = p0;
    i1 = 1u;
    i0 += i1;
    p0 = i0;
    i0 = p1;
    i1 = 4294967295u;
    i0 += i1;
    p1 = i0;
    i1 = 0u;
    i0 = i0 != i1;
    l2 = i0;
    i0 = p1;
    i0 = !(i0);
    if (i0) {goto B3;}
    i0 = p0;
    i1 = 3u;
    i0 &= i1;
    if (i0) {goto L4;}
  B3:;
  i0 = l2;
  i0 = !(i0);
  if (i0) {goto B1;}
  B2:;
  i0 = p0;
  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
  i0 = !(i0);
  if (i0) {goto B0;}
  i0 = p1;
  i1 = 4u;
  i0 = i0 >= i1;
  if (i0) {
    i0 = p1;
    i1 = 4294967292u;
    i0 += i1;
    l3 = i0;
    i1 = 3u;
    i0 &= i1;
    l2 = i0;
    i0 = l3;
    i1 = 4294967292u;
    i0 &= i1;
    i1 = p0;
    i0 += i1;
    i1 = 4u;
    i0 += i1;
    l3 = i0;
    L7:
      i0 = p0;
      i0 = i32_load(Z_envZ_memory, (u64)(i0));
      l4 = i0;
      i1 = 4294967295u;
      i0 ^= i1;
      i1 = l4;
      i2 = 4278124287u;
      i1 += i2;
      i0 &= i1;
      i1 = 2155905152u;
      i0 &= i1;
      if (i0) {goto B5;}
      i0 = p0;
      i1 = 4u;
      i0 += i1;
      p0 = i0;
      i0 = p1;
      i1 = 4294967292u;
      i0 += i1;
      p1 = i0;
      i1 = 3u;
      i0 = i0 > i1;
      if (i0) {goto L7;}
    i0 = l2;
    p1 = i0;
    i0 = l3;
    p0 = i0;
  }
  i0 = p1;
  i0 = !(i0);
  if (i0) {goto B1;}
  B5:;
  L8:
    i0 = p0;
    i0 = i32_load8_u(Z_envZ_memory, (u64)(i0));
    i0 = !(i0);
    if (i0) {goto B0;}
    i0 = p0;
    i1 = 1u;
    i0 += i1;
    p0 = i0;
    i0 = p1;
    i1 = 4294967295u;
    i0 += i1;
    p1 = i0;
    if (i0) {goto L8;}
  B1:;
  i0 = 0u;
  goto Bfunc;
  B0:;
  i0 = p0;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f22(u32 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  i0 = p0;
  if (i0) {
    i0 = p1;
    i1 = 127u;
    i0 = i0 <= i1;
    if (i0) {goto B0;}
    i0 = 2096u;
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i0 = i32_load(Z_envZ_memory, (u64)(i0));
    i0 = !(i0);
    if (i0) {
      i0 = p1;
      i1 = 4294967168u;
      i0 &= i1;
      i1 = 57216u;
      i0 = i0 == i1;
      if (i0) {goto B0;}
      goto B2;
    }
    i0 = p1;
    i1 = 2047u;
    i0 = i0 <= i1;
    if (i0) {
      i0 = p0;
      i1 = p1;
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
      i0 = p0;
      i1 = p1;
      i2 = 6u;
      i1 >>= (i2 & 31);
      i2 = 192u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = 2u;
      goto Bfunc;
    }
    i0 = p1;
    i1 = 55296u;
    i0 = i0 >= i1;
    i1 = 0u;
    i2 = p1;
    i3 = 4294959104u;
    i2 &= i3;
    i3 = 57344u;
    i2 = i2 != i3;
    i0 = i2 ? i0 : i1;
    i0 = !(i0);
    if (i0) {
      i0 = p0;
      i1 = p1;
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 2), i1);
      i0 = p0;
      i1 = p1;
      i2 = 12u;
      i1 >>= (i2 & 31);
      i2 = 224u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i2 = 6u;
      i1 >>= (i2 & 31);
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
      i0 = 3u;
      goto Bfunc;
    }
    i0 = p1;
    i1 = 4294901760u;
    i0 += i1;
    i1 = 1048575u;
    i0 = i0 <= i1;
    if (i0) {
      i0 = p0;
      i1 = p1;
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 3), i1);
      i0 = p0;
      i1 = p1;
      i2 = 18u;
      i1 >>= (i2 & 31);
      i2 = 240u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = p0;
      i1 = p1;
      i2 = 6u;
      i1 >>= (i2 & 31);
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 2), i1);
      i0 = p0;
      i1 = p1;
      i2 = 12u;
      i1 >>= (i2 & 31);
      i2 = 63u;
      i1 &= i2;
      i2 = 128u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0 + 1), i1);
      i0 = 4u;
      goto Bfunc;
    }
    B2:;
    i0 = 3192u;
    i1 = 25u;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
    i0 = 4294967295u;
  } else {
    i0 = 1u;
  }
  goto Bfunc;
  B0:;
  i0 = p0;
  i1 = p1;
  i32_store8(Z_envZ_memory, (u64)(i0), i1);
  i0 = 1u;
  Bfunc:;
  FUNC_EPILOGUE;
  return i0;
}

static void f23(u32 p0) {
  u32 l1 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1;
  i0 = g0_stack;
  i1 = 16u;
  i0 -= i1;
  l1 = i0;
  g0_stack = i0;
  i0 = l1;
  i1 = p0;
  i32_store(Z_envZ_memory, (u64)(i0 + 12), i1);
  i0 = 1260u;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  i1 = p0;
  f28(i0, i1);
  i0 = l1;
  i1 = 16u;
  i0 += i1;
  g0_stack = i0;
  FUNC_EPILOGUE;
}

static u32 f24(u64 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0;
  u64 l5 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3;
  u64 j0, j1, j2, j3;
  j0 = p0;
  j1 = 4294967296ull;
  i0 = j0 < j1;
  if (i0) {
    j0 = p0;
    l5 = j0;
    goto B0;
  }
  L2:
    i0 = p1;
    i1 = 4294967295u;
    i0 += i1;
    p1 = i0;
    j1 = p0;
    j2 = p0;
    j3 = 10ull;
    j2 = DIV_U(j2, j3);
    l5 = j2;
    j3 = 10ull;
    j2 *= j3;
    j1 -= j2;
    i1 = (u32)(j1);
    i2 = 48u;
    i1 |= i2;
    i32_store8(Z_envZ_memory, (u64)(i0), i1);
    j0 = p0;
    j1 = 42949672959ull;
    i0 = j0 > j1;
    l2 = i0;
    j0 = l5;
    p0 = j0;
    i0 = l2;
    if (i0) {goto L2;}
  B0:;
  j0 = l5;
  i0 = (u32)(j0);
  l2 = i0;
  if (i0) {
    L4:
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      i1 = l2;
      i2 = l2;
      i3 = 10u;
      i2 = DIV_U(i2, i3);
      l3 = i2;
      i3 = 10u;
      i2 *= i3;
      i1 -= i2;
      i2 = 48u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      i0 = l2;
      i1 = 9u;
      i0 = i0 > i1;
      l4 = i0;
      i0 = l3;
      l2 = i0;
      i0 = l4;
      if (i0) {goto L4;}
  }
  i0 = p1;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f25_e_k_check_passwd(u32 p0_passwd) {
  u32 l1 = 0, l2 = 0, l3 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1;
  u64 j1;
  i0 = g0_stack;
  i1 = 48u;
  i0 -= i1;
  l2 = i0;
  g0_stack = i0;
  // g0 -= 48
  i0 = l2;
  i1 = 0u;
  i32_store8(Z_envZ_memory, (u64)(i0 + 32), i1);
  i0 = l2;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 24), j1);
  i0 = l2;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 16), j1);
  i0 = l2;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0 + 8), j1);
  i0 = l2;
  j1 = 0ull;
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  i0 = 1189u;
  l3 = i0;
  i0 = p0_passwd;
  i0 = f9_maybe_strlen(i0);
  i1 = 15u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  // assert len(passwd) == 15
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 1));
  i0 += i1;
  i1 = 225u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  // assert passwd[0] + passwd[1] == 255
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 2));
  i1 = l1;
  i0 += i1;
  i1 = 190u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  // passwd[0] + passwd[2] == 190
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 3));
  i1 = l1;
  i0 += i1;
  i1 = 190u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  // [0] + [3] == 190
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 4));
  i1 = l1;
  i0 += i1;
  i1 = 197u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  // [0] + [4] == 197
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 5));
  i1 = l1;
  i0 += i1;
  i1 = 197u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  // [0] + [5] == 197
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 6));
  i1 = l1;
  i0 += i1;
  i1 = 222u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  // [0] + [6] == 222
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 7));
  i1 = l1;
  i0 += i1;
  i1 = 187u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  i1 = l1;
  i0 += i1;
  i1 = 164u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 214u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 194u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 222u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 179u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 214u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 158u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 1));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 2));
  i0 += i1;
  i1 = 197u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 3));
  i1 = l1;
  i0 += i1;
  i1 = 197u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 4));
  i1 = l1;
  i0 += i1;
  i1 = 204u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 5));
  i1 = l1;
  i0 += i1;
  i1 = 204u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 6));
  i1 = l1;
  i0 += i1;
  i1 = 229u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 7));
  i1 = l1;
  i0 += i1;
  i1 = 194u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  i1 = l1;
  i0 += i1;
  i1 = 171u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 221u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 201u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 229u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 186u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 221u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 165u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 2));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 3));
  i0 += i1;
  i1 = 162u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 4));
  i1 = l1;
  i0 += i1;
  i1 = 169u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 5));
  i1 = l1;
  i0 += i1;
  i1 = 169u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 6));
  i1 = l1;
  i0 += i1;
  i1 = 194u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 7));
  i1 = l1;
  i0 += i1;
  i1 = 159u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  i1 = l1;
  i0 += i1;
  i1 = 136u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 186u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 166u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 194u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 151u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 186u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 130u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 3));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 4));
  i0 += i1;
  i1 = 169u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 5));
  i1 = l1;
  i0 += i1;
  i1 = 169u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 6));
  i1 = l1;
  i0 += i1;
  i1 = 194u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 7));
  i1 = l1;
  i0 += i1;
  i1 = 159u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  i1 = l1;
  i0 += i1;
  i1 = 136u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 186u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 166u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 194u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 151u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 186u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 130u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 4));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 5));
  i0 += i1;
  i1 = 176u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 6));
  i1 = l1;
  i0 += i1;
  i1 = 201u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 7));
  i1 = l1;
  i0 += i1;
  i1 = 166u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  i1 = l1;
  i0 += i1;
  i1 = 143u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 193u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 173u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 201u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 158u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 193u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 137u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 5));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 6));
  i0 += i1;
  i1 = 201u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 7));
  i1 = l1;
  i0 += i1;
  i1 = 166u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  i1 = l1;
  i0 += i1;
  i1 = 143u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 193u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 173u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 201u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 158u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 193u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 137u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 6));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 7));
  i0 += i1;
  i1 = 191u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  i1 = l1;
  i0 += i1;
  i1 = 168u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 218u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 198u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 226u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 183u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 218u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 162u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 7));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 8));
  i0 += i1;
  i1 = 133u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  i1 = l1;
  i0 += i1;
  i1 = 183u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 163u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 191u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 148u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 183u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 127u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 8));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 9));
  i0 += i1;
  i1 = 160u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  i1 = l1;
  i0 += i1;
  i1 = 140u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 168u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 125u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 160u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 104u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 9));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 10));
  i0 += i1;
  i1 = 190u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  i1 = l1;
  i0 += i1;
  i1 = 218u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 175u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 210u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 154u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 10));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 11));
  i0 += i1;
  i1 = 198u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  i1 = l1;
  i0 += i1;
  i1 = 155u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 190u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 134u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 11));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 12));
  i0 += i1;
  i1 = 183u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  i1 = l1;
  i0 += i1;
  i1 = 218u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 162u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 12));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 13));
  i0 += i1;
  i1 = 175u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 14));
  i1 = l1;
  i0 += i1;
  i1 = 119u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = p0_passwd;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 13));
  l1 = i0;
  i1 = p0_passwd;
  i1 = i32_load8_s(Z_envZ_memory, (u64)(i1 + 14));
  i0 += i1;
  i1 = 154u;
  i0 = i0 != i1;
  if (i0) {goto B0;}
  i0 = l2;
  i1 = p0_passwd;
  j1 = i64_load(Z_envZ_memory, (u64)(i1));
  i64_store(Z_envZ_memory, (u64)(i0), j1);
  i0 = l2;
  i1 = p0_passwd;
  j1 = i64_load(Z_envZ_memory, (u64)(i1 + 7));
  i64_store(Z_envZ_memory, (u64)(i0 + 7), j1);
  i0 = p0_passwd;
  i0 = i32_load8_u(Z_envZ_memory, (u64)(i0 + 14));
  l3 = i0;
  i0 = l2;
  i1 = l1;
  i32_store8(Z_envZ_memory, (u64)(i0 + 16), i1);
  i0 = l2;
  i1 = l3;
  i32_store8(Z_envZ_memory, (u64)(i0 + 15), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 12));
  i32_store8(Z_envZ_memory, (u64)(i0 + 17), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 11));
  i32_store8(Z_envZ_memory, (u64)(i0 + 18), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 10));
  i32_store8(Z_envZ_memory, (u64)(i0 + 19), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 9));
  i32_store8(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 8));
  i32_store8(Z_envZ_memory, (u64)(i0 + 21), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 7));
  i32_store8(Z_envZ_memory, (u64)(i0 + 22), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 6));
  i32_store8(Z_envZ_memory, (u64)(i0 + 23), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 5));
  i32_store8(Z_envZ_memory, (u64)(i0 + 24), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 4));
  i32_store8(Z_envZ_memory, (u64)(i0 + 25), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 3));
  i32_store8(Z_envZ_memory, (u64)(i0 + 26), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 2));
  i32_store8(Z_envZ_memory, (u64)(i0 + 27), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1 + 1));
  i32_store8(Z_envZ_memory, (u64)(i0 + 28), i1);
  i0 = l2;
  i1 = p0_passwd;
  i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
  i32_store8(Z_envZ_memory, (u64)(i0 + 29), i1);
  i0 = l2;
  l3 = i0;
  B0:;
  i0 = l2;
  i1 = 48u;
  i0 += i1;
  g0_stack = i0;
  i0 = l3;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f26(u64 p0, u32 p1, u32 p2) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j0, j1;
  j0 = p0;
  i0 = !(j0);
  i0 = !(i0);
  if (i0) {
    L1:
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      j1 = p0;
      i1 = (u32)(j1);
      i2 = 15u;
      i1 &= i2;
      i2 = 1760u;
      i1 += i2;
      i1 = i32_load8_u(Z_envZ_memory, (u64)(i1));
      i2 = p2;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      j0 = p0;
      j1 = 4ull;
      j0 >>= (j1 & 63);
      p0 = j0;
      j1 = 0ull;
      i0 = j0 != j1;
      if (i0) {goto L1;}
  }
  i0 = p1;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f27(u64 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  u64 j0, j1;
  j0 = p0;
  i0 = !(j0);
  i0 = !(i0);
  if (i0) {
    L1:
      i0 = p1;
      i1 = 4294967295u;
      i0 += i1;
      p1 = i0;
      j1 = p0;
      i1 = (u32)(j1);
      i2 = 7u;
      i1 &= i2;
      i2 = 48u;
      i1 |= i2;
      i32_store8(Z_envZ_memory, (u64)(i0), i1);
      j0 = p0;
      j1 = 3ull;
      j0 >>= (j1 & 63);
      p0 = j0;
      j1 = 0ull;
      i0 = j0 != j1;
      if (i0) {goto L1;}
  }
  i0 = p1;
  FUNC_EPILOGUE;
  return i0;
}

static void f28(u32 p0, u32 p1) {
  u32 l2 = 0, l3 = 0, l4 = 0;
  FUNC_PROLOGUE;
  u32 i0, i1, i2, i3, i4;
  i0 = g0_stack;
  i1 = 208u;
  i0 -= i1;
  l2 = i0;
  g0_stack = i0;
  i0 = l2;
  i1 = p1;
  i32_store(Z_envZ_memory, (u64)(i0 + 204), i1);
  i0 = 0u;
  p1 = i0;
  i0 = l2;
  i1 = 160u;
  i0 += i1;
  i1 = 0u;
  i2 = 40u;
  f8(i0, i1, i2);
  i0 = l2;
  i1 = l2;
  i1 = i32_load(Z_envZ_memory, (u64)(i1 + 204));
  i32_store(Z_envZ_memory, (u64)(i0 + 200), i1);
  i0 = 0u;
  i1 = l2;
  i2 = 200u;
  i1 += i2;
  i2 = l2;
  i3 = 80u;
  i2 += i3;
  i3 = l2;
  i4 = 160u;
  i3 += i4;
  i0 = f5(i0, i1, i2, i3);
  i1 = 0u;
  i0 = (u32)((s32)i0 < (s32)i1);
  if (i0) {goto B0;}
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 76));
  i1 = 0u;
  i0 = (u32)((s32)i0 >= (s32)i1);
  if (i0) {
    i0 = 1u;
    p1 = i0;
  }
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0));
  l3 = i0;
  i0 = p0;
  i0 = i32_load8_s(Z_envZ_memory, (u64)(i0 + 74));
  i1 = 0u;
  i0 = (u32)((s32)i0 <= (s32)i1);
  if (i0) {
    i0 = p0;
    i1 = l3;
    i2 = 4294967263u;
    i1 &= i2;
    i32_store(Z_envZ_memory, (u64)(i0), i1);
  }
  i0 = l3;
  i1 = 32u;
  i0 &= i1;
  l4 = i0;
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 48));
  if (i0) {
    i0 = p0;
    i1 = l2;
    i2 = 200u;
    i1 += i2;
    i2 = l2;
    i3 = 80u;
    i2 += i3;
    i3 = l2;
    i4 = 160u;
    i3 += i4;
    i0 = f5(i0, i1, i2, i3);
    goto B3;
  }
  i0 = p0;
  i1 = 80u;
  i32_store(Z_envZ_memory, (u64)(i0 + 48), i1);
  i0 = p0;
  i1 = l2;
  i2 = 80u;
  i1 += i2;
  i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
  i0 = p0;
  i1 = l2;
  i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
  i0 = p0;
  i1 = l2;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 44));
  l3 = i0;
  i0 = p0;
  i1 = l2;
  i32_store(Z_envZ_memory, (u64)(i0 + 44), i1);
  i0 = p0;
  i1 = l2;
  i2 = 200u;
  i1 += i2;
  i2 = l2;
  i3 = 80u;
  i2 += i3;
  i3 = l2;
  i4 = 160u;
  i3 += i4;
  i0 = f5(i0, i1, i2, i3);
  i1 = l3;
  i1 = !(i1);
  if (i1) {goto B3;}
  i0 = p0;
  i1 = 0u;
  i2 = 0u;
  i3 = p0;
  i3 = i32_load(Z_envZ_memory, (u64)(i3 + 36));
  i0 = CALL_INDIRECT((*Z_envZ_table), u32 (*)(u32, u32, u32), 0, i3, i0, i1, i2);
  i0 = p0;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0 + 48), i1);
  i0 = p0;
  i1 = l3;
  i32_store(Z_envZ_memory, (u64)(i0 + 44), i1);
  i0 = p0;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0 + 28), i1);
  i0 = p0;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0 + 16), i1);
  i0 = p0;
  i0 = i32_load(Z_envZ_memory, (u64)(i0 + 20));
  i0 = p0;
  i1 = 0u;
  i32_store(Z_envZ_memory, (u64)(i0 + 20), i1);
  i0 = 0u;
  B3:;
  i0 = p0;
  i1 = p0;
  i1 = i32_load(Z_envZ_memory, (u64)(i1));
  i2 = l4;
  i1 |= i2;
  i32_store(Z_envZ_memory, (u64)(i0), i1);
  i0 = p1;
  i0 = !(i0);
  if (i0) {goto B0;}
  B0:;
  i0 = l2;
  i1 = 208u;
  i0 += i1;
  g0_stack = i0;
  FUNC_EPILOGUE;
}

static u64 f29(u32 p0, u64 p1, u32 p2) {
  FUNC_PROLOGUE;
  u64 j0;
  j0 = 0ull;
  FUNC_EPILOGUE;
  return j0;
}

static u32 f30(u32 p0) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = 0u;
  FUNC_EPILOGUE;
  return i0;
}

static u32 f31_main(u32 p0, u32 p1) {
  FUNC_PROLOGUE;
  u32 i0;
  i0 = 1024u;
  f6(i0);
  i0 = 1100u;
  f6(i0);
  i0 = 0u;
  FUNC_EPILOGUE;
  return i0;
}

static void ___wasm_call_ctors(void) {
  FUNC_PROLOGUE;
  FUNC_EPILOGUE;
}

static const u8 data_segment_data_0[] = {
  0x4f, 0x6e, 0x6c, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20, 0x68, 0x61, 0x63,
  0x6b, 0x65, 0x72, 0x73, 0x20, 0x77, 0x68, 0x6f, 0x20, 0x62, 0x72, 0x65,
  0x61, 0x6b, 0x20, 0x74, 0x68, 0x65, 0x20, 0x72, 0x75, 0x6c, 0x65, 0x73,
  0x20, 0x61, 0x72, 0x65, 0x20, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x74, 0x6f,
  0x20, 0x6f, 0x70, 0x65, 0x6e, 0x20, 0x6d, 0x79, 0x20, 0x76, 0x61, 0x75,
  0x6c, 0x74, 0x2e, 0xf0, 0x9f, 0x98, 0x98, 0xf0, 0x9f, 0x98, 0x98, 0xf0,
  0x9f, 0x98, 0x98, 0x00, 0x66, 0x6f, 0x72, 0x20, 0x6d, 0x79, 0x73, 0x65,
  0x6c, 0x66, 0x3a, 0x20, 0x54, 0x68, 0x65, 0x20, 0x63, 0x68, 0x61, 0x72,
  0x73, 0x65, 0x74, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x70,
  0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0x20, 0x69, 0x73, 0x20, 0x5b,
  0x41, 0x2d, 0x5a, 0x61, 0x2d, 0x7a, 0x30, 0x2d, 0x39, 0x5d, 0x2c, 0x20,
  0x74, 0x68, 0x65, 0x20, 0x6b, 0x65, 0x79, 0x62, 0x6f, 0x61, 0x72, 0x64,
  0x20, 0x74, 0x68, 0x65, 0x72, 0x65, 0x20, 0x6a, 0x75, 0x73, 0x74, 0x20,
  0x66, 0x6f, 0x72, 0x20, 0x66, 0x75, 0x6e, 0x2e,
};

static const u8 data_segment_data_1[] = {
  0x05, 0x44, 0x3d, 0x28, 0x07, 0x6b, 0x16, 0x29, 0x16, 0x1d, 0x3d, 0x40,
  0x35, 0x36, 0x00, 0x42, 0x36, 0x31, 0x14, 0x37, 0x08, 0x44, 0x3d, 0x14,
  0x35, 0x3a, 0x3d, 0x28, 0x55, 0x4c, 0x20, 0x2d, 0x2d, 0x20, 0x45, 0x4f,
  0x46, 0x7b, 0x25, 0x73, 0x7d, 0x20, 0x2d, 0x2d, 0x20, 0x0a, 0x00, 0x59,
  0x6f, 0x75, 0x20, 0x67, 0x6f, 0x74, 0x20, 0x69, 0x74, 0x00, 0x00, 0x00,
  0xf0, 0x06, 0x00, 0x00, 0x2d, 0x2b, 0x20, 0x20, 0x20, 0x30, 0x58, 0x30,
  0x78, 0x00, 0x28, 0x6e, 0x75, 0x6c, 0x6c, 0x29,
};

static const u8 data_segment_data_2[] = {
  0x11, 0x00, 0x0a, 0x00, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x0b,

};

static const u8 data_segment_data_3[] = {
  0x11, 0x00, 0x0f, 0x0a, 0x11, 0x11, 0x11, 0x03, 0x0a, 0x07, 0x00, 0x01,
  0x13, 0x09, 0x0b, 0x0b, 0x00, 0x00, 0x09, 0x06, 0x0b, 0x00, 0x00, 0x0b,
  0x00, 0x06, 0x11, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11,
};

static const u8 data_segment_data_4[] = {
  0x0b,
};

static const u8 data_segment_data_5[] = {
  0x11, 0x00, 0x0a, 0x0a, 0x11, 0x11, 0x11, 0x00, 0x0a, 0x00, 0x00, 0x02,
  0x00, 0x09, 0x0b, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0b, 0x00, 0x00, 0x0b,

};

static const u8 data_segment_data_6[] = {
  0x0c,
};

static const u8 data_segment_data_7[] = {
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0c,
};

static const u8 data_segment_data_8[] = {
  0x0e,
};

static const u8 data_segment_data_9[] = {
  0x0d, 0x00, 0x00, 0x00, 0x04, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0e,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x0e,
};

static const u8 data_segment_data_10[] = {
  0x10,
};

static const u8 data_segment_data_11[] = {
  0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x09, 0x10,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x12,
  0x00, 0x00, 0x00, 0x12, 0x12, 0x12,
};

static const u8 data_segment_data_12[] = {
  0x12, 0x00, 0x00, 0x00, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x09,
};

static const u8 data_segment_data_13[] = {
  0x0b,
};

static const u8 data_segment_data_14[] = {
  0x0a, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0b,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x0b,
};

static const u8 data_segment_data_15[] = {
  0x0c,
};

static const u8 data_segment_data_16[] = {
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x30,
  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43,
  0x44, 0x45, 0x46,
};

static const u8 data_segment_data_17[] = {
  0x05,
};

static const u8 data_segment_data_18[] = {
  0x01,
};

static const u8 data_segment_data_19[] = {
  0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x78, 0x08, 0x00, 0x00,
  0x00, 0x04,
};

static const u8 data_segment_data_20[] = {
  0x01,
};

static const u8 data_segment_data_21[] = {
  0x0a, 0xff, 0xff, 0xff, 0xff,
};

static const u8 data_segment_data_22[] = {
  0xa4, 0x0c,
};

static void init_memory(void) {
  Z_envZ_memory = malloc(sizeof(wasm_rt_memory_t));
  wasm_rt_allocate_memory(Z_envZ_memory, 1, 1);

  // store argv[1] at 0x3000
  memcpy(&((*Z_envZ_memory).data[0x3000]), global_arg1, 16);

  memcpy(&((*Z_envZ_memory).data[1024u]), data_segment_data_0, 164);
  memcpy(&((*Z_envZ_memory).data[1200u]), data_segment_data_1, 80);
  memcpy(&((*Z_envZ_memory).data[1296u]), data_segment_data_2, 24);
  memcpy(&((*Z_envZ_memory).data[1328u]), data_segment_data_3, 33);
  memcpy(&((*Z_envZ_memory).data[1377u]), data_segment_data_4, 1);
  memcpy(&((*Z_envZ_memory).data[1386u]), data_segment_data_5, 24);
  memcpy(&((*Z_envZ_memory).data[1435u]), data_segment_data_6, 1);
  memcpy(&((*Z_envZ_memory).data[1447u]), data_segment_data_7, 21);
  memcpy(&((*Z_envZ_memory).data[1493u]), data_segment_data_8, 1);
  memcpy(&((*Z_envZ_memory).data[1505u]), data_segment_data_9, 21);
  memcpy(&((*Z_envZ_memory).data[1551u]), data_segment_data_10, 1);
  memcpy(&((*Z_envZ_memory).data[1563u]), data_segment_data_11, 30);
  memcpy(&((*Z_envZ_memory).data[1618u]), data_segment_data_12, 14);
  memcpy(&((*Z_envZ_memory).data[1667u]), data_segment_data_13, 1);
  memcpy(&((*Z_envZ_memory).data[1679u]), data_segment_data_14, 21);
  memcpy(&((*Z_envZ_memory).data[1725u]), data_segment_data_15, 1);
  memcpy(&((*Z_envZ_memory).data[1737u]), data_segment_data_16, 39);
  memcpy(&((*Z_envZ_memory).data[1776u]), data_segment_data_17, 1);
  memcpy(&((*Z_envZ_memory).data[1788u]), data_segment_data_18, 1);
  memcpy(&((*Z_envZ_memory).data[1812u]), data_segment_data_19, 14);
  memcpy(&((*Z_envZ_memory).data[1836u]), data_segment_data_20, 1);
  memcpy(&((*Z_envZ_memory).data[1851u]), data_segment_data_21, 5);
  memcpy(&((*Z_envZ_memory).data[2096u]), data_segment_data_22, 2);
}

static void init_table(void) {
  Z_envZ_table = malloc(sizeof(wasm_rt_table_t));
  wasm_rt_allocate_table(Z_envZ_table, 4, 4);
  uint32_t offset;
  offset = 1u;
  (*Z_envZ_table).data[offset + 0] = (wasm_rt_elem_t){func_types[1], (wasm_rt_anyfunc_t)(&f30)};
  (*Z_envZ_table).data[offset + 1] = (wasm_rt_elem_t){func_types[0], (wasm_rt_anyfunc_t)(&f13)};
  (*Z_envZ_table).data[offset + 2] = (wasm_rt_elem_t){func_types[13], (wasm_rt_anyfunc_t)(&f29)};
}

/* export: 'c' */
void (*WASM_RT_ADD_PREFIX(Z_cZ_vv))(void);
/* export: 'd' */
u32 (*WASM_RT_ADD_PREFIX(Z_dZ_iii))(u32, u32);
/* export: 'e' */
u32 (*WASM_RT_ADD_PREFIX(Z_eZ_ii))(u32);
/* export: 'f' */
u32 (*WASM_RT_ADD_PREFIX(Z_fZ_ii))(u32);
/* export: 'g' */
u32 (*WASM_RT_ADD_PREFIX(Z_gZ_iv))(void);
/* export: 'h' */
u32 (*WASM_RT_ADD_PREFIX(Z_hZ_ii))(u32);
/* export: 'i' */
void (*WASM_RT_ADD_PREFIX(Z_iZ_vi))(u32);

static void init_exports(void) {
  /* export: 'c' */
  WASM_RT_ADD_PREFIX(Z_cZ_vv) = (&___wasm_call_ctors);
  /* export: 'd' */
  WASM_RT_ADD_PREFIX(Z_dZ_iii) = (&f31_main);
  /* export: 'e' */
  WASM_RT_ADD_PREFIX(Z_eZ_ii) = (&f25_e_k_check_passwd);
  /* export: 'f' */
  WASM_RT_ADD_PREFIX(Z_fZ_ii) = (&f19_v);
  /* export: 'g' */
  WASM_RT_ADD_PREFIX(Z_gZ_iv) = (&f18_stackSave);
  /* export: 'h' */
  WASM_RT_ADD_PREFIX(Z_hZ_ii) = (&f17_stackAlloc);
  /* export: 'i' */
  WASM_RT_ADD_PREFIX(Z_iZ_vi) = (&f16_stackRestore);
}

void WASM_RT_ADD_PREFIX(init)(void) {
  init_func_types();
  init_globals();
  init_memory();
  init_table();
  init_exports();
}
