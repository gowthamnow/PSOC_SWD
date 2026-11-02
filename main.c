/******************************************************************************
 * \file    main.c
 * \version 2.0.0
 *
 * \brief
 * Infineon CY8CKIT-149 PSoC 4100S Plus CapSense Touch Slider Application.
 *
 * \details
 * Implements a production-grade CapSense linear slider human-machine interface
 * (HMI) with LED bargraph tracking, power-on diagnostic sweep, and continuous
 * Long-Term Stability (LTS) monitoring.
 *
 * Hardware Mapping:
 *  - Target MCU:      Infineon CY8C4147AZI-S475 (Cortex-M0+ @ 48 MHz)
 *  - Board:           CY8CKIT-149 PSoC 4100S Plus Prototyping Kit
 *  - CapSense Slider: 5-segment CSD linear slider (Pins P2.1, P2.2, P2.3, P2.4, P2.5)
 *  - Slider LEDs:     5 green LEDs (P1.0, P1.2, P1.4, P1.6, P2.0) Active-Low
 *  - User LED 1:      Blue LED (P3.4) Active-High
 *  - Programmer:      ST-LINK V2 clone (SWD interface)
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "led_task.h"
#include "capsense_task.h"
#include "lts_test.h"

/*******************************************************************************
 * Function Name: handle_touch_event
 *******************************************************************************
 * \brief
 * Callback invoked whenever CapSense processes a scan cycle.
 *
 * \param[in] position Centroid coordinate (0 to 100).
 * \param[in] is_touched True if finger is detected on slider.
 *******************************************************************************/
static void handle_touch_event(uint16_t position, bool is_touched)
{
    /* Update LED bargraph to match slider coordinate */
    led_update_slider(position, is_touched);

    /* Feed stability monitoring pipeline */
    lts_test_process(is_touched);
}

/*******************************************************************************
 * Function Name: main
 *******************************************************************************
 * \brief
 * Application entry point.
 *******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize device and platform clocks via official Infineon BSP */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize LED peripheral driver and drive modes */
    led_init();

    /* Execute power-on visual sweep to confirm LED integrity */
    led_startup_sweep();

    /* Initialize CapSense CSD peripheral and start sensing */
    result = capsense_init();
    if (result != CY_RSLT_SUCCESS)
    {
        /* Alert fatal initialization error by rapid LED flashing */
        led_flash_all(10u, 50u);
        CY_ASSERT(0);
    }

    /* Register callback to decouple sensing from visualization */
    capsense_register_callback(handle_touch_event);

    /* Initialize Long-Term Stability monitoring subsystem */
    lts_test_init();

    /* Run pre-flight hardware self-test suite */
    lts_test_report_t self_test = lts_test_run_self_test();
    if (!self_test.is_healthy)
    {
        /* Signal test discrepancy via warning flash */
        led_flash_all(3u, 150u);
    }

    /* Main real-time event loop */
    for (;;)
    {
        /* Service CapSense sensing state machine */
        capsense_process();
    }
}
