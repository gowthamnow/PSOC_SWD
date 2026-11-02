/******************************************************************************
 * \file    main.c
 * \version 2.2.0
 *
 * \brief
 * Multi-Mode Interactive CapSense Touch Slider Application for CY8CKIT-149.
 *
 * \details
 * Features:
 *  - 3 User-Selectable Display Modes:
 *      * Mode 0: Proportional Bargraph Fill
 *      * Mode 1: Precision Single-Dot Centroid Tracker
 *      * Mode 2: Knight Rider Wave Scanner (Touch-Modulated Speed)
 *  - Hardware User Button (SW2, P3.7) Mode Switcher with Debounce
 *  - Integrated Long-Term Stability (LTS) Diagnostics Suite
 *  - High-Speed SWD Flashing & Debugging via Raspberry Pi Pico 2 (CMSIS-DAP)
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

/* User Button SW2 configuration on CY8CKIT-149: Pin P3.7 (Active-Low) */
#define USER_BTN_PORT       GPIO_PRT3
#define USER_BTN_PIN        7U

static uint32_t s_btn_debounce_timer = 0;
static bool s_btn_last_state = true;

/*******************************************************************************
 * Function Name: handle_user_button
 *******************************************************************************
 * \brief
 * Checks onboard mechanical User Switch SW2 (P3.7) and cycles display mode.
 *******************************************************************************/
static void handle_user_button(void)
{
    /* Active-Low: 0 = pressed, 1 = released */
    uint32_t pin_val = Cy_GPIO_Read(USER_BTN_PORT, USER_BTN_PIN);
    bool is_pressed = (pin_val == 0u);

    if (is_pressed && s_btn_last_state)
    {
        /* Button was just pressed - debounce check */
        if (s_btn_debounce_timer == 0)
        {
            /* Cycle visual mode */
            led_cycle_mode();
            s_btn_debounce_timer = 15000u; /* Debounce holdoff */
        }
    }

    if (s_btn_debounce_timer > 0)
    {
        s_btn_debounce_timer--;
    }

    s_btn_last_state = !is_pressed;
}

/*******************************************************************************
 * Function Name: handle_touch_event
 *******************************************************************************
 * \brief
 * Callback invoked after each CapSense scan cycle completes.
 *******************************************************************************/
static void handle_touch_event(uint16_t position, bool is_touched)
{
    /* Update multi-mode LED visuals */
    led_update_slider(position, is_touched);

    /* Feed stability monitoring pipeline */
    lts_test_process(is_touched);
}

/*******************************************************************************
 * Function Name: main
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

    __enable_irq();

    /* Initialize LED peripheral driver and drive modes */
    led_init();

    /* Configure User Button SW2 (P3.7) with internal pull-up */
    Cy_GPIO_SetDrivemode(USER_BTN_PORT, USER_BTN_PIN, CY_GPIO_DM_PULLUP);

    /* Run power-on visual sweep to confirm all LEDs are operational */
    led_startup_sweep();

    /* Initialize CapSense CSD peripheral and begin sensing */
    result = capsense_init();
    if (result != CY_RSLT_SUCCESS)
    {
        led_flash_all(10u, 50u);
        CY_ASSERT(0);
    }

    /* Register callback */
    capsense_register_callback(handle_touch_event);

    /* Initialize Long-Term Stability subsystem */
    lts_test_init();

    /* Pre-flight hardware self-test */
    lts_test_report_t self_test = lts_test_run_self_test();
    if (!self_test.is_healthy)
    {
        led_flash_all(3u, 150u);
    }

    /* Main real-time event loop */
    for (;;)
    {
        /* Service CapSense sensing state machine */
        capsense_process();

        /* Service mechanical User Button */
        handle_user_button();
    }
}
