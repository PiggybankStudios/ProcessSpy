@echo off

set BuildWindows=1
set BuildLinux=0

if "%BuildWindows%"=="1" (
	build_win32.bat
)

if "%BuildLinux%"=="1" (
	wsl ./build_linux.sh
)
