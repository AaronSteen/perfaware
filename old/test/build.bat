@echo off

set "IGNORE_WARNINGS=-wd4100 -wd4189 -wd4200"

if not exist build (
    mkdir build
)

del *.exe
del *.asm

pushd build

del *.ilk *.obj *.pdb

cl -Zi -W4 -WX -Od -nologo %IGNORE_WARNINGS% %~dp0test.c /link /DEBUG:FULL

popd
