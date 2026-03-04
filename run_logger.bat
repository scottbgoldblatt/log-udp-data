@echo off
setlocal enabledelayedexpansion

REM ===== Default values =====
set DEFAULT_IP=192.168.10.5
set DEFAULT_PORT=25001

REM ===== Resolve IP =====
if "%~1"=="" (
    set BIND_IP=%DEFAULT_IP%
) else (
    set BIND_IP=%~1
)

REM ===== Resolve PORT =====
if "%~2"=="" (
    set BIND_PORT=%DEFAULT_PORT%
) else (
    set BIND_PORT=%~2
)

REM ===== Check executable exists =====
if not exist "hrg_logger.exe" (
    echo ERROR: hrg_logger.exe not found in this folder.
    echo Make sure this .bat file is next to the executable.
    pause
    exit /b 1
)

echo ============================================
echo Starting hrg_logger
echo Bind IP   : %BIND_IP%
echo Bind Port : %BIND_PORT%
echo ============================================
echo.

REM Run the program
hrg_logger.exe %BIND_IP% %BIND_PORT%

echo.
echo Program exited.
pause