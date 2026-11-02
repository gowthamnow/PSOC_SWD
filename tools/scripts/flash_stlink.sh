#!/usr/bin/env bash
# ==============================================================================
# Linux / WSL Flashing script for Infineon CY8CKIT-149 with ST-LINK V2 clone
# ==============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HEX_FILE="${SCRIPT_DIR}/../../binaries/mtb-example-psoc4-hello-world.hex"

if [ ! -f "${HEX_FILE}" ]; then
    echo "[ERROR] Hex file not found: ${HEX_FILE}"
    exit 1
fi

echo "===================================================================="
echo " Flashing CY8CKIT-149 PSoC 4100S Plus via ST-LINK V2..."
echo "===================================================================="
echo "Firmware: ${HEX_FILE}"
echo ""

# Use custom patched openocd if available in PATH or fallback
OPENOCD_CMD="openocd"
if command -v /home/gowtham/openocd_psoc4 &>/dev/null; then
    OPENOCD_CMD="/home/gowtham/openocd_psoc4"
fi

${OPENOCD_CMD} \
  -f interface/stlink.cfg \
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
    echo "[+] Programming and Reset Complete!"
else
    echo ""
    echo "[-] Flashing failed with exit code ${STATUS}."
fi
exit ${STATUS}
