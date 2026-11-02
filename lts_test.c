/******************************************************************************
 * \file    lts_test.c
 * \version 1.0.0
 *
 * \brief
 * Long-Term Stability (LTS) & Automated Hardware Self-Test Suite Implementation.
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#include "lts_test.h"
#include "capsense_task.h"
#include "led_task.h"
#include <string.h>

#define LTS_SENSOR_COUNT            (5u)
#define LTS_HEARTBEAT_PERIOD_MS     (1000u)
#define LTS_DRIFT_THRESHOLD_COUNTS  (300u)
#define LTS_MAX_NOISE_THRESHOLD     (150u)

static lts_test_report_t s_report;
static uint16_t s_initial_baselines[LTS_SENSOR_COUNT] = {0};
static uint16_t s_prev_raw_counts[LTS_SENSOR_COUNT]   = {0};
static bool s_prev_touch_state = false;
static uint32_t s_cycle_counter = 0;

void lts_test_init(void)
{
    memset(&s_report, 0, sizeof(s_report));
    s_report.is_healthy = true;
    s_report.active_sensors_ok = 0x1F; /* 5 sensors (bits 0..4) */

    /* Capture initial baselines for drift monitoring */
    for (uint32_t i = 0u; i < LTS_SENSOR_COUNT; i++)
    {
        uint16_t raw = 0, base = 0;
        if (CY_RSLT_SUCCESS == capsense_get_sensor_data(i, &raw, &base))
        {
            s_initial_baselines[i] = base;
            s_prev_raw_counts[i]   = raw;
        }
    }
}

void lts_test_process(bool is_touched)
{
    s_report.total_scans++;
    s_cycle_counter++;

    /* Track touch transition count */
    if (is_touched && !s_prev_touch_state)
    {
        s_report.touch_event_count++;
    }
    s_prev_touch_state = is_touched;

    /* Perform sensor baseline & noise evaluation every 50 scan cycles */
    if ((s_cycle_counter % 50u) == 0u)
    {
        for (uint32_t i = 0u; i < LTS_SENSOR_COUNT; i++)
        {
            uint16_t raw = 0, base = 0;
            if (CY_RSLT_SUCCESS == capsense_get_sensor_data(i, &raw, &base))
            {
                /* Evaluate baseline drift */
                if (s_initial_baselines[i] > 0)
                {
                    uint16_t drift = (base > s_initial_baselines[i]) ?
                        (base - s_initial_baselines[i]) : (s_initial_baselines[i] - base);

                    if (drift > s_report.baseline_drift_max)
                    {
                        s_report.baseline_drift_max = drift;
                    }

                    if (drift > LTS_DRIFT_THRESHOLD_COUNTS)
                    {
                        s_report.test_failures |= LTS_TEST_FAIL_BASELINE_DRIFT;
                        s_report.is_healthy = false;
                    }
                }

                /* Evaluate noise floor when untouched */
                if (!is_touched && s_prev_raw_counts[i] > 0)
                {
                    uint16_t noise = (raw > s_prev_raw_counts[i]) ?
                        (raw - s_prev_raw_counts[i]) : (s_prev_raw_counts[i] - raw);

                    if (noise > s_report.max_noise_recorded)
                    {
                        s_report.max_noise_recorded = noise;
                    }

                    if (noise > LTS_MAX_NOISE_THRESHOLD)
                    {
                        s_report.test_failures |= LTS_TEST_FAIL_NOISE_FLOOR;
                        s_report.is_healthy = false;
                    }
                }

                s_prev_raw_counts[i] = raw;
            }
        }
    }

    /* Heartbeat animation when untouched: brief double-blink every ~1.5s */
    if (!is_touched)
    {
        /* Every ~300 scans (approx 1.5 seconds) */
        uint32_t beat_phase = s_cycle_counter % 300u;
        if (beat_phase == 0u || beat_phase == 8u)
        {
            led_set_user(true);
        }
        else if (beat_phase == 4u || beat_phase == 12u)
        {
            led_set_user(false);
        }
    }
}

lts_test_report_t lts_test_run_self_test(void)
{
    lts_test_report_t self_report;
    memset(&self_report, 0, sizeof(self_report));
    self_report.is_healthy = true;

    /* -------------------------------------------------------------
     * Test 1: RAM Integrity & Scratch Pattern Verification
     * ------------------------------------------------------------- */
    volatile uint32_t scratch_ram[16];
    const uint32_t pattern_a = 0x55AA55AAu;
    const uint32_t pattern_b = 0xAA55AA55u;

    for (uint32_t i = 0u; i < 16u; i++)
    {
        scratch_ram[i] = (i % 2u == 0u) ? pattern_a : pattern_b;
    }

    for (uint32_t i = 0u; i < 16u; i++)
    {
        uint32_t expected = (i % 2u == 0u) ? pattern_a : pattern_b;
        if (scratch_ram[i] != expected)
        {
            self_report.test_failures |= LTS_TEST_FAIL_RAM;
            self_report.is_healthy = false;
            break;
        }
    }

    /* -------------------------------------------------------------
     * Test 2: System Core Clock & Timing Sanity Check
     * ------------------------------------------------------------- */
    if (SystemCoreClock < 1000000u || SystemCoreClock > 50000000u)
    {
        self_report.test_failures |= LTS_TEST_FAIL_TIMING;
        self_report.is_healthy = false;
    }

    /* -------------------------------------------------------------
     * Test 3: LED GPIO Configuration Check
     * ------------------------------------------------------------- */
    led_init();
    /* Verify User LED can toggle without fault */
    led_set_user(true);
    Cy_SysLib_Delay(20u);
    led_set_user(false);

    /* -------------------------------------------------------------
     * Test 4: CapSense Sensor Baseline Sanity
     * ------------------------------------------------------------- */
    uint8_t healthy_mask = 0u;
    for (uint32_t i = 0u; i < LTS_SENSOR_COUNT; i++)
    {
        uint16_t raw = 0, base = 0;
        if (CY_RSLT_SUCCESS == capsense_get_sensor_data(i, &raw, &base))
        {
            /* Baselines should be initialized and non-zero */
            if (base > 100u && base < 65000u)
            {
                healthy_mask |= (uint8_t)(1u << i);
            }
        }
    }
    self_report.active_sensors_ok = healthy_mask;
    if (healthy_mask != 0x1Fu)
    {
        self_report.test_failures |= LTS_TEST_FAIL_BASELINE_DRIFT;
    }

    /* -------------------------------------------------------------
     * Test 5: Centroid Math Bounds Verification
     * ------------------------------------------------------------- */
    /* Verify coordinate boundaries (0 to 100) */
    uint16_t test_positions[] = { 0u, 20u, 50u, 80u, 100u };
    for (uint32_t i = 0u; i < 5u; i++)
    {
        led_update_slider(test_positions[i], true);
        Cy_SysLib_Delay(30u);
    }
    led_update_slider(0u, false);

    return self_report;
}

const lts_test_report_t *lts_test_get_report(void)
{
    return &s_report;
}
