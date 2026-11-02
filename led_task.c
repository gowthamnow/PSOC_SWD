/******************************************************************************
 * \file    led_task.c
 * \version 1.0.0
 *
 * \brief
 * LED control task implementation for CY8CKIT-149.
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#include "led_task.h"

/* Slider LED definitions (Active-Low) */
#define LED_ACTIVE_LOW_ON           (0u)
#define LED_ACTIVE_LOW_OFF          (1u)

/* User LED1 Blue (Active-High) */
#define USER_LED_ON                 (1u)
#define USER_LED_OFF                (0u)

/* Pin Mapping per CY8CKIT-149 Rev ** Schematics */
#define LED_SLD0_PORT               GPIO_PRT1
#define LED_SLD0_PIN                0U

#define LED_SLD1_PORT               GPIO_PRT1
#define LED_SLD1_PIN                2U

#define LED_SLD2_PORT               GPIO_PRT1
#define LED_SLD2_PIN                4U

#define LED_SLD3_PORT               GPIO_PRT1
#define LED_SLD3_PIN                6U

#define LED_SLD4_PORT               GPIO_PRT2
#define LED_SLD4_PIN                0U

#define USER_LED_PORT               GPIO_PRT3
#define USER_LED_PIN                4U

#define STEP_SIZE                   (100u / LED_SLIDER_SEGMENT_COUNT) /* 20 */

static GPIO_PRT_Type * const s_sld_ports[LED_SLIDER_SEGMENT_COUNT] = {
    LED_SLD0_PORT, LED_SLD1_PORT, LED_SLD2_PORT, LED_SLD3_PORT, LED_SLD4_PORT
};

static const uint32_t s_sld_pins[LED_SLIDER_SEGMENT_COUNT] = {
    LED_SLD0_PIN,  LED_SLD1_PIN,  LED_SLD2_PIN,  LED_SLD3_PIN,  LED_SLD4_PIN
};

static bool s_user_led_state = false;

void led_init(void)
{
    /* Configure all slider LED pins as Strong Drive outputs */
    for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
    {
        Cy_GPIO_SetDrivemode(s_sld_ports[i], s_sld_pins[i], CY_GPIO_DM_STRONG);
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
    }

    /* Configure User LED */
    Cy_GPIO_SetDrivemode(USER_LED_PORT, USER_LED_PIN, CY_GPIO_DM_STRONG);
    Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, USER_LED_OFF);
    s_user_led_state = false;
}

void led_startup_sweep(void)
{
    /* Forward sweep */
    for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
    {
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_ON);
        Cy_SysLib_Delay(60U);
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
    }

    /* Reverse sweep */
    for (int32_t i = (int32_t)(LED_SLIDER_SEGMENT_COUNT - 1u); i >= 0; i--)
    {
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_ON);
        Cy_SysLib_Delay(60U);
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
    }

    /* User LED confirmation pulse */
    Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, USER_LED_ON);
    Cy_SysLib_Delay(100U);
    Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, USER_LED_OFF);
}

void led_update_slider(uint16_t position, bool is_touched)
{
    if (is_touched)
    {
        /* Bargraph mode: LEDs turn on progressively with touch coordinate */
        for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
        {
            uint32_t threshold = i * STEP_SIZE;
            uint32_t val = (position >= threshold) ? LED_ACTIVE_LOW_ON : LED_ACTIVE_LOW_OFF;
            Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], val);
        }

        /* User LED active on touch */
        led_set_user(true);
    }
    else
    {
        /* Turn all off */
        for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
        {
            Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
        }

        led_set_user(false);
    }
}

void led_set_user(bool state)
{
    s_user_led_state = state;
    Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, state ? USER_LED_ON : USER_LED_OFF);
}

void led_toggle_user(void)
{
    s_user_led_state = !s_user_led_state;
    Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, s_user_led_state ? USER_LED_ON : USER_LED_OFF);
}

void led_flash_all(uint32_t count, uint32_t delay_ms)
{
    for (uint32_t c = 0u; c < count; c++)
    {
        for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
        {
            Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_ON);
        }
        Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, USER_LED_ON);

        Cy_SysLib_Delay(delay_ms);

        for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
        {
            Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
        }
        Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, USER_LED_OFF);

        Cy_SysLib_Delay(delay_ms);
    }
}
