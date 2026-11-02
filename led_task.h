/******************************************************************************
 * \file    led_task.h
 * \version 2.1.0
 *
 * \brief
 * Multi-Mode LED visual controller for CY8CKIT-149.
 *
 * \details
 * Supports multiple visual interaction modes:
 *  - Mode 0 (Bargraph): Solid fill proportional to touch coordinate.
 *  - Mode 1 (Dot Tracker): Single illuminated LED tracking fingertip.
 *  - Mode 2 (Knight Rider / Wave): Active bounce animation modulated by touch speed.
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

#define LED_SLIDER_SEGMENT_COUNT    (5u)

/**
 * \brief Selectable visual display modes.
 */
typedef enum
{
    LED_MODE_BARGRAPH   = 0,  /**< Progressive bargraph fill */
    LED_MODE_SINGLE_DOT = 1,  /**< Single dot cursor tracking */
    LED_MODE_SCANNER    = 2,  /**< Larson / Knight Rider wave modulated by touch */
    LED_MODE_MAX
} led_mode_t;

/**
 * \brief Initialize LED GPIO pins and drive modes.
 */
void led_init(void);

/**
 * \brief Startup visual verification sweep.
 */
void led_startup_sweep(void);

/**
 * \brief Update slider LEDs based on current mode and touch coordinate.
 *
 * \param[in] position Centroid coordinate (0 to 100).
 * \param[in] is_touched True if touch is currently detected.
 */
void led_update_slider(uint16_t position, bool is_touched);

/**
 * \brief Cycle to the next visual display mode.
 *
 * \return New active led_mode_t.
 */
led_mode_t led_cycle_mode(void);

/**
 * \brief Retrieve current active display mode.
 */
led_mode_t led_get_mode(void);

/**
 * \brief Set specific display mode.
 */
void led_set_mode(led_mode_t mode);

/**
 * \brief Control blue User LED (P3.4).
 */
void led_set_user(bool state);

/**
 * \brief Toggle blue User LED.
 */
void led_toggle_user(void);

/**
 * \brief Flash all slider LEDs for error or mode change indication.
 */
void led_flash_all(uint32_t count, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* LED_TASK_H */
