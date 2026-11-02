@echo off
REM ==============================================================================
REM Flashing script for Infineon CY8CKIT-149 with ST-LINK V2 clone
REM ==============================================================================

set HEX_FILE=%~dp0..\..\binaries\mtb-example-psoc4-hello-world.hex

if not exist "%HEX_FILE%" (
    echo [ERROR] Firmware hex file not found: %HEX_FILE%
    exit /b 1
)

echo ====================================================================
echo  Flashing CY8CKIT-149 PSoC 4100S Plus via ST-LINK V2...
echo ====================================================================
echo Firmware: %HEX_FILE%
echo.

openocd -f interface/stlink.cfg -c "set CPUTAPID 0x0bc11477" -f target/psoc4.cfg ^
  -c "init" ^
  -c "halt" ^
  -c "flash write_image erase %HEX_FILE%" ^
  -c "reset run" ^
  -c "exit"

if %ERRORLEVEL% equ 0 (
    echo.
    echo [+] Programming and Reset Complete!
) else (
    echo.
    echo [-] Flashing failed with error code %ERRORLEVEL%.
)
