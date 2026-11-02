# Infineon CY8CKIT-149 PSoC 4100S Plus SWD Programming via ST-LINK V2 Clone

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![MCU: PSoC 4100S Plus](https://img.shields.io/badge/MCU-CY8C4147AZI--S475-red.svg)](https://www.infineon.com)
[![Toolchain: ModusToolbox](https://img.shields.io/badge/Toolchain-ModusToolbox_3.2%2B-green.svg)](https://www.infineon.com/modustoolbox)
[![Programmer: ST-LINK V2](https://img.shields.io/badge/Programmer-ST--LINK_V2_Clone-orange.svg)](https://openocd.org)

Production-grade Infineon ModusToolbox application featuring a **CapSense® 5-segment touch slider with LED bargraph tracking**, integrated **Long-Term Stability (LTS) diagnostics & hardware self-test suite**, and a turnkey **OpenOCD flashing pipeline using a low-cost ST-LINK V2 clone programmer**.

---

## Table of Contents
1. [Overview](#overview)
2. [Hardware Setup & Wiring](#hardware-setup--wiring)
3. [The ST-LINK V2 Clone Gotcha & Solution](#the-st-link-v2-clone-gotcha--solution)
4. [Firmware Architecture](#firmware-architecture)
5. [Long-Term Stability (LTS) & Self-Test Suite](#long-term-stability-lts--self-test-suite)
6. [OpenOCD Technical Breakthroughs](#openocd-technical-breakthroughs)
7. [Quick Start: Build & Flash](#quick-start-build--flash)
8. [ModusShell Makefile Commands](#modusshell-makefile-commands)
9. [Pre-compiled Binaries](#pre-compiled-binaries)
10. [Roadmap](#roadmap)

---

## Overview

The **Infineon CY8CKIT-149 PSoC 4100S Plus Prototyping Kit** is a versatile dual-silicon development platform featuring:
* **Target MCU (U2)**: Infineon PSoC® 4100S Plus (`CY8C4147AZI-S475`, ARM® Cortex®-M0+ running at 48 MHz, 128 KB Flash, 16 KB SRAM).
* **BLE Module (U3)**: EZ-BLE™ PRoC™ (`CYBLE-022001-00`).
* **HMI Sensors**: 5-segment CapSense linear slider with 5 dedicated green LEDs (`P1.0`, `P1.2`, `P1.4`, `P1.6`, `P2.0`) and blue User LED (`P3.4`).

Standard development typically relies on the on-board KitProg programmer via Micro-USB. This repository provides a complete, standalone build and SWD flashing workflow using a generic **ST-LINK V2 USB clone dongle** (VID:PID `0483:3748`), freeing developers from proprietary hardware bridges.

---

## Hardware Setup & Wiring

### Switch Configurations on CY8CKIT-149
* **SW4 (Debug Selection Switch)**: Set to **PSoC** position.  
  * Connects the SWD lines of the 10-pin debug header (`J4`) to the PSoC 4100S Plus MCU (`U2`), completely bypassing the BLE module (`U3`).
* **SW1 (Power Mode)**: Set to **Regulated** (or default factory position).

### SWD Pin Mapping (ST-LINK V2 Clone to J4 Header)

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

### Prerequisites
* [ModusToolbox 3.x](https://www.infineon.com/modustoolbox) with ModusShell
* [OpenOCD](https://openocd.org) (v0.12.0 or higher)
* [usbipd-win](https://github.com/dorssel/usbipd-win) (if running OpenOCD inside WSL2)

### Option A: Using ModusShell (Windows / Cygwin)
Open **ModusShell**, navigate to the project directory, and run:

```bash
# 1. Compile the firmware
make build

# 2. Flash to CY8CKIT-149 via ST-LINK V2
make flash-stlink
```

### Option B: Standalone Script (Windows Command Prompt / PowerShell)
```cmd
tools\scripts\flash_stlink.bat
```

### Option C: Standalone Script (Linux / WSL)
```bash
chmod +x tools/scripts/flash_stlink.sh
./tools/scripts/flash_stlink.sh
```

---

## ModusShell Makefile Commands

| Command | Action |
| :--- | :--- |
| `make build` | Compiles source files into ELF and HEX binaries |
| `make clean` | Cleans build artifacts |
| `make flash-stlink` | Programs the application into PSoC 4 flash via ST-LINK and resets target |
| `make probe-stlink` | Reads target voltage, verifies Cortex-M0+ core ID, and probes 128 KiB flash |
| `make reset-stlink` | Issues a software reset and starts firmware execution |
| `make halt-stlink` | Halts CPU and reads core registers (`xPSR`, `PC`, `MSP`) |

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
