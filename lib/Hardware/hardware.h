/*
 * hardware.h - Hardware abstraction layer for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../../config/config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================================
    // LED CONTROL
    // ============================================================================

    typedef enum
    {
        LED_1ER_RING = 0,
        LED_3ER_RING,
        LED_4ER_RING,
        LED_5ER_RING,
        LED_COUNT_MAX
    } led_id_t;

    // Hardware initialization
    void hardware_init(void);
    void hardware_update(void);

    // LED Functions
    void hardware_led_set(led_id_t led, uint8_t brightness);
    void hardware_led_all_off(void);

    // PWM Functions for LED brightness control
    void hardware_init_pwm(void);

    // ============================================================================
    // TIMING FUNCTIONS
    // ============================================================================

    uint32_t hardware_get_millis(void);

    // ============================================================================
    // MICROPHONE SENSOR (ADC)
    // ============================================================================

    void hardware_microphone_init(void);
    uint16_t hardware_microphone_read(void);
    uint16_t hardware_microphone_read_filtered(void);

    // ============================================================================
    // AUDIO OUTPUT (PWM)
    // ============================================================================

    void hardware_audio_set_frequency(uint16_t frequency);
    void hardware_audio_stop(void);

    // ============================================================================
    // EEPROM STORAGE (Persistent storage across resets)
    // ============================================================================

    uint8_t hardware_eeprom_read_byte(uint16_t address);
    void hardware_eeprom_write_byte(uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // HARDWARE_H_