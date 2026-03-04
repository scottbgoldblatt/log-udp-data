@echo off
setlocal

REM ===== Default network settings =====
set DEFAULT_IP=192.168.10.5
set DEFAULT_PORT=25001

echo.
echo ============================================
echo IMU LOGGER LAUNCHER
echo ============================================
echo.
echo Select logger type:
echo.
echo   1 - Binary Logger (recommended)
echo   2 - Text Logger
echo.

set /p CHOICE=Enter choice (1 or 2): 

echo.
set /p BIND_IP=Enter IP address [default %DEFAULT_IP%]: 
if "%BIND_IP%"=="" set BIND_IP=%DEFAULT_IP%

set /p BIND_PORT=Enter port [default %DEFAULT_PORT%]: 
if "%BIND_PORT%"=="" set BIND_PORT=%DEFAULT_PORT%

echo.
echo ============================================
echo Starting Logger
echo IP   : %BIND_IP%
echo Port : %BIND_PORT%
echo ============================================
echo.

if "%CHOICE%"=="1" (
    if exist "bin_logger.exe" (
        bin_logger.exe %BIND_IP% %BIND_PORT%
    ) else (
        echo ERROR: bin_logger.exe not found
    )
) else if "%CHOICE%"=="2" (
    if exist "text_logger.exe" (
        text_logger.exe %BIND_IP% %BIND_PORT%
    ) else (
        echo ERROR: text_logger.exe not found
    )
) else (
    echo Invalid choice.
)

echo.
echo Program exited.
pause