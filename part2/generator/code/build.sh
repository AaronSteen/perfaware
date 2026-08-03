#!/bin/bash

# process here is:
#   run "build" command in neovim to call this script
#   this script opens command prompt in the same directory and runs a .bat file which does the following:
#           runs VCVARSALL to set environment variables
#           makes a build directory in the top-level directory for the project if one doesn't exist
#           enters that build directory
#           calls clang-cl to compile the program
#           leaves the build directory and returns to the directory from which build.sh was called
#   control returns to build.sh at that point, which pipes the compiler output to sed, which makes these edits
#           to the compiler's output so that it is parseable by neovim's quickfix list:
#                   remove the windows carriage return characters at the ends of lines
#                   replace occurrences of the Windows version of the directory path (e.g., C:\\Users\\ams56... with
#                           its WSL equivalent (e.g., /mnt/c/Users/ams56...)
#                   replace all instances of windows-style \\ dir separators with linux equivalent /
#                   change line and character locations for error messages from the windows-formatted
#                           versions, e.g., 
#
#                           C:\Users\ams56\work\beaver\code\beaver.cpp(142,17): error: use of undeclared identifier
#
#                   with versions that use : as a delimiter so it's parseable by the quickfix list, e.g.:
#
#                           C:\Users\ams56\work\beaver\code\beaver.cpp:142:17: error: use of undeclared identifier
#

# store the directory containing the script that's running right now, so we can use it in the next line.
# this assumes that the batch script we're going to run to compile the program in command prompt
#       is in the same directory as build.sh that we call (i.e., the script running right now,
#       to start the whole process.)
SCRIPT_DIR=$(dirname "$(realpath "$0")")

# store the path to the build.bat we're going to call when we compile the program using command prompt
SCRIPT_WIN=$(wslpath -w "$SCRIPT_DIR/build_wsl.bat")

# open command prompt and run the script. pipe the output to sed.

cmd.exe /c "$SCRIPT_WIN" 2>&1 | sed \
    -e 's|\r||g' \
    -e 's|C:\\Users\\ams56\\work\\|/mnt/c/Users/ams56/work/|g' \
    -e 's|\\|/|g' \
    -e 's|(\([0-9][0-9]*\),\([0-9][0-9]*\))|:\1:\2|g'

exit ${PIPESTATUS[0]}
