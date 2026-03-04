@echo off
setlocal EnableExtensions EnableDelayedExpansion

set IMU_IP=192.168.10.5
set PORT=25001

cd /d "%~dp0"

REM =============================
REM Find executables
REM =============================

set BIN_EXE=
set TXT_EXE=

if exist "bin_logger.exe" set BIN_EXE=bin_logger.exe
if exist "text_logger.exe" set TXT_EXE=text_logger.exe

if exist "build\bin_logger.exe" if "%BIN_EXE%"=="" set BIN_EXE=build\bin_logger.exe
if exist "build\text_logger.exe" if "%TXT_EXE%"=="" set TXT_EXE=build\text_logger.exe

if "%BIN_EXE%"=="" if "%TXT_EXE%"=="" (
 echo ERROR: No logger executables found.
 echo Checked:
 echo   .
 echo   .\build
 pause
 exit /b 1
)

REM =============================
REM Detect correct IP
REM =============================

set BIND_IP=0.0.0.0

for /f "tokens=*" %%A in ('ipconfig ^| find "%IMU_IP%"') do (
 set BIND_IP=%IMU_IP%
)

REM =============================
REM Ask user which logger
REM =============================

echo.
echo IMU LOGGER
echo.

if not "%BIN_EXE%"=="" echo 1 - Binary Logger (recommended)
if not "%TXT_EXE%"=="" echo 2 - Text Logger
echo.

set /p CHOICE=Select logger [default 1]: 
if "%CHOICE%"=="" set CHOICE=1

if "%CHOICE%"=="1" (
 set EXE=%BIN_EXE%
) else (
 set EXE=%TXT_EXE%
)

echo.
echo Starting %EXE%
echo IP   : %BIND_IP%
echo Port : %PORT%
echo.

"%EXE%" %BIND_IP% %PORT%

pause