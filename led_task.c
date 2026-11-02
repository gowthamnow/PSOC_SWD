/******************************************************************************
 * \file    led_task.c
 * \version 2.1.0
 *
 * \brief
 * Multi-Mode LED visual controller implementation for CY8CKIT-149.
 *
 * \copyright
 * (c) 2026, Infineon Technologies AG / Professional Embedded Engineering.
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#include "led_task.h"

/* Active-Low slider LED definitions */
#define LED_ACTIVE_LOW_ON           (0u)
#define LED_ACTIVE_LOW_OFF          (1u)

/* User LED1 Blue (Active-High) */
#define USER_LED_ON                 (1u)
#define USER_LED_OFF                (0u)

/* Hardware port and pin mapping */
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

static led_mode_t s_current_mode = LED_MODE_BARGRAPH;
static bool s_user_led_state = false;
static uint32_t s_scanner_index = 0u;
static int32_t s_scanner_dir = 1;
static uint32_t s_tick_counter = 0u;

void led_init(void)
{
    for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
    {
        Cy_GPIO_SetDrivemode(s_sld_ports[i], s_sld_pins[i], CY_GPIO_DM_STRONG);
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
    }

    Cy_GPIO_SetDrivemode(USER_LED_PORT, USER_LED_PIN, CY_GPIO_DM_STRONG);
    Cy_GPIO_Write(USER_LED_PORT, USER_LED_PIN, USER_LED_OFF);
    s_user_led_state = false;
    s_current_mode = LED_MODE_BARGRAPH;
}

void led_startup_sweep(void)
{
    /* Bidirectional visual check */
    for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
    {
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_ON);
        Cy_SysLib_Delay(50U);
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
    }

    for (int32_t i = (int32_t)(LED_SLIDER_SEGMENT_COUNT - 1u); i >= 0; i--)
    {
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_ON);
        Cy_SysLib_Delay(50U);
        Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
    }

    /* Double flash confirmation */
    led_set_user(true);
    Cy_SysLib_Delay(80U);
    led_set_user(false);
}

led_mode_t led_cycle_mode(void)
{
    s_current_mode = (led_mode_t)((s_current_mode + 1u) % LED_MODE_MAX);

    /* Visual confirmation of new mode: blink N times corresponding to mode */
    for (uint32_t b = 0u; b <= (uint32_t)s_current_mode; b++)
    {
        led_set_user(true);
        Cy_SysLib_Delay(60U);
        led_set_user(false);
        Cy_SysLib_Delay(60U);
    }

    return s_current_mode;
}

led_mode_t led_get_mode(void)
{
    return s_current_mode;
}

void led_set_mode(led_mode_t mode)
{
    if (mode < LED_MODE_MAX)
    {
        s_current_mode = mode;
    }
}

void led_update_slider(uint16_t position, bool is_touched)
{
    s_tick_counter++;

    if (is_touched)
    {
        led_set_user(true);

        switch (s_current_mode)
        {
            case LED_MODE_BARGRAPH:
            {
                /* Bargraph fill mode: all LEDs up to position are active */
                for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
                {
                    uint32_t threshold = i * STEP_SIZE;
                    uint32_t val = (position >= threshold) ? LED_ACTIVE_LOW_ON : LED_ACTIVE_LOW_OFF;
                    Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], val);
                }
                break;
            }

            case LED_MODE_SINGLE_DOT:
            {
                /* Single beacon cursor mode: only the single active segment lights up */
                uint32_t active_idx = position / STEP_SIZE;
                if (active_idx >= LED_SLIDER_SEGMENT_COUNT)
                {
                    active_idx = LED_SLIDER_SEGMENT_COUNT - 1u;
                }

                for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
                {
                    uint32_t val = (i == active_idx) ? LED_ACTIVE_LOW_ON : LED_ACTIVE_LOW_OFF;
                    Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], val);
                }
                break;
            }

            case LED_MODE_SCANNER:
            {
                /* Scanner mode: touch position modulates speed of wave bounce */
                uint32_t speed_divider = 5u + (100u - position) / 10u; /* faster when touched higher */
                if ((s_tick_counter % speed_divider) == 0u)
                {
                    if (s_scanner_dir > 0)
                    {
                        if (s_scanner_index + 1u < LED_SLIDER_SEGMENT_COUNT)
                        {
                            s_scanner_index++;
                        }
                        else
                        {
                            s_scanner_dir = -1;
                        }
                    }
                    else
                    {
                        if (s_scanner_index > 0u)
                        {
                            s_scanner_index--;
                        }
                        else
                        {
                            s_scanner_dir = 1;
                        }
                    }
                }

                for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
                {
                    uint32_t val = (i == s_scanner_index) ? LED_ACTIVE_LOW_ON : LED_ACTIVE_LOW_OFF;
                    Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], val);
                }
                break;
            }

            default:
                break;
        }
    }
    else
    {
        /* Idle state */
        if (s_current_mode == LED_MODE_SCANNER)
        {
            /* Slow idle wave bounce in scanner mode */
            if ((s_tick_counter % 25u) == 0u)
            {
                if (s_scanner_dir > 0)
                {
                    if (s_scanner_index + 1u < LED_SLIDER_SEGMENT_COUNT)
                    {
                        s_scanner_index++;
                    }
                    else
                    {
                        s_scanner_dir = -1;
                    }
                }
                else
                {
                    if (s_scanner_index > 0u)
                    {
                        s_scanner_index--;
                    }
                    else
                    {
                        s_scanner_dir = 1;
                    }
                }
            }

            for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
            {
                uint32_t val = (i == s_scanner_index) ? LED_ACTIVE_LOW_ON : LED_ACTIVE_LOW_OFF;
                Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], val);
            }
            led_set_user(false);
        }
        else
        {
            /* Turn off in other modes */
            for (uint32_t i = 0u; i < LED_SLIDER_SEGMENT_COUNT; i++)
            {
                Cy_GPIO_Write(s_sld_ports[i], s_sld_pins[i], LED_ACTIVE_LOW_OFF);
            }
            led_set_user(false);
        }
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
