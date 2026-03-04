@echo off
setlocal

REM ---- Usage Check ----
if "%1"=="" (
    echo.
    echo Usage:
    echo run_udp_test_sender.bat DEST_PORT [DEST_IP] [NUM_PACKETS] [INTERVAL_MS]
    echo.
    echo Example:
    echo run_udp_test_sender.bat 25001
    echo run_udp_test_sender.bat 25001 127.0.0.1 10 100
    echo.
    exit /b 1
)

set PORT=%1
set IP=%2
set COUNT=%3
set INTERVAL=%4

REM ---- Defaults ----
if "%IP%"=="" set IP=127.0.0.1
if "%COUNT%"=="" set COUNT=1
if "%INTERVAL%"=="" set INTERVAL=200

REM ---- Adjust path if needed ----
REM If executable is in build\bin\:
set EXE=udp_test_sender_boost.exe

REM If running from project root and exe is in build\bin:
if not exist "%EXE%" (
    if exist "build\bin\udp_test_sender_boost.exe" (
        set EXE=build\bin\udp_test_sender_boost.exe
    )
)

echo.
echo Sending to %IP%:%PORT%
echo Packets: %COUNT%
echo Interval: %INTERVAL% ms
echo.

"%EXE%" %IP% %PORT% %COUNT% %INTERVAL%

endlocal