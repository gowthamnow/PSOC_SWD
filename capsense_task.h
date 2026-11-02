/******************************************************************************
 * \file    capsense_task.h
 * \version 1.0.0
 *
 * \brief
 * CapSense sensing module task for Infineon CY8CKIT-149.
 *
 * \details
 * Encapsulates the CapSense CSD hardware block initialization, ISR dispatch,
 * automated scanning, calibration, and widget processing.
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#ifndef CAPSENSE_TASK_H
#define CAPSENSE_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "cy_pdl.h"
#include "cycfg_capsense.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Callback signature invoked when a CapSense scan and processing cycle completes.
 *
 * \param[in] position Centroid coordinate (0 to 100), or 0 if untouched.
 * \param[in] is_touched True if an active touch is detected on the slider.
 */
typedef void (*capsense_callback_t)(uint16_t position, bool is_touched);

/**
 * \brief Initialize the CSD block, configure NVIC interrupts, and start sensing.
 *
 * \return CY_RSLT_SUCCESS on successful initialization, or error code.
 */
cy_rslt_t capsense_init(void);

/**
 * \brief Register an optional application callback for processed touch events.
 *
 * \param[in] callback Function pointer to receive touch events.
 */
void capsense_register_callback(capsense_callback_t callback);

/**
 * \brief Main polling function for CapSense. Must be called repeatedly in the application loop.
 *
 * \details Non-blocking. Checks if CSD hardware completed a scan, processes widgets,
 *          notifies registered callbacks, and automatically initiates the next scan.
 */
void capsense_process(void);

/**
 * \brief Retrieve the latest processed slider centroid coordinate.
 *
 * \return Position value in range 0..100.
 */
uint16_t capsense_get_slider_position(void);

/**
 * \brief Check if the linear slider is currently being touched.
 *
 * \return True if touched, False otherwise.
 */
bool capsense_is_slider_touched(void);

/**
 * \brief Read the raw count and baseline of a specific sensor element for diagnostics.
 *
 * \param[in]  sensor_idx Sensor element index (0 to 4 for SNS0..SNS4).
 * \param[out] out_raw    Pointer to receive raw count.
 * \param[out] out_base   Pointer to receive sensor baseline.
 * \return CY_RSLT_SUCCESS if valid index, error code otherwise.
 */
cy_rslt_t capsense_get_sensor_data(uint32_t sensor_idx, uint16_t *out_raw, uint16_t *out_base);

#ifdef __cplusplus
}
#endif

#endif /* CAPSENSE_TASK_H */
