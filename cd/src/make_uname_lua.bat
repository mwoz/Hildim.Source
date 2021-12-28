@echo off
REM This builds all the Lua libraries of the folder for 1 uname

REM Building for the default (USE_LUA51) 
REM or for the defined at the environment

call tecmake %1 "MF=cdlua5" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=cdluapdf5" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=cdluagl5" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=cdluaim5" %2 %3 %4 %5 %6 %7 %8

REM These are NOT available in some compilers
REM so this may result in errors, just ignore them
call tecmake %1 "MF=cdluacontextplus5" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=cdluacairo5" %2 %3 %4 %5 %6 %7 %8
call tecmake %1 "MF=cdluadirect2d5" %2 %3 %4 %5 %6 %7 %8
