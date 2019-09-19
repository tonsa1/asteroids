@echo off

REM cl -Zi -FC -nologo user32.lib gdi32.lib  ..\Code\win32_asteroids.cpp

set CommonCompilerFlags= -O2 -MT -nologo -Gm- -GR -EHa-  -Oi -WX -W4 -wd4201 -wd4100 -wd4505 -wd4456 -wd4189 -wd4244 -wd4996 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib Kernel32.lib winmm.lib 

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\Code\asteroids.cpp -LD /link -incremental:no -opt:ref /PDB:asteroids%random%.pdb /DLL /EXPORT:GameUpdateAndRender
cl %CommonCompilerFlags% ..\Code\win32_asteroids.cpp -Fmwin32_asteroids.map /link %CommonLinkerFlags%

popd
