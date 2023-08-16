@echo off

where /q python3
IF ERRORLEVEL 1 (
    ECHO Error: Can't find Python3!
    EXIT /B
) ELSE (
    python3 tools/scripts/build.py %*
)