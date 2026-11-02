/******************************************************************************
 * \file    capsense_task.c
 * \version 1.0.0
 *
 * \brief
 * CapSense sensing module implementation for Infineon CY8CKIT-149.
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#include "capsense_task.h"
#include "cybsp.h"

#define CAPSENSE_INTR_PRIORITY      (3u)

static capsense_callback_t s_touch_callback = NULL;
static uint16_t s_slider_pos = 0u;
static bool s_is_touched = false;

/* Hardware ISR dispatch function */
static void capsense_isr(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}

cy_rslt_t capsense_init(void)
{
    cy_capsense_status_t status = CY_CAPSENSE_STATUS_SUCCESS;

    const cy_stc_sysint_t capsense_interrupt_config =
    {
        .intrSrc = CYBSP_CSD_IRQ,
        .intrPriority = CAPSENSE_INTR_PRIORITY,
    };

    /* Initialize CSD HW block and CapSense data structures */
    status = Cy_CapSense_Init(&cy_capsense_context);
    if (status != CY_CAPSENSE_STATUS_SUCCESS)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    /* Configure and enable CapSense NVIC interrupt */
    Cy_SysInt_Init(&capsense_interrupt_config, capsense_isr);
    NVIC_ClearPendingIRQ(capsense_interrupt_config.intrSrc);
    NVIC_EnableIRQ(capsense_interrupt_config.intrSrc);

    /* Enable CapSense firmware sensing module */
    status = Cy_CapSense_Enable(&cy_capsense_context);
    if (status != CY_CAPSENSE_STATUS_SUCCESS)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    /* Trigger the first scan */
    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

    return CY_RSLT_SUCCESS;
}

void capsense_register_callback(capsense_callback_t callback)
{
    s_touch_callback = callback;
}

void capsense_process(void)
{
    /* Check if current scan is complete */
    if (CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context))
    {
        /* Process all widgets: filter raw counts, update baselines, calc centroids */
        Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);

        /* Query linear slider widget */
        if (Cy_CapSense_IsWidgetActive(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, &cy_capsense_context))
        {
            cy_stc_capsense_touch_t *touch = Cy_CapSense_GetTouchInfo(
                CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, &cy_capsense_context);

            if (touch != NULL && touch->numPosition > 0)
            {
                s_slider_pos = touch->ptrPosition->x;
                s_is_touched = true;
            }
            else
            {
                s_is_touched = false;
            }
        }
        else
        {
            s_is_touched = false;
        }

        /* Dispatch registered callback */
        if (s_touch_callback != NULL)
        {
            s_touch_callback(s_slider_pos, s_is_touched);
        }

        /* Initiate next scan cycle */
        Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
    }
}

uint16_t capsense_get_slider_position(void)
{
    return s_slider_pos;
}

bool capsense_is_slider_touched(void)
{
    return s_is_touched;
}

cy_rslt_t capsense_get_sensor_data(uint32_t sensor_idx, uint16_t *out_raw, uint16_t *out_base)
{
    if (out_raw == NULL || out_base == NULL)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    if (sensor_idx >= cy_capsense_context.ptrWdConfig[CY_CAPSENSE_LINEARSLIDER0_WDGT_ID].numSns)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    const cy_stc_capsense_widget_config_t *wd_cfg =
        &cy_capsense_context.ptrWdConfig[CY_CAPSENSE_LINEARSLIDER0_WDGT_ID];

    if (wd_cfg->ptrSnsContext != NULL)
    {
        *out_raw  = wd_cfg->ptrSnsContext[sensor_idx].raw;
        *out_base = wd_cfg->ptrSnsContext[sensor_idx].bsln;
        return CY_RSLT_SUCCESS;
    }

    return CY_RSLT_TYPE_ERROR;
}
