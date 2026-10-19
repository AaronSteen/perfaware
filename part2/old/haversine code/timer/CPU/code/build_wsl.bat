@echo off
set VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat
set CLANGCL=C:\ProgramDirIMade\clang+llvm-21.1.4-x86_64-pc-windows-msvc\clang+llvm-21.1.4-x86_64-pc-windows-msvc\bin\clang-cl.exe
set CODE=C:\Users\ams56\work\pap\part2\timer\CPU\code
set BUILD=C:\Users\ams56\work\pap\part2\timer\CPU\build
set IncludeDirs=/IC:\Users\ams56\work\pap\part2\shared /IC:\Users\ams56\work\pap\part2\timer
set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -Wno-unused-function -Wno-writable-strings -wd4189 -Wno-unused-variable -Wno-sign-compare -Wno-missing-field-initializers -Wno-missing-braces -Wno-unused-but-set-variable -FC -Z7
set CommonLinkerFlags=-opt:ref -incremental:no /DEBUG

call "%VCVARSALL%" x64 > NUL 2>&1
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

if not exist %BUILD% mkdir %BUILD%
pushd %BUILD%
"%CLANGCL%" %IncludeDirs% %CommonCompilerFlags% %CODE%\cpu_timer_main.c /link %CommonLinkerFlags% /OUT:cpu_timer_main.exe
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
echo cpu_timer_main.exe: OK
popd
