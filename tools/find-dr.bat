@echo off
set dircounter=0

for /f "delims=\" %%a in ("%cd%") do set current-dir=%%~nxa

:LOOP
set /A dircounter=dircounter+1
if %dircounter% EQU 10 (
echo "Could not find any valid DragonRuby super directory. Make sure that this project is somewhere in your DragonRuby directory or any subdirectory of it!"
exit 1
)
if exist dragonruby.exe (
set dr-path=%CD%
) else (
cd..
if not exist dragonruby.exe (
for /f "delims=\" %%a in ("%cd%") do set current-dir=%%~nxa\%current-dir%
)
goto :LOOP
)

echo %dr-path%
exit 0