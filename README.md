# Infineon CY8CKIT-149 PSoC 4100S Plus SWD Programming via Raspberry Pi Pico 2 & ST-LINK V2

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows_10%2F11-0078D6.svg)](WINDOWS_SETUP.md)
[![Programmer: Pico 2 (RP2350)](https://img.shields.io/badge/Programmer-Raspberry_Pi_Pico_2_(CMSIS--DAP)-purple.svg)](https://github.com/raspberrypi/debugprobe)
[![MCU: PSoC 4100S Plus](https://img.shields.io/badge/MCU-CY8C4147AZI--S475-red.svg)](https://www.infineon.com)
[![Toolchain: ModusToolbox](https://img.shields.io/badge/Toolchain-ModusToolbox_3.2%2B-green.svg)](https://www.infineon.com/modustoolbox)

Production-grade Infineon ModusToolbox application featuring a **CapSense® 5-segment touch slider with LED bargraph tracking**, integrated **Long-Term Stability (LTS) diagnostics & hardware self-test suite**, and turnkey **SWD flashing pipelines using either a Raspberry Pi Pico 2 (CMSIS-DAP v2) or an ST-LINK V2 clone**.

> [!TIP]
> **Windows Users**: For a complete, dedicated end-to-end walkthrough including USB drivers, switch configurations, and 1-click batch flashing, see the **[Windows Setup & Flashing Guide (WINDOWS_SETUP.md)](WINDOWS_SETUP.md)**.

---

## Table of Contents
1. [Overview](#overview)
2. [Hardware Setup & Wiring](#hardware-setup--wiring)
   - [Programmer Option 1: Raspberry Pi Pico 2 (RP2350 CMSIS-DAP)](#programmer-option-1-raspberry-pi-pico-2-rp2350-cmsis-dap-recommended)
   - [Programmer Option 2: ST-LINK V2 Clone](#programmer-option-2-st-link-v2-clone)
3. [The ST-LINK V2 Clone Gotcha & Solution](#the-st-link-v2-clone-gotcha--solution)
4. [Firmware Architecture](#firmware-architecture)
5. [Long-Term Stability (LTS) & Self-Test Suite](#long-term-stability-lts--self-test-suite)
6. [OpenOCD Technical Breakthroughs](#openocd-technical-breakthroughs)
7. [Quick Start: Build & Flash](#quick-start-build--flash)
8. [ModusShell Makefile Commands](#modusshell-makefile-commands)
9. [Pre-compiled Binaries](#pre-compiled-binaries)

---

## Overview

The **Infineon CY8CKIT-149 PSoC 4100S Plus Prototyping Kit** features:
* **Target MCU (U2)**: Infineon PSoC® 4100S Plus (`CY8C4147AZI-S475`, ARM® Cortex®-M0+ running at 48 MHz, 128 KB Flash, 16 KB SRAM).
* **BLE Module (U3)**: EZ-BLE™ PRoC™ (`CYBLE-022001-00`).
* **HMI Sensors**: 5-segment CapSense linear slider with 5 dedicated green LEDs (`P1.0`, `P1.2`, `P1.4`, `P1.6`, `P2.0`) and blue User LED (`P3.4`).

---

## Hardware Setup & Wiring

### Switch Configurations on CY8CKIT-149
* **SW4 (Debug Selection Switch)**: Set to **PSoC** position.  
  * Connects the SWD lines of the 10-pin debug header (`J4`) to the PSoC 4100S Plus MCU (`U2`), bypassing the BLE module.
* **SW1 (Power Mode)**: Set to **Regulated**.

---

### Programmer Option 1: Raspberry Pi Pico 2 (RP2350 CMSIS-DAP) [Recommended]

The **Raspberry Pi Pico 2 (RP2350)** functions as a high-speed **CMSIS-DAP v2 hardware debug probe** with a built-in USB-to-UART serial bridge (enumerating natively on Windows as `COMx` with zero custom driver installs required).

```
   Raspberry Pi Pico 2 (RP2350)              CY8CKIT-149 J4 Header
   +---------------------------+                  +---------+
   | (Pin 4)  GP2 [SWCLK]      |----------------> |  Pin 4  | SWDCLK
   | (Pin 5)  GP3 [SWDIO]      |<---------------> |  Pin 3  | SWDIO
   | (Pin 38) GND              |----------------> |  Pin 2  | GND
   | (Pin 36) 3V3(OUT)         |----------------> |  Pin 1  | VTARG (3.3V)
   |                           |                  |         |
   | (Pin 6)  GP4 [UART1_TX]   |---> (Optional Target UART_RX)
   | (Pin 7)  GP5 [UART1_RX]   |<--- (Optional Target UART_TX)
   +---------------------------+                  +---------+
```

#### Pin Mapping Table (Pico 2 to J4 Header)

| Pico 2 Pin Name | Pico 2 Physical Pin | Target Signal | CY8CKIT-149 J4 Pin | Description |
| :--- | :--- | :--- | :--- | :--- |
| **GP2** | **Pin 4** | `SWDCLK` | **Pin 4** | SWD Clock |
| **GP3** | **Pin 5** | `SWDIO` | **Pin 3** | SWD Data Bidirectional |
| **GND** | **Pin 38 or Pin 3** | `GND` | **Pin 2** | Common Ground |
| **3V3(OUT)** | **Pin 36** | `VTARG` | **Pin 1** | Target 3.3V Power Supply |
| *GP4 (UART TX)* | *Pin 6* | *UART_RX* | *Optional* | Debug Probe to Target TX |
| *GP5 (UART RX)* | *Pin 7* | *UART_TX* | *Optional* | Target to Serial Monitor |

> [!TIP]
> To configure the Pico 2, simply copy [`tools/debugprobe_on_pico2.uf2`](tools/debugprobe_on_pico2.uf2) to the Pico 2 USB drive (`RP2350`) in BOOTSEL mode. It will automatically flash and reboot as a CMSIS-DAP probe.

---

### Programmer Option 2: ST-LINK V2 Clone

| ST-LINK V2 Clone Pin | CY8CKIT-149 J4 Header | Signal Name | Description |
| :--- | :--- | :--- | :--- |
| **Pin 2 (SWCLK)** | **Pin 4** | `SWDCLK` | SWD Serial Clock |
| **Pin 4 (SWDIO)** | **Pin 3** | `SWDIO` | SWD Serial Data I/O |
| **Pin 6 (GND)** | **Pin 2** | `GND` | Common Ground Reference |
| **Pin 8 (3.3V)** | **Pin 1** | `VTARG / VDD` | 3.3V Power Supply from ST-LINK |
| **Pin 10 (RST)** | **DO NOT CONNECT** | `XRES` | **Must remain disconnected!** |

| ST-LINK V2 Clone Pin | CY8CKIT-149 J4 Header | Signal Name | Description |
| :--- | :--- | :--- | :--- |
| **Pin 2 (SWCLK)** | **Pin 4** | `SWDCLK` | SWD Serial Clock |
| **Pin 4 (SWDIO)** | **Pin 3** | `SWDIO` | SWD Serial Data I/O |
| **Pin 6 (GND)** | **Pin 2** | `GND` | Common Ground Reference |
| **Pin 8 (3.3V / VCC)** | **Pin 1** | `VTARG / VDD` | 3.3V Power Supply from ST-LINK |
| **Pin 10 (RST)** | **DO NOT CONNECT** | `XRES` | **Must remain disconnected!** |

> [!CAUTION]
> **Do NOT plug in the Micro-USB cable (`J8`)** while powering the board via the ST-LINK 3.3V pin to avoid conflicting voltage regulators.

---

## The ST-LINK V2 Clone Gotcha & Solution

### Root Cause
Generic aluminum ST-LINK V2 dongles are dual-purpose programmers (STM8 SWIM + STM32 SWD). On these clone boards, the `RST` pin is wired directly to an internal STM8 pull-down transistor circuit. When connected to the PSoC `XRES` pin, the ST-LINK clone clamps `XRES` to 0V (GND).

On Cypress / Infineon PSoC 4 silicon, **holding `XRES` at 0V disables the internal SWD debug multiplexer**. The programmer fails with:
```text
Error: init mode failed (unable to connect to the target)
```

### The Fix
1. **Completely disconnect the `RST` wire.**
2. Use **4-wire SWD**: `SWCLK`, `SWDIO`, `GND`, and `3.3V`.
3. OpenOCD resets the core seamlessly using Cortex-M0+ Application Interrupt and Reset Control Register (`AIRCR.SYSRESETREQ` via `mww 0xe000ed0c 0x05fa0004` / `reset run`).

---

## Firmware Architecture

The firmware is designed following Infineon ModusToolbox design principles, cleanly dividing hardware sensing, visual feedback, and system verification into modular tasks:

```text
├── main.c              # Application entry point, BSP initialization, pre-flight checks, and event loop
├── capsense_task.h/c   # CSD HW block init, ISR dispatch, non-blocking polling, centroid extraction
├── led_task.h/c        # GPIO drive modes, power-on visual sweep, bargraph tracking, error flashing
├── lts_test.h/c        # Long-Term Stability monitoring, baseline drift analysis, noise floor evaluation
├── bsps/               # Official Infineon CY8CKIT-149 Board Support Package & Device Configurator files
├── binaries/           # Ready-to-flash .hex and .elf images
└── tools/              # OpenOCD target configurations, patch files, and automated flash scripts
```

### Main Operational Flow
1. **BSP Initialization**: `cybsp_init()` initializes system clocks (IMO = 48 MHz) and power supply rails.
2. **Visual LED Sweep**: `led_startup_sweep()` performs a bidirectional sweep across the 5 slider LEDs and confirms User LED functionality.
3. **Pre-flight Self-Test**: `lts_test_run_self_test()` executes 5 hardware-in-the-loop diagnostic routines before entering normal operation.
4. **CapSense Sensing**: `capsense_init()` starts autonomous non-blocking scanning. Centroid positions (0..100) are mapped to the 5 LEDs in real time.
5. **LTS Monitoring**: `lts_test_process()` tracks sensor baselines and emits a rhythmic heartbeat pulse on the blue User LED when healthy.

---

## Long-Term Stability (LTS) & Self-Test Suite

The included `lts_test` module provides automotive-style verification to guarantee firmware reliability:

### Automated 5-Stage Pre-Flight Self-Test
* **Test 1 (RAM Integrity)**: Validates SRAM read/write integrity using alternating checkerboard test patterns (`0x55AA55AA` / `0xAA55AA55`).
* **Test 2 (Core Timing)**: Verifies `SystemCoreClock` registers match expected 48 MHz system operating frequency.
* **Test 3 (GPIO Port State)**: Asserts strong drive modes and verifies pin toggle capability without bus faults.
* **Test 4 (Sensor Baseline Sanity)**: Confirms CapSense CSD baselines are non-zero and within nominal range (100 to 65,000 counts) across all 5 physical sensors (`SNS0` to `SNS4`).
* **Test 5 (Centroid Math Bounds)**: Validates slider boundary math (0, 20, 50, 80, 100) and step-size thresholds.

### Runtime Stability Diagnostics
* **Baseline Drift Monitor**: Tracks baseline drift over time. Flags an alert if drift exceeds `LTS_DRIFT_THRESHOLD_COUNTS`.
* **Noise Floor Tracking**: Evaluates peak-to-peak raw count noise when the slider is untouched.
* **Heartbeat Indicator**: Emits a distinct dual-blink heartbeat pattern every 1.5 seconds on the blue User LED (`P3.4`) when untouched, confirming the MCU is active and healthy.

---

## OpenOCD Technical Breakthroughs

Using OpenOCD with the PSoC 4100S Plus and ST-LINK V2 clone requires two critical configurations:

### 1. Cortex-M0+ DAP TAPID
Standard PSoC 4 / PRoC BLE MCUs use Cortex-M0 with ID `0x0bb11477`. The PSoC 4100S Plus features an ARM **Cortex-M0+** core with TAPID **`0x0bc11477`**.
In OpenOCD:
```tcl
set CPUTAPID 0x0bc11477
```

### 2. PSoC 4100S Plus Family ID (`0xB5`)
Upstream OpenOCD 0.12.0 `psoc4.c` flash driver does not include Family ID `0xB5` (181 decimal) for PSoC 4100S Plus, failing with:
```text
Error: Cannot identify PSoC 4 family.
```
This repository provides:
* The source patch in `tools/patches/openocd_psoc4100sp_family.patch`.
* Custom target script in `tools/openocd/target/psoc4100sp.cfg`.

---

## Quick Start: Build & Flash

### Terminal Output Verification

#### 1. Live Flashing via Raspberry Pi Pico 2 (CMSIS-DAP)
![Raspberry Pi Pico 2 CMSIS-DAP Flashing Terminal Output](images/terminal_flash_pico2.png)

#### 2. ModusToolbox Build Pipeline
![ModusToolbox Build Output](images/terminal_build_complete.png)

---

### Prerequisites
* [ModusToolbox 3.x](https://www.infineon.com/modustoolbox) with ModusShell
* [OpenOCD](https://openocd.org) (bundled pre-patched binary included in `tools/openocd_win/`)

### Option A: Using Raspberry Pi Pico 2 [Fastest - No Drivers Required]
1. Wire Pico 2 (`GP2` -> SWCLK, `GP3` -> SWDIO, `GND` -> GND, `3V3(OUT)` -> Pin 1 VTARG).
2. Double-click or run:
   ```cmd
   tools\scripts\flash_pico2_dap.bat
   ```
   Or via ModusShell:
   ```bash
   make flash-pico2
   ```

### Option B: Using ST-LINK V2 Clone
1. Wire ST-LINK (`SWCLK`, `SWDIO`, `GND`, `3.3V` — **leave `RST` disconnected**).
2. Run:
   ```cmd
   tools\scripts\flash_stlink.bat
   ```
   Or via ModusShell:
   ```bash
   make flash-stlink
   ```

### Option C: Standalone Linux / WSL Script
```bash
chmod +x tools/scripts/flash_pico2_dap.sh
./tools/scripts/flash_pico2_dap.sh
```

---

## ModusShell Makefile Commands

| Command | Action |
| :--- | :--- |
| `make build` | Compiles source files into ELF and HEX binaries (`-j8` multi-core supported) |
| `make clean` | Cleans build artifacts |
| `make flash-pico2` | Programs firmware via **Raspberry Pi Pico 2** CMSIS-DAP v2 interface and resets MCU |
| `make probe-pico2` | Probes target voltage, Cortex-M0+ core ID, and flash memory via Pico 2 |
| `make reset-pico2` | Software reset of PSoC 4 via Pico 2 |
| `make flash-stlink` | Programs firmware via **ST-LINK V2 clone** and resets MCU |
| `make probe-stlink` | Probes target via ST-LINK V2 |
| `make reset-stlink` | Software reset of PSoC 4 via ST-LINK |

---

## Pre-compiled Binaries

Ready-to-flash binaries are included in the [`binaries/`](binaries/) directory:
* [`mtb-example-psoc4-hello-world.hex`](binaries/mtb-example-psoc4-hello-world.hex) (Intel HEX format, ~56 KB)
* [`mtb-example-psoc4-hello-world.elf`](binaries/mtb-example-psoc4-hello-world.elf) (ELF executable with debug symbols)

---

## Roadmap

- [x] PSoC 4100S Plus CapSense linear slider & LED bargraph implementation.
- [x] ST-LINK V2 clone SWD programming workflow & OpenOCD driver integration.
- [x] Long-Term Stability (LTS) self-test and baseline monitoring suite.
- [ ] **Next Phase**: Porting and interfacing with **Raspberry Pi Pico (RP2040)** as an external coprocessor/bridge.

---

## License

Licensed under the [Apache License, Version 2.0](LICENSE).  
Copyright (c) 2026 Infineon Technologies AG / Gowtham Thangaraj.
