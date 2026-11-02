@echo off
REM ==============================================================================
REM Universal Windows Flashing Script for CY8CKIT-149 with ST-LINK V2
REM Supports native Windows OpenOCD, xPack OpenOCD, or automatic WSL2 fallback
REM ==============================================================================

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set REPO_ROOT=%SCRIPT_DIR%..\..
set HEX_FILE=%REPO_ROOT%\binaries\mtb-example-psoc4-hello-world.hex

REM If built locally, prefer fresh build binary
if exist "%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex" (
    set HEX_FILE=%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex
)

if not exist "%HEX_FILE%" (
    echo [ERROR] Firmware hex file not found: %HEX_FILE%
    pause
    exit /b 1
)

echo ====================================================================
echo  Infineon CY8CKIT-149 PSoC 4100S Plus ST-LINK V2 Flashing Utility
echo ====================================================================
echo Firmware: %HEX_FILE%
echo.

REM 1. Check for native Windows openocd.exe in PATH
where openocd >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] Detected native Windows OpenOCD. Flashing directly...
    openocd -f interface/stlink.cfg -c "set CPUTAPID 0x0bc11477" -f target/psoc4.cfg ^
      -c "init" ^
      -c "halt" ^
      -c "flash write_image erase \"%HEX_FILE%\"" ^
      -c "reset run" ^
      -c "exit"
    goto check_result
)

REM 2. Check for xPack OpenOCD in common default directories
set XPACK_PATH=C:\xpack-openocd\bin\openocd.exe
if exist "%XPACK_PATH%" (
    echo [*] Detected xPack OpenOCD at %XPACK_PATH%. Flashing...
    "%XPACK_PATH%" -f interface/stlink.cfg -c "set CPUTAPID 0x0bc11477" -f target/psoc4.cfg ^
      -c "init" ^
      -c "halt" ^
      -c "flash write_image erase \"%HEX_FILE%\"" ^
      -c "reset run" ^
      -c "exit"
    goto check_result
)

REM 3. Fallback: Check if WSL2 is installed and has OpenOCD
where wsl >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] OpenOCD not found in Windows PATH. Attempting WSL2 execution...
    set WSL_HEX=/mnt/d/PSOC_SWD/binaries/mtb-example-psoc4-hello-world.hex
    wsl -d Ubuntu -- bash -c "openocd -f interface/stlink.cfg -c 'set CPUTAPID 0x0bc11477' -f target/psoc4.cfg -c 'init; halt; flash write_image erase %WSL_HEX%; reset run; exit'"
    goto check_result
)

echo [!] OpenOCD was not found on your Windows system.
echo.
echo Please do one of the following:
echo   1. Install xPack OpenOCD from: https://github.com/xpack-dev-tools/openocd-xpack/releases
echo      and add its 'bin' directory to your Windows PATH.
echo   2. Or install Zadig (https://zadig.akeo.ie) and use WSL2 with usbipd-win.
echo.
echo See WINDOWS_SETUP.md for complete step-by-step instructions.
pause
exit /b 1

:check_result
if %ERRORLEVEL% equ 0 (
    echo.
    echo ====================================================================
    echo  [+] Programming and Target Reset Completed Successfully!
    echo ====================================================================
) else (
    echo.
    echo ====================================================================
    echo  [-] Flashing failed with error code %ERRORLEVEL%.
    echo  See WINDOWS_SETUP.md troubleshooting section for common fixes.
    echo ====================================================================
)
pause
exit /b %ERRORLEVEL%
