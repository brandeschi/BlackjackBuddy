@echo off
if not exist .\build (
    mkdir .\build
)
pushd .\build
:: -Oi will turn on 'intrinsic optimizations'
:: -GR- will turn off runtime type info
:: -EHa- will turn off exception handling overhead
:: -MT will package the C runtime into our exe so we don't have to load the random dll that calls into our program. [For shipping imo]
:: -Gm- turns off any increment. builds that the compiler tries to do
:: TODO: MAYBE TURN ON -Od to remove all possible optimizations (prob only do this when casey wants to show some assembler stuff)
set CompilerFlags= -MTd /Zi /EHsc /FC -Oi -GR- -EHa- -Gm-
:: WX is supposed to represent don't compile with warnings. We will edit our warnings to only the ones we care about!
:: -Wd[warning#] is how to ignore certain warnings.
set WarningFlags=-WX -W4 -wd4201 -wd4100 -wd4189 -wd4505
cl -nologo %WarningFlags% -DNEO_SPEED=0 -DNEO_INTERNAL=1 %CompilerFlags% ..\src\win32_nebula.cpp user32.lib gdi32.lib winmm.lib opengl32.lib
popd
