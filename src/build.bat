@echo off

set DEBUG=1

pushd ..\build

set flags=/nologo /D_CRT_SECURE_NO_WARNINGS /Gm- /GR- /EHa- /Zi /FC /W4 /WX /wd4201 /wd4505
set linker_flags=/incremental:no

if %DEBUG% == 1 (
	set flags=%flags% /Od
) else (
	set flags=%flags% /O2
)

cl %flags% ..\src\main.cpp /link %linker_flags%

popd