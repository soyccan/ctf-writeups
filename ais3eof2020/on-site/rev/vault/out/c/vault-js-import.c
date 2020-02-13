#include "vault.h"
#include <stdio.h>

/* import: 'env' 'a' */
u32 _emscripten_memcpy_big(u32 dest, u32 src, u32 num) {
    fprintf(stderr, "_emscripten_memcpy_big %p %p %d\n",dest,src,num);
}
u32 (*Z_envZ_aZ_iiii)(u32, u32, u32) = _emscripten_memcpy_big;


/* 'wasi_snapshot_preview1' 'b' */
    // printChar: function(stream, curr) {
    //     var buffer = SYSCALLS.buffers[stream];
    //     if (curr === 0 || curr === 10) {
    //         (stream === 1 ? out : err)(UTF8ArrayToString(buffer, 0));
    //         buffer.length = 0
    //     } else {
    //         buffer.push(curr)
    //     }
    // },
u32 _fd_write(u32 fd, u32 iov, u32 iovcnt, u32 pnum) {
    fprintf(stderr, "_fd_write %d %d %d %d\n",fd,iov,iovcnt,pnum);
        // u32 num = 0;
        // for (u32 i = 0; i < iovcnt; i++) {
        //     u32 ptr = HEAP32[iov + i * 8 >> 2];
        //     u32 len = HEAP32[iov + (i * 8 + 4) >> 2];
        //     for (u32 j = 0; j < len; j++) {
        //         SYSCALLS.printChar(fd, HEAPU8[ptr + j])
        //     }
        //     num += len;
        // }
        // HEAP32[pnum >> 2] = num;
        // return 0
}
u32 (*Z_wasi_snapshot_preview1Z_bZ_iiiii)(u32, u32, u32, u32) = _fd_write;

/* import: 'env' 'memory' */
wasm_rt_memory_t (*Z_envZ_memory);

/* import: 'env' 'table' */
wasm_rt_table_t (*Z_envZ_table);