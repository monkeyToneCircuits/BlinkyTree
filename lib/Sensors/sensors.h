/*
 * sensors.h - Sensor system for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../config/config.h"
#include "../Hardware/hardware.h"
#if FEATURE_AUDIO_OUTPUT
#include "../Audio/audio.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================================
    // SENSOR CONFIGURATION
    // ============================================================================

// Configurable sensor parameters
#define SENSORS_BUFFER_SIZE 16                // Smaller buffer for mean calculation
#define SENSORS_DEFAULT_UPDATE_INTERVAL_MS 10 // Default update interval

// Breath detection thresholds (for electret mic + peak detector circuit)
// Use configurable values from config.h for adjustable sensitivity
// Note: Baseline is now dynamically calibrated - no static define needed
#define BREATH_THRESHOLD_START BREATH_LIGHT_THRESHOLD   // Light breath - capacitor voltage for gentle blow
#define BREATH_THRESHOLD_STRONG BREATH_STRONG_THRESHOLD // Strong breath - capacitor voltage for strong blow    // ============================================================================
    // SENSOR DATA STRUCTURES
    // ============================================================================

    typedef struct
    {
        uint16_t raw_value;
        uint16_t mean_value;
    } sensor_reading_t;

    // ============================================================================
    // SENSOR SYSTEM FUNCTIONS
    // ============================================================================

    // System initialization and control
    bool sensors_init(void);
    void sensors_update(void);
    void sensors_shutdown(void);

    // Sensor readings
    uint16_t sensors_get_raw_value(void);
    uint16_t sensors_get_mean_value(void);

    // Calibration
    void sensors_calibrate(bool force_immediate);
    void sensors_force_recalibration(void);

    // Basic breath detection
    bool sensors_is_breath_detected(void);
    uint8_t sensors_get_breath_intensity(void);

#ifdef __cplusplus
}
#endif

#endif // SENSORS_H_