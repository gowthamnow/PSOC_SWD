#!/usr/bin/env bash
# ==============================================================================
# Linux / WSL Flashing Script for CY8CKIT-149 with Raspberry Pi Pico 2 (CMSIS-DAP)
# ==============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HEX_FILE="${SCRIPT_DIR}/../../binaries/mtb-example-psoc4-hello-world.hex"

if [ -f "${SCRIPT_DIR}/../../build/APP_CY8CKIT-149/Debug/mtb-example-psoc4-hello-world.hex" ]; then
    HEX_FILE="${SCRIPT_DIR}/../../build/APP_CY8CKIT-149/Debug/mtb-example-psoc4-hello-world.hex"
fi

if [ ! -f "${HEX_FILE}" ]; then
    echo "[ERROR] Hex file not found: ${HEX_FILE}"
    exit 1
fi

echo "===================================================================="
echo " Flashing CY8CKIT-149 via Raspberry Pi Pico 2 (CMSIS-DAP)..."
echo "===================================================================="
echo "Firmware: ${HEX_FILE}"
echo ""

OPENOCD_CMD="openocd"
if command -v /home/gowtham/openocd_psoc4 &>/dev/null; then
    OPENOCD_CMD="/home/gowtham/openocd_psoc4"
fi

${OPENOCD_CMD} \
  -f interface/cmsis-dap.cfg \
  -c "transport select swd" \
  -c "adapter speed 2000" \
  -c "set CPUTAPID 0x0bc11477" \
  -f target/psoc4.cfg \
  -c "init" \
  -c "halt" \
  -c "flash write_image erase ${HEX_FILE}" \
  -c "reset run" \
  -c "exit"

STATUS=$?
if [ ${STATUS} -eq 0 ]; then
    echo ""
    echo "[+] Pico 2 Programming & Target Reset Complete!"
else
    echo ""
    echo "[-] Flashing failed with exit code ${STATUS}."
fi
exit ${STATUS}
