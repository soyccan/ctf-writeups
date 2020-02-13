#ifndef VAULT_H_GENERATED_
#define VAULT_H_GENERATED_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "wasm-rt.h"

#ifndef WASM_RT_MODULE_PREFIX
#define WASM_RT_MODULE_PREFIX
#endif

#define WASM_RT_PASTE_(x, y) x ## y
#define WASM_RT_PASTE(x, y) WASM_RT_PASTE_(x, y)
#define WASM_RT_ADD_PREFIX(x) WASM_RT_PASTE(WASM_RT_MODULE_PREFIX, x)

/* TODO(binji): only use stdint.h types in header */
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;

extern void WASM_RT_ADD_PREFIX(init)(void);

/* import: 'env' 'a' */
extern u32 (*Z_envZ_aZ_iiii)(u32, u32, u32);
/* import: 'wasi_snapshot_preview1' 'b' */
extern u32 (*Z_wasi_snapshot_preview1Z_bZ_iiiii)(u32, u32, u32, u32);
/* import: 'env' 'memory' */
extern wasm_rt_memory_t (*Z_envZ_memory);
/* import: 'env' 'table' */
extern wasm_rt_table_t (*Z_envZ_table);

/* export: 'c' */
extern void (*WASM_RT_ADD_PREFIX(Z_cZ_vv))(void);
/* export: 'd' */
extern u32 (*WASM_RT_ADD_PREFIX(Z_dZ_iii))(u32, u32);
/* export: 'e' */
extern u32 (*WASM_RT_ADD_PREFIX(Z_eZ_ii))(u32);
/* export: 'f' */
extern u32 (*WASM_RT_ADD_PREFIX(Z_fZ_ii))(u32);
/* export: 'g' */
extern u32 (*WASM_RT_ADD_PREFIX(Z_gZ_iv))(void);
/* export: 'h' */
extern u32 (*WASM_RT_ADD_PREFIX(Z_hZ_ii))(u32);
/* export: 'i' */
extern void (*WASM_RT_ADD_PREFIX(Z_iZ_vi))(u32);
#ifdef __cplusplus
}
#endif

#endif  /* VAULT_H_GENERATED_ */
