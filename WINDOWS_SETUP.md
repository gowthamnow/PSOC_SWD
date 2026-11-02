# Windows Setup & Flashing Guide: Infineon CY8CKIT-149 with ST-LINK V2

This guide provides end-to-end instructions for building, flashing, and debugging the Infineon CY8CKIT-149 PSoC 4100S Plus kit on **Microsoft Windows 10 / 11** using a low-cost **ST-LINK V2 clone programmer**.

---

## Table of Contents
1. [Prerequisites & Software Checklist](#1-prerequisites--software-checklist)
2. [Hardware Preparation & Switch Settings](#2-hardware-preparation--switch-settings)
3. [Wiring: ST-LINK V2 Clone to J4 Debug Header](#3-wiring-st-link-v2-clone-to-j4-debug-header)
4. [ST-LINK USB Driver Setup on Windows](#4-st-link-usb-driver-setup-on-windows)
5. [Windows Flashing Methods](#5-windows-flashing-methods)
   - [Method A: Standalone Windows Batch Script (Recommended)](#method-a-standalone-windows-batch-script-recommended)
   - [Method B: Native Windows ModusShell CLI (`make flash-stlink`)](#method-b-native-windows-modusshell-cli-make-flash-stlink)
   - [Method C: WSL2 + usbipd-win Toolchain](#method-c-wsl2--usbipd-win-toolchain)
6. [Building Firmware from Source on Windows](#6-building-firmware-from-source-on-windows)
7. [Windows Troubleshooting Guide](#7-windows-troubleshooting-guide)

---

## 1. Prerequisites & Software Checklist

| Tool | Purpose | Download Link |
| :--- | :--- | :--- |
| **ModusToolbox™ (v3.2+)** | Official Infineon IDE, GCC ARM toolchain, and ModusShell | [Infineon ModusToolbox](https://www.infineon.com/modustoolbox) |
| **ST-LINK USB Drivers** | Windows device driver for ST-LINK V2 dongles | [STSW-LINK009](https://www.st.com/en/development-tools/stsw-link009.html) or [Zadig](https://zadig.akeo.ie/) |
| **OpenOCD for Windows** | On-chip debugger supporting SWD | [xPack OpenOCD](https://github.com/xpack-dev-tools/openocd-xpack/releases) |
| **usbipd-win (Optional)** | Required only if running OpenOCD inside WSL2 | [usbipd-win GitHub](https://github.com/dorssel/usbipd-win/releases) |

---

## 2. Hardware Preparation & Switch Settings

Before making any wire connections, verify the slide switches on the CY8CKIT-149 board:

```
+--------------------------------------------------------------+
|                    CY8CKIT-149 PROTOTYPING KIT               |
|                                                              |
|   [ SW4 ]                                                    |
|   +---------------+                                          |
|   | BLE | PSoC 4  |  <--- Set slide switch to **PSoC**       |
|   +---------------+                                          |
|                                                              |
|   [ SW1 ]                                                    |
|   +---------------+                                          |
|   | Reg | Bypass  |  <--- Set slide switch to **Regulated**  |
|   +---------------+                                          |
+--------------------------------------------------------------+
```

* **SW4 (Debug Multiplexer Switch)**: Move slider to **PSoC**. This routes the `SWDIO` and `SWDCLK` pins from the 10-pin header (`J4`) directly to the PSoC 4100S Plus MCU (`U2`).
* **SW1 (Power Regulator Switch)**: Ensure it is set to **Regulated** (default factory setting).

---

## 3. Wiring: ST-LINK V2 Clone to J4 Debug Header

The CY8CKIT-149 features a 10-pin 0.050" / 0.100" debug header labeled **J4** adjacent to the MCU.

### Pinout Comparison & Connections

```
    ST-LINK V2 Clone (Pinout)            CY8CKIT-149 J4 Header
       +-----------------+                    +---------+
 RST   | 10           9  | SWIM         Pin 1 | [o] [o] | Pin 2  (GND)
 GND   |  8           7  | GND          Pin 3 | [o] [o] | Pin 4  (SWDCLK)
 SWIM  |  6           5  | 3.3V         Pin 5 | [o] [o] | Pin 6
 SWDIO |  4           3  | 3.3V         Pin 7 | [o] [o] | Pin 8
 SWCLK |  2           1  | 5.0V         Pin 9 | [o] [o] | Pin 10
       +-----------------+                    +---------+
```

| ST-LINK Pin | ST-LINK Signal | Wire Color | CY8CKIT-149 J4 Header | J4 Signal |
| :--- | :--- | :--- | :--- | :--- |
| **Pin 2** | `SWCLK` | Yellow | **Pin 4** | `SWDCLK` |
| **Pin 4** | `SWDIO` | Green | **Pin 3** | `SWDIO` |
| **Pin 6 or 8** | `GND` | Black | **Pin 2** | `GND` |
| **Pin 3 or 5** | `3.3V` | Red | **Pin 1** | `VTARG / VDD` |
| **Pin 10** | `RST` | **NONE** | **DO NOT CONNECT** | `XRES` |

> [!CRITICAL]
> **DO NOT CONNECT THE `RST` WIRE!**  
> On generic aluminum ST-LINK V2 clones, the `RST` pin is internally wired to an STM8 SWIM pull-down circuit. Connecting it permanently clamps the PSoC `XRES` line to 0V (ground), which disables the PSoC 4 debug port. Leaving `RST` disconnected allows the MCU to boot and communicate properly via 4-wire SWD.

> [!WARNING]
> **DO NOT connect the board's Micro-USB cable (`J8`)** while powering from the ST-LINK's 3.3V pin. Powering from two simultaneous USB sources can damage the onboard LDO or your PC USB port.

---

## 4. ST-LINK USB Driver Setup on Windows

### Check Current Driver in Windows Device Manager
1. Plug the ST-LINK V2 clone into your Windows USB port.
2. Press `Win + X` and select **Device Manager**.
3. Look under:
   * **Universal Serial Bus devices** -> `ST-Link Debug` or `STM32 STLink`
   * Or **USB Serial Bus controllers** -> `STMicroelectronics STLink Dongle`

### Driver Options
* **Option A (Official ST Driver)**: Download and run the [STSW-LINK009](https://www.st.com/en/development-tools/stsw-link009.html) installer (`dpinst_amd64.exe`).
* **Option B (Zadig WinUSB Driver for OpenOCD)**:
  1. Download [Zadig](https://zadig.akeo.ie/).
  2. Select **Options** -> **List All Devices**.
  3. Select `STM32 STLink` from the drop-down.
  4. Select **WinUSB (v6.1.7600.16385)** as the target driver and click **Replace Driver** (or **Install Driver**).

---

## 5. Windows Flashing Methods

### Method A: Standalone Windows Batch Script (Recommended)

This repository includes a smart Windows batch file that automatically detects your OpenOCD environment:

1. Open File Explorer and navigate to `tools\scripts\`.
2. Double-click **`flash_stlink.bat`** (or execute from Command Prompt / PowerShell):
   ```cmd
   cd D:\PSOC_SWD\tools\scripts
   flash_stlink.bat
   ```
3. The script will:
   * Validate the pre-compiled binary [`binaries/mtb-example-psoc4-hello-world.hex`](binaries/mtb-example-psoc4-hello-world.hex).
   * Connect to the PSoC 4100S Plus MCU via SWD.
   * Erase flash and write the application.
   * Reset the MCU into normal run mode.

---

### Method B: Native Windows ModusShell CLI (`make flash-stlink`)

If you have ModusToolbox installed:

1. Open **ModusShell** from the Windows Start menu.
2. Navigate to the project directory:
   ```bash
   cd /cygdrive/d/PSOC_SWD
   ```
3. Run the custom target:
   ```bash
   make flash-stlink
   ```
4. Other available utility commands:
   ```bash
   make probe-stlink    # Verifies target voltage (3.18V), Cortex-M0+ core, and 128 KiB flash
   make reset-stlink    # Performs software reset via Cortex-M0+ AIRCR
   make halt-stlink     # Halts CPU and reads registers (xPSR, PC, MSP)
   ```

---

### Method C: WSL2 + usbipd-win Toolchain

If using Windows Subsystem for Linux (WSL2):

1. **Install usbipd-win**:
   ```powershell
   winget install dorssel.usbipd-win
   ```
2. **Share and Attach ST-LINK to WSL** (Run in PowerShell as Administrator):
   ```powershell
   # Find the BUSID of the STM32 STLink (e.g. 3-13)
   usbipd list

   # Bind and attach to WSL
   usbipd bind --busid 3-13 --force
   usbipd attach --wsl --busid 3-13
   ```
3. **Execute Flashing in WSL Terminal**:
   ```bash
   chmod +x tools/scripts/flash_stlink.sh
   ./tools/scripts/flash_stlink.sh
   ```

---

## 6. Building Firmware from Source on Windows

To modify code (e.g., adding custom CapSense gestures or modifying LED behaviors):

1. Open **ModusShell** on Windows.
2. Navigate to the repository root:
   ```bash
   cd /cygdrive/d/PSOC_SWD
   ```
3. Build the project using all available CPU threads:
   ```bash
   make build -j8
   ```
4. Output files are placed in:
   * `build/APP_CY8CKIT-149/Debug/mtb-example-psoc4-hello-world.hex`
   * `build/APP_CY8CKIT-149/Debug/mtb-example-psoc4-hello-world.elf`
5. Flash immediately:
   ```bash
   make flash-stlink
   ```

---

## 7. Windows Troubleshooting Guide

| Issue / Error Message | Root Cause | Windows Fix |
| :--- | :--- | :--- |
| `Error: init mode failed (unable to connect to the target)` | `RST` wire is connected to ST-LINK clone, clamping `XRES` to 0V. | **Disconnect the `RST` wire completely.** Use only `SWCLK`, `SWDIO`, `GND`, and `3.3V`. |
| `Warn : UNEXPECTED idcode: 0x0bc11477` | PSoC 4100S Plus uses Cortex-M0+ TAPID `0x0bc11477` instead of classic PSoC 4 M0 (`0x0bb11477`). | Ensure `set CPUTAPID 0x0bc11477` is passed before loading `target/psoc4.cfg`. (Already configured in project scripts). |
| `Error: Cannot identify PSoC 4 family.` | Upstream OpenOCD 0.12.0 lacks Family ID `0xB5` (CY8C4147AZI-S475) in its ROM lookup table. | Use the pre-patched OpenOCD or apply [`tools/patches/openocd_psoc4100sp_family.patch`](tools/patches/openocd_psoc4100sp_family.patch). |
| `ST-LINK not detected / USB Error` | Windows USB driver missing or corrupted. | Run [Zadig](https://zadig.akeo.ie/) and reinstall the `WinUSB` driver for `STM32 STLink`. |
| `usbipd: error: Access denied` | Non-admin PowerShell session attempting USB driver bind. | Open PowerShell with **"Run as Administrator"** and repeat `usbipd bind`. |
| Slider LEDs do not light on touch | SW4 switch left in BLE position. | Slide **SW4** switch to the **PSoC** position. |
| Board does not power on | ST-LINK 3.3V pin disconnected or jumper missing on kit. | Verify 3.3V jumper on J4 Pin 1. Check with multimeter that VTARG is ~3.2V. |
