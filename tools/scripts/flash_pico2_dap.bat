@echo off
REM ==============================================================================
REM Universal Windows Flashing Script for CY8CKIT-149 with Raspberry Pi Pico 2
REM Uses CMSIS-DAP v2 interface (native Windows WinUSB, zero driver install required)
REM ==============================================================================

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set REPO_ROOT=%SCRIPT_DIR%..\..
set HEX_FILE=%REPO_ROOT%\binaries\mtb-example-psoc4-hello-world.hex
set OPENOCD_EXE=%REPO_ROOT%\tools\openocd_win\xpack-openocd-0.12.0-7\bin\openocd.exe
set SCRIPTS_DIR=%REPO_ROOT%\tools\openocd_win\xpack-openocd-0.12.0-7\openocd\scripts

REM If local build exists, use fresh build
if exist "%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex" (
    set HEX_FILE=%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex
)

if not exist "%HEX_FILE%" (
    echo [ERROR] Firmware hex file not found: %HEX_FILE%
    pause
    exit /b 1
)

echo ====================================================================
echo  Infineon CY8CKIT-149 Flashing via Raspberry Pi Pico 2 (CMSIS-DAP)
echo ====================================================================
echo Firmware: %HEX_FILE%
echo.

if exist "%OPENOCD_EXE%" (
    echo [*] Using bundled Windows OpenOCD with PSoC 4100S+ support...
    "%OPENOCD_EXE%" -s "%SCRIPTS_DIR%" ^
      -f interface/cmsis-dap.cfg ^
      -c "transport select swd" ^
      -c "adapter speed 2000" ^
      -c "set CPUTAPID 0x0bc11477" ^
      -f target/psoc4.cfg ^
      -c "init" ^
      -c "halt" ^
      -c "flash write_image erase \"%HEX_FILE%\"" ^
      -c "reset run" ^
      -c "exit"
    goto check_result
)

where openocd >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] Using system OpenOCD...
    openocd -f interface/cmsis-dap.cfg ^
      -c "transport select swd" ^
      -c "adapter speed 2000" ^
      -c "set CPUTAPID 0x0bc11477" ^
      -f target/psoc4.cfg ^
      -c "init" ^
      -c "halt" ^
      -c "flash write_image erase \"%HEX_FILE%\"" ^
      -c "reset run" ^
      -c "exit"
    goto check_result
)

echo [ERROR] OpenOCD executable not found.
pause
exit /b 1

:check_result
if %ERRORLEVEL% equ 0 (
    echo.
    echo ====================================================================
    echo  [+] Pico 2 Programming and Reset Completed Successfully!
    echo ====================================================================
) else (
    echo.
    echo ====================================================================
    echo  [-] Flashing failed with error code %ERRORLEVEL%.
    echo  Please verify SWCLK (GP2), SWDIO (GP3), GND, and 3.3V wiring.
    echo ====================================================================
)
pause
exit /b %ERRORLEVEL%
