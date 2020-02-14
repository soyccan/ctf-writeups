workspaceFolder=$(realpath $PWD/../../)
clang "${workspaceFolder}/wabt/wasm2c/wasm-rt-impl.c" \
      "${workspaceFolder}/out/c/vault.c" \
      "${workspaceFolder}/out/c/main.c" \
      "${workspaceFolder}/out/c/vault-js-import.c" \
      "-I${workspaceFolder}/wabt/wasm2c/" \
      "-I${workspaceFolder}/wabt/third_party/wasm-c-api/include" \
      "-o" "${workspaceFolder}/out/c/vault" \
      "-g" \
      "-m32" \
#       "-v" \
