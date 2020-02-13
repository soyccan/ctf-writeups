clang -g \
      -I../wabt/wasm2c/ \
      -I../wabt/third_party/wasm-c-api/include \
      ../wabt/wasm2c/wasm-rt-impl.c \
      vault.c \
      main.c
