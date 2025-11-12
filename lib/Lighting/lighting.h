/*
 * lighting.h - LED lighting effects for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef LIGHTING_H_
#define LIGHTING_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../config/config.h"
#include "../Hardware/hardware.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================================
    // LIGHTING CONFIGURATION
    // ============================================================================

#define LIGHTING_MAX_BRIGHTNESS 255
#define LIGHTING_DEFAULT_SPEED 100
#define LIGHTING_UPDATE_INTERVAL_MS 50

    // Startup Animation Configuration
#define STARTUP_ANIMATION_DURATION_MS 1000  // Total duration of startup sequence
#define STARTUP_STEP_DELAY_MS 150           // Delay between each LED ring lighting up
#define STARTUP_FLASH_DURATION_MS 250       // Duration of final brightness flash
#define STARTUP_DARK_PAUSE_MS 80           // Duration of dark pause after flash
#define STARTUP_BUILDUP_BRIGHTNESS 85       // 1/3 of max brightness (255/3) for buildup phase

    // ============================================================================
    // LIGHTING EFFECTS
    // ============================================================================

    typedef enum
    {
        LIGHTING_EFFECT_NONE = 0,
        LIGHTING_EFFECT_STATIC,
        LIGHTING_EFFECT_BREATHING,
        LIGHTING_EFFECT_CHRISTMAS_TRADITIONAL,
        LIGHTING_EFFECT_WINTER_WONDERLAND,
        LIGHTING_EFFECT_CANDLE,
        LIGHTING_EFFECT_CANDLE_BLOWN,   // Special candle effect when breath is detected
        LIGHTING_EFFECT_CANDLE_1_RING,  // Only top ring (LED_1ER_RING) flickers
        LIGHTING_EFFECT_CANDLE_2_RINGS, // Top 2 rings (LED_1ER_RING + LED_3ER_RING) flicker
        LIGHTING_EFFECT_CANDLE_3_RINGS, // Top 3 rings (LED_1ER_RING + LED_3ER_RING + LED_4ER_RING) flicker
        LIGHTING_EFFECT_CANDLE_4_RINGS, // All 4 rings flicker (same as LIGHTING_EFFECT_CANDLE)
        LIGHTING_EFFECT_CELEBRATION,
        LIGHTING_EFFECT_STARTUP,
        LIGHTING_EFFECT_BRIGHTNESS_TEST,   // Rotates LEDs with cycling brightness
        LIGHTING_EFFECT_ADC_TEST,          // ADC sensor test mode - LEDs light up based on sensor input
        LIGHTING_EFFECT_MICROPHONE_LEVELS, // Direct microphone intensity visualization
        LIGHTING_EFFECT_COUNT
    } lighting_effect_t;

    typedef struct
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } rgb_color_t;

// Pre-defined colors
#define COLOR_OFF {0, 0, 0}
#define COLOR_WHITE {255, 255, 255}
#define COLOR_RED {255, 0, 0}
#define COLOR_GREEN {0, 255, 0}
#define COLOR_BLUE {0, 0, 255}
#define COLOR_YELLOW {255, 255, 0}
#define COLOR_ORANGE {255, 165, 0}
#define COLOR_PURPLE {128, 0, 128}
#define COLOR_WARM_WHITE {255, 200, 150}

    // ============================================================================
    // LIGHTING SYSTEM FUNCTIONS
    // ============================================================================

    // System initialization and control
    bool lighting_init(void);
    void lighting_update(void);
    void lighting_startup_animation(void);  // Blocking startup animation

    // Effect control
    void lighting_set_effect(lighting_effect_t effect);
    void lighting_set_candle_intensity_boost(uint8_t boost); // 0-100 additional intensity

    // Audio-reactive lighting functions (simple on/off control - 4-LED setup)
    void lighting_audio_reactive_note(uint16_t frequency); // Light LED rings based on note frequency (direct pin control)
    void lighting_audio_reactive_off(void);                // Turn off all audio-reactive LEDs (direct pin control)

    // All other lighting effects removed - were never implemented

#ifdef __cplusplus
}
#endif

#endif // LIGHTING_H_