@echo off
if not exist .\build (
    mkdir .\build
)
pushd .\build
rem -Oi will turn on 'intrinsic optimizations'
rem -GR- will turn off runtime type info
rem -EHa- will turn off exception handling overhead
rem -MT will package the C runtime into our exe so we don't have to load the random dll that calls into our program. [For shipping imo]
rem -Gm- turns off any increment. builds that the compiler tries to do
rem TODO: MAYBE TURN ON -Od to remove all possible optimizations (prob only do this when casey wants to show some assembler stuff)
set CompilerFlags= -MTd /Zi /EHsc /FC -Oi -GR- -EHa- -Gm-
rem WX is supposed to represent don't compile with warnings. We will edit our warnings to only the ones we care about!
rem -Wd[warning#] is how to ignore certain warnings.
set WarningFlags=-WX -W4 -wd4201 -wd4100 -wd4189 -wd4505
ctime -begin bb_timing.ctm
cl -nologo %WarningFlags% -DNEO_SPEED=0 -DNEO_INTERNAL=1 %CompilerFlags% ..\src\plat\win32_nebula.cpp user32.lib gdi32.lib winmm.lib opengl32.lib
ctime -end bb_timing.ctm
popd
