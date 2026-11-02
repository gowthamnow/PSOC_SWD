@echo off
REM ==============================================================================
REM Universal Windows Flashing Script for CY8CKIT-149 with ST-LINK V2
REM Supports native Windows OpenOCD, bundled OpenOCD, or automatic WSL2 fallback
REM ==============================================================================

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set REPO_ROOT=%SCRIPT_DIR%..\..

for %%i in ("%REPO_ROOT%\binaries\mtb-example-psoc4-hello-world.hex") do set HEX_FILE=%%~fi
if exist "%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex" (
    for %%i in ("%REPO_ROOT%\build\APP_CY8CKIT-149\Debug\mtb-example-psoc4-hello-world.hex") do set HEX_FILE=%%~fi
)

set OPENOCD_HEX=!HEX_FILE:\=/!

for %%i in ("%REPO_ROOT%\tools\openocd_win\xpack-openocd-0.12.0-7\bin\openocd.exe") do set OPENOCD_EXE=%%~fi
for %%i in ("%REPO_ROOT%\tools\openocd_win\xpack-openocd-0.12.0-7\openocd\scripts") do set SCRIPTS_DIR=%%~fi
set OPENOCD_SCRIPTS=!SCRIPTS_DIR:\=/!

for %%i in ("%REPO_ROOT%\tools\openocd\interface\stlink.cfg") do set STLINK_CFG=%%~fi
set OPENOCD_STLINK_CFG=!STLINK_CFG:\=/!

if not exist "%HEX_FILE%" (
    echo [ERROR] Firmware hex file not found: %HEX_FILE%
    pause
    exit /b 1
)

echo ====================================================================
echo  Infineon CY8CKIT-149 PSoC 4100S Plus ST-LINK V2 Flashing Utility
echo ====================================================================
echo Firmware: %OPENOCD_HEX%
echo.

if exist "%OPENOCD_EXE%" (
    echo [*] Using bundled Windows OpenOCD with PSoC 4100S+ support...
    "%OPENOCD_EXE%" -s "%OPENOCD_SCRIPTS%" ^
      -f "%OPENOCD_STLINK_CFG%" ^
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
    echo [*] Detected native Windows OpenOCD. Flashing directly...
    openocd -f interface/stlink.cfg -c "set CPUTAPID 0x0bc11477" -f target/psoc4.cfg ^
      -c "init" ^
      -c "halt" ^
      -c "flash write_image erase %OPENOCD_HEX%" ^
      -c "reset run" ^
      -c "exit"
    goto check_result
)

where wsl >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] OpenOCD not found in Windows PATH. Attempting WSL2 execution...
    set WSL_HEX=/mnt/d/PSOC_SWD/binaries/mtb-example-psoc4-hello-world.hex
    wsl -d Ubuntu -- bash -c "openocd -f interface/stlink.cfg -c 'set CPUTAPID 0x0bc11477' -f target/psoc4.cfg -c 'init; halt; flash write_image erase %WSL_HEX%; reset run; exit'"
    goto check_result
)

echo [!] OpenOCD was not found on your Windows system.
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
