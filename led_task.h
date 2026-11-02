/******************************************************************************
 * \file    led_task.h
 * \version 1.0.0
 *
 * \brief
 * LED control task and visual feedback module for CY8CKIT-149.
 *
 * \details
 * Manages the 5-segment linear capacitive slider LEDs (P1.0, P1.2, P1.4, P1.6,
 * P2.0) and the blue User LED (P3.4).
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#ifndef LED_TASK_H
#define LED_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "cy_pdl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Total number of active slider segments mapped to LEDs */
#define LED_SLIDER_SEGMENT_COUNT    (5u)

/**
 * \brief Initialize all LED GPIO pins as strong drive outputs and turn them off.
 */
void led_init(void);

/**
 * \brief Run power-on visual verification sweep across all slider LEDs.
 */
void led_startup_sweep(void);

/**
 * \brief Update slider bargraph LEDs based on current CapSense centroid.
 *
 * \param[in] position Centroid value (0 to 100), or 0 if untouched.
 * \param[in] is_touched True if an active touch is present on the slider.
 */
void led_update_slider(uint16_t position, bool is_touched);

/**
 * \brief Control the User LED (P3.4, Blue, Active-High).
 *
 * \param[in] state True = ON, False = OFF.
 */
void led_set_user(bool state);

/**
 * \brief Toggle the User LED state (useful for heartbeats / tick indicators).
 */
void led_toggle_user(void);

/**
 * \brief Flash all slider LEDs simultaneously (used for error / LTS alerts).
 *
 * \param[in] count Number of flashes.
 * \param[in] delay_ms Flash duration in milliseconds.
 */
void led_flash_all(uint32_t count, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* LED_TASK_H */
