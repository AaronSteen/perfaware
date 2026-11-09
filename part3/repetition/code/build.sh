#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BUILD="$(cd "$SCRIPT_DIR/.." && pwd)/build"

# Build mode: "debug" (default, no optimization) or "release" (-O2).
# Neovim's :make passes its arguments through, so ":make release" runs "./build.sh release".
MODE="${1:-debug}"
if [ "$MODE" != "debug" ] && [ "$MODE" != "release" ]; then
    echo "build.sh: unknown mode '$MODE' (expected debug or release)"
    exit 1
fi

# Optional second word: "analyze" also runs the static analyzer after a successful
# compile. Left out, it's a plain compile. In Neovim: <leader>b = compile only,
# <leader>ab = compile + analyze (runs "./build.sh debug analyze").
ANALYZE="$2"
if [ -n "$ANALYZE" ] && [ "$ANALYZE" != "analyze" ]; then
    echo "build.sh: unknown option '$ANALYZE' (expected analyze, or nothing)"
    exit 1
fi

mkdir -p "$BUILD"

# compile_commands.json tells clangd exactly how each file is compiled, so it can
# index the whole project (this is what makes <leader>ss and gd reach files you
# haven't opened). The code that writes it is shared by all projects and lives in
# the dotfiles repo. If it's missing (e.g. a fresh clone on another machine), the
# build still works; clangd just won't see the whole project.
CDB_HELPER="$HOME/.dotfiles/scripts/compile_commands.sh"
if [ -f "$CDB_HELPER" ]; then
    source "$CDB_HELPER"
else
    echo "note: $CDB_HELPER not found, skipping compile_commands.json"
    write_compile_commands()      { :; }   # do-nothing stand-ins
    write_compile_commands_msvc() { :; }
fi

SCRIPT_WIN=$(wslpath -w "$SCRIPT_DIR/build_wsl.bat")

# build_wsl.bat compiles, then (only if asked to and the compile succeeded)
# runs the static analyzer. Both outputs come through this same sed, so analyzer findings get
# the same path and (line,col) -> :line:col conversion as compiler errors.
# "$ANALYZE" is either "analyze" or empty; build_wsl.bat reads it as %2.
cmd.exe /c "$SCRIPT_WIN" "$MODE" "$ANALYZE" 2>&1 | sed \
    -e 's|\r||g' \
    -e 's|C:\\Users\\ams56\\work\\|/mnt/c/Users/ams56/work/|g' \
    -e 's|\\|/|g' \
    -e 's|(\([0-9][0-9]*\),\([0-9][0-9]*\))|:\1:\2|g'

STATUS=${PIPESTATUS[0]}   # must be read immediately after the pipeline above

# build_wsl.bat leaves its compiler flags and header-folder list in build/;
# the shared helper turns them into compile_commands.json with WSL-style paths.
write_compile_commands_msvc "$BUILD" "$SCRIPT_DIR/read_overhead_test.cpp"

exit $STATUS
