/*
 * sensors.cpp - Sensor system for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 *
 * Clean ATtiny85 implementation for microphone sensor processing
 */

#include "sensors.h"
#include <string.h>
#include "../Hardware/hardware.h"
#include "../Lighting/lighting.h"
#if FEATURE_AUDIO_OUTPUT
#include "../Audio/audio.h"
#endif

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

typedef struct
{
    bool initialized;
    uint32_t last_update_time;
    uint16_t baseline;
    uint16_t light_threshold;
    uint16_t strong_threshold;
    uint32_t update_interval_ms;
    uint16_t current_raw;
    uint16_t breath_intensity;
    uint32_t strong_threshold_start_time;
    bool strong_threshold_active;

    // Post-song cooldown to prevent immediate re-triggering
    uint32_t song_cooldown_end_time; // When cooldown period ends
    bool in_song_cooldown;           // Whether we're in cooldown period
} sensors_state_t;

static sensors_state_t g_sensors_state;

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

bool sensors_init(void)
{
    memset(&g_sensors_state, 0, sizeof(g_sensors_state));

    // Initialize hardware microphone
    hardware_microphone_init();

    // Set default values
    g_sensors_state.baseline = 0; // Will be set by calibration
    g_sensors_state.light_threshold = BREATH_LIGHT_THRESHOLD;
    g_sensors_state.strong_threshold = BREATH_STRONG_THRESHOLD;
    g_sensors_state.update_interval_ms = 40;
    g_sensors_state.last_update_time = hardware_get_millis();

#if FEATURE_AUDIO_OUTPUT
    // Note: Now using rotating song system for strong breath triggers
#endif

    g_sensors_state.initialized = true;

    // Perform initial calibration (normal mode with smoothing)
    sensors_calibrate(false);

    return true;
}

void sensors_update(void)
{
    if (!g_sensors_state.initialized)
    {
        return;
    }

    uint32_t current_time = hardware_get_millis();

    // Check update interval
    if (current_time - g_sensors_state.last_update_time < g_sensors_state.update_interval_ms)
    {
        return;
    }

    g_sensors_state.last_update_time = current_time;

    // SIMPLIFIED: Basic breath detection with fixed baseline
    uint16_t raw_value = hardware_microphone_read();
    g_sensors_state.current_raw = raw_value;

    // Simple fixed threshold detection (no complex baseline tracking)
    uint16_t breath_threshold = g_sensors_state.baseline + g_sensors_state.light_threshold;
    uint16_t strong_threshold = g_sensors_state.baseline + g_sensors_state.strong_threshold;

    if (raw_value > strong_threshold)
    {
        // Strong breath - trigger song immediately (no duration check)
        audio_play_next_melody();
        return;
    }
    else if (raw_value > breath_threshold)
    {
        // Light breath - candle effect
        uint8_t boost = ((raw_value - breath_threshold) * 50) / g_sensors_state.light_threshold;
        lighting_set_candle_intensity_boost(boost > 50 ? 50 : boost);
    }
    else
    {
        // No breath - normal candle
        lighting_set_candle_intensity_boost(0);
    }
}

void sensors_shutdown(void)
{
    g_sensors_state.initialized = false;
}

uint16_t sensors_get_raw_value(void)
{
    return g_sensors_state.current_raw;
}

uint16_t sensors_get_mean_value(void)
{
    return g_sensors_state.current_raw; // Simplified - no buffering
}

void sensors_calibrate(bool force_immediate)
{
    (void)force_immediate; // Suppress unused parameter warning
    // SIMPLIFIED: Fixed baseline approach - eliminates all calibration complexity
    g_sensors_state.baseline = 200; // Reasonable fixed value - adjust if needed
}

void sensors_force_recalibration(void)
{
    // Wrapper for immediate calibration after audio events
    sensors_calibrate(true);
}


// ============================================================================
// BREATH DETECTION API FUNCTIONS
// ============================================================================

bool sensors_is_breath_detected(void)
{
    // Simplified: breath is detected if current intensity > 0
    return g_sensors_state.breath_intensity > 0;
}

uint8_t sensors_get_breath_intensity(void)
{
    // Enhanced breath response with exponential curve for more dramatic effect
    if (g_sensors_state.breath_intensity == 0)
    {
        return 0;
    }

    uint16_t range_size = g_sensors_state.strong_threshold - g_sensors_state.light_threshold;
    if (range_size == 0)
        return LED_BREATH_MAX_BOOST; // Avoid division by zero

    uint16_t intensity_above_light = (g_sensors_state.breath_intensity > g_sensors_state.light_threshold)
                                         ? (g_sensors_state.breath_intensity - g_sensors_state.light_threshold)
                                         : 0;

    // Normalize to 0-255 range for easier math
    uint16_t normalized = (intensity_above_light * 255) / range_size;
    if (normalized > 255)
        normalized = 255;

    // Apply exponential curve: squared response for more dramatic effect
    // This makes small increases in breath result in larger LED brightness jumps
    uint16_t curved = (normalized * normalized) / 255; // Square the response (0-255)

    // Map back to boost range: min_boost to max_boost
    uint8_t boost_range = LED_BREATH_MAX_BOOST - LED_BREATH_MIN_BOOST;
    uint16_t boost = LED_BREATH_MIN_BOOST + ((curved * boost_range) / 255);

    return (boost > LED_BREATH_MAX_BOOST) ? LED_BREATH_MAX_BOOST : (uint8_t)boost;
}
