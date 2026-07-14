#!/bin/bash
# Generates compile_commands.json at the project root for clangd.
# Run this whenever build flags change, or whenever a new file is added
# that's #include'd into main.c as part of the unity build -- it needs
# its own entry here, with -include flags replaying whatever main.c
# includes ahead of it, or clangd can't resolve types/symbols when you
# open it directly.
# Bear cannot be used here because the actual build runs Windows binaries via cmd.exe.
# Requires mingw-w64 for Windows headers: sudo apt install mingw-w64

SCRIPT_DIR=$(dirname "$(realpath "$0")")
ROOT=$(dirname "$SCRIPT_DIR")
BUILD="$ROOT/build"

FLAGS="-target x86_64-w64-mingw32 -isystem /usr/x86_64-w64-mingw32/include -std=c17 -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-missing-field-initializers -D_CRT_SECURE_NO_WARNINGS"

cat > "$ROOT/compile_commands.json" << EOF
[
  {
    "directory": "$BUILD",
    "command": "clang $FLAGS $SCRIPT_DIR/main.c",
    "file": "$SCRIPT_DIR/main.c"
},
  {
    "directory": "$BUILD",
    "command": "clang $FLAGS -include $SCRIPT_DIR/common.h -include $SCRIPT_DIR/tables.c $SCRIPT_DIR/source.c",
    "file": "$SCRIPT_DIR/source.c"
},
  {
    "directory": "$BUILD",
    "command": "clang $FLAGS -include $SCRIPT_DIR/common.h $SCRIPT_DIR/tables.c",
    "file": "$SCRIPT_DIR/tables.c"
}
]
EOF

echo "compile_commands.json written to $ROOT"
