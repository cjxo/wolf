@echo off
setlocal enabledelayedexpansion
if not exist ..\build mkdir ..\build
pushd ..\build
ml64 /nologo /Zi /c ..\code\renderer.asm
cl /nologo /Oi /GS- /Zi /Od /Gs9999999 /W3 ..\code\main.c /DWOLF_DEBUG /Fawolf.asm /link /nodefaultlib /incremental:no /subsystem:windows /entry:EntryPoint /stack:0x100000,0x100000 kernel32.lib User32.lib winmm.lib gdi32.lib renderer.obj
popd
endlocal