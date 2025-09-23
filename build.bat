clang-cl /clang:-fdiagnostics-absolute-paths /Zi /Od /MTd /D_CRT_SECURE_NO_WARNINGS /FC /diagnostics:column /nologo ^
  main.c /link /DEBUG:FULL /PDB:build\app.pdb /OUT:build\app.exe
