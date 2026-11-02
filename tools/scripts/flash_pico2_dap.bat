@echo off
REM ==============================================================================
REM Universal Windows Flashing Script for CY8CKIT-149 with Raspberry Pi Pico 2
REM Uses CMSIS-DAP v2 interface (native Windows WinUSB, zero driver install required)
REM ==============================================================================

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set REPO_ROOT=%SCRIPT_DIR%..\..

REM Resolve canonical full path to HEX file
for %%i in ("%REPO_ROOT%\binaries\mtb-example-psoc4-hello-world.hex") do set HEX_FILE=%%~fi
if exist "%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex" (
    for %%i in ("%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex") do set HEX_FILE=%%~fi
)

REM Convert backslashes to forward slashes for OpenOCD TCL parser
set OPENOCD_HEX=!HEX_FILE:\=/!

for %%i in ("%REPO_ROOT%\tools\openocd_win\xpack-openocd-0.12.0-7\bin\openocd.exe") do set OPENOCD_EXE=%%~fi
for %%i in ("%REPO_ROOT%\tools\openocd_win\xpack-openocd-0.12.0-7\openocd\scripts") do set SCRIPTS_DIR=%%~fi
set OPENOCD_SCRIPTS=!SCRIPTS_DIR:\=/!

for %%i in ("%REPO_ROOT%\tools\openocd\interface\cmsis-dap.cfg") do set DAP_CFG=%%~fi
set OPENOCD_DAP_CFG=!DAP_CFG:\=/!

if not exist "%HEX_FILE%" (
    echo [ERROR] Firmware hex file not found: %HEX_FILE%
    pause
    exit /b 1
)

echo ====================================================================
echo  Infineon CY8CKIT-149 Flashing via Raspberry Pi Pico 2 [CMSIS-DAP]
echo ====================================================================
echo Firmware: %OPENOCD_HEX%
echo.

if exist "%OPENOCD_EXE%" (
    echo [*] Using bundled Windows OpenOCD with PSoC 4100S+ support...
    "%OPENOCD_EXE%" -s "%OPENOCD_SCRIPTS%" ^
      -f "%OPENOCD_DAP_CFG%" ^
      -c "transport select swd" ^
      -c "adapter speed 2000" ^
      -c "set CPUTAPID 0x0bc11477" ^
      -f target/psoc4.cfg ^
      -c "init" ^
      -c "halt" ^
      -c "flash write_image erase %OPENOCD_HEX%" ^
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
      -c "flash write_image erase %OPENOCD_HEX%" ^
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
    echo  Please verify SWCLK [GP2], SWDIO [GP3], GND, and 3.3V wiring.
    echo ====================================================================
)
pause
exit /b %ERRORLEVEL%
