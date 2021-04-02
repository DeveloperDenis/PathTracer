@echo off

pushd ..\build

set flags=/Od /nologo /D_CRT_SECURE_NO_WARNINGS /Gm- /GR- /EHa- /Zi /FC /W4 /WX /wd4201
set linker_flags=/incremental:no

cl %flags% ..\src\main.cpp /link %linker_flags%

popd