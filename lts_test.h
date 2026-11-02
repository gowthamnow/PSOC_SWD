/******************************************************************************
 * \file    lts_test.h
 * \version 1.0.0
 *
 * \brief
 * Long-Term Stability (LTS) & Automated Hardware Self-Test Suite for CY8CKIT-149.
 *
 * \details
 * Implements continuous stability monitoring, baseline drift tracking,
 * noise envelope validation, and automated on-target self-tests for the
 * Infineon PSoC 4100S Plus MCU and CapSense peripheral.
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#ifndef LTS_TEST_H
#define LTS_TEST_H

#include <stdint.h>
#include <stdbool.h>
#include "cy_pdl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief LTS self-test result status codes.
 */
typedef enum
{
    LTS_TEST_PASS               = 0x00,
    LTS_TEST_FAIL_RAM           = 0x01,
    LTS_TEST_FAIL_TIMING        = 0x02,
    LTS_TEST_FAIL_LED_GPIO      = 0x04,
    LTS_TEST_FAIL_CAPSENSE_INIT = 0x08,
    LTS_TEST_FAIL_BASELINE_DRIFT= 0x10,
    LTS_TEST_FAIL_NOISE_FLOOR   = 0x20
} lts_test_status_t;

/**
 * \brief Comprehensive LTS diagnostic report structure.
 */
typedef struct
{
    uint32_t total_scans;           /**< Total number of CapSense scans executed */
    uint32_t uptime_seconds;        /**< System uptime in seconds */
    uint32_t touch_event_count;     /**< Total number of distinct touch transitions */
    uint16_t max_noise_recorded;    /**< Maximum raw count peak noise detected */
    uint16_t baseline_drift_max;    /**< Maximum baseline variation observed */
    uint8_t  active_sensors_ok;     /**< Bitmask of sensors passing baseline sanity */
    uint32_t test_failures;         /**< Bitwise OR of lts_test_status_t error flags */
    bool     is_healthy;            /**< True if all metrics are within nominal limits */
} lts_test_report_t;

/**
 * \brief Initialize the Long-Term Stability monitoring subsystem.
 */
void lts_test_init(void);

/**
 * \brief Execute periodic LTS health check and heartbeat update.
 *
 * \details Non-blocking. Should be called inside the main application loop.
 *
 * \param[in] is_touched Current touch state of the slider.
 */
void lts_test_process(bool is_touched);

/**
 * \brief Run a comprehensive on-target hardware and peripheral self-test suite.
 *
 * \details Executes 5 automated diagnostic tests:
 *          1. RAM integrity test (pattern write & readback).
 *          2. SysTick & system core clock timing validation.
 *          3. LED GPIO drive mode and pin state validation.
 *          4. CapSense sensor baseline sanity check.
 *          5. Centroid math and boundary limit verification.
 *
 * \return lts_test_report_t Diagnostic summary of results.
 */
lts_test_report_t lts_test_run_self_test(void);

/**
 * \brief Get pointer to the live LTS runtime diagnostic report.
 *
 * \return Const pointer to the current lts_test_report_t.
 */
const lts_test_report_t *lts_test_get_report(void);

#ifdef __cplusplus
}
#endif

#endif /* LTS_TEST_H */
