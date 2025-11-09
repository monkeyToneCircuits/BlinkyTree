#ifndef CONFIG_H_
#define CONFIG_H_

/*
 * BlinkyTree Configuration File
 * ATtiny85 Interactive Christmas Tree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 *
 * This file contains all hardware and feature configurations
 * for both debug and release builds.
 */

// ============================================================================
// PROJECT INFORMATION
// ============================================================================
#define PROJECT_NAME "BlinkyTree"
#define PROJECT_VERSION "1.0.0" // First production release
#define PROJECT_AUTHOR "Dschi"

// ============================================================================
// FEATURE CONFIGURATION - MAIN CONTROL CENTER
// ============================================================================


// Lighting Effects - chose ONE !!!
#define FEATURE_CANDLE_EFFECT 1       // Flickering candle simulation - ACTIVE
#define FEATURE_BREATHING_EFFECT 0    // Smooth breathing animation - Available but disabled
#define FEATURE_CANDLE_EFFECT_KRANZ 0 // Flickering candle simulation (Kranz) - Available but disabled

// Sensor Support
#define FEATURE_MICROPHONE_SENSOR 1 // Breath detection via microphone (enabled for testing)

// Audio System 
#define FEATURE_AUDIO_OUTPUT 1   // Full-featured buzzer/speaker support

// Advanced Features
#define FEATURE_EEPROM_SETTINGS 1     // Persistent settings storage
#define FEATURE_MEMORY_OPTIMIZATION 1 // Aggressive size optimization

// ============================================================================
// PIN ASSIGNMENTS (ATtiny85)
// ============================================================================
/*
 * ATtiny85 Pinout - New HW Variant:
 * Pin 1 (PB5/RESET): Reset/Programming (ALWAYS functional - keeps ISP) [ISP: RESET]
 * Pin 2 (PB3/ADC3):  Microphone + LED_3ER (SHARED - time-multiplexed)
 * Pin 3 (PB4):       Buzzer (dedicated)
 * Pin 4 (GND):       Ground [ISP: GND]
 * Pin 5 (PB0/OC0A):  LED_4ER (hardware PWM) [ISP: MOSI]
 * Pin 6 (PB1/OC0B):  LED_5ER (hardware PWM) [ISP: MISO]
 * Pin 7 (PB2):       LED_1ER (software PWM) [ISP: SCK]
 * Pin 8 (VCC):       +3V Power [ISP: VCC]
 */

// ============================================================================
// PIN ASSIGNMENTS - Conditional based on build type
// ============================================================================

#ifdef RESET_PIN_AS_IO

#ifdef OLD_HARDWARE_REVISION
/*
 * OLD HARDWARE REVISION - Reset pin disabled, used as BUZZER
 * Pin 1 (PB5): BUZZER (dedicated GPIO - no ISP possible!)
 * Pin 2 (PB3): Microphone (dedicated ADC - no sharing needed)
 * Pin 3 (PB4): LED_3ER (dedicated - no sharing!)
 * Pin 4 (GND): Ground
 * Pin 5 (PB0): LED_4ER (hardware PWM)
 * Pin 6 (PB1): LED_5ER (hardware PWM)
 * Pin 7 (PB2): LED_1ER (software PWM)
 * Pin 8 (VCC): +3V Power
 */

// Old Hardware Pin Assignments - PB5 as BUZZER
#define PIN_LED_1ER PB2   // Pin 7 - LED Ring 1 (software PWM)
#define PIN_LED_3ER PB4   // Pin 3 - LED Ring 3 (DEDICATED - was shared in debug!)
#define PIN_LED_4ER PB0   // Pin 5 - LED Ring 4 (hardware PWM)
#define PIN_LED_5ER PB1   // Pin 6 - LED Ring 5 (hardware PWM)
#define PIN_MIC_INPUT PB3 // Pin 2 - Microphone ADC input (DEDICATED - no sharing!)
#define BUZZER_PIN PB5    // Pin 1 - BUZZER (was reset pin!)

// No shared pins in old hardware production build
#define SHARED_PIN_MIC_LED 0xFF // No shared pin - all dedicated

#else
/*
 * NEW HARDWARE REVISION - Reset pin disabled, used as LED_3ER
 * Pin 1 (PB5): LED_3ER (dedicated GPIO - no ISP possible!)
 * Pin 2 (PB3): Microphone (dedicated ADC - no sharing needed)
 * Pin 3 (PB4): Buzzer (dedicated)
 * Pin 4 (GND): Ground
 * Pin 5 (PB0): LED_4ER (hardware PWM)
 * Pin 6 (PB1): LED_5ER (hardware PWM)
 * Pin 7 (PB2): LED_1ER (software PWM)
 * Pin 8 (VCC): +3V Power
 */

// New Hardware Pin Assignments - PB5 as LED_3ER
#define PIN_LED_1ER PB2         // Pin 7 - LED Ring 1 (software PWM)
#define PIN_LED_3ER PB5         // Pin 1 - LED Ring 3 (DEDICATED - was reset pin!)
#define PIN_LED_4ER PB0         // Pin 5 - LED Ring 4 (hardware PWM)
#define PIN_LED_5ER PB1         // Pin 6 - LED Ring 5 (hardware PWM)
#define PIN_MIC_INPUT PB3       // Pin 2 - Microphone ADC input (DEDICATED - no sharing!)
#define BUZZER_PIN PB4          // Pin 3 - Buzzer (dedicated, no sharing)

// No shared pins in new hardware production build
#define SHARED_PIN_MIC_LED 0xFF // No shared pin - all dedicated

#endif // OLD_HARDWARE_REVISION

#else

// Debug Pin Assignments - Reset pin reserved for ISP
#define PIN_RESET PB5          // Pin 1 - Reset/Programming (always functional)
#define PIN_LED_1ER PB2        // Pin 7 - LED Ring 1 (software PWM)
#define PIN_LED_3ER PB3        // Pin 2 - LED Ring 3 (SHARED with microphone - time-multiplexed)
#define PIN_LED_4ER PB0        // Pin 5 - LED Ring 4 (hardware PWM)
#define PIN_LED_5ER PB1        // Pin 6 - LED Ring 5 (hardware PWM)
#define PIN_MIC_INPUT PB3      // Pin 2 - Microphone ADC input (shared with LED_3ER)
#define BUZZER_PIN PB4         // Pin 3 - Buzzer (dedicated, no sharing)

// Shared Pin Configuration for debug builds
#define SHARED_PIN_MIC_LED PB3 // Pin that is time-multiplexed between mic and LED_3ER

// ISP Programming Pin Assignments (6-pin connector) - DEBUG ONLY
#define ISP_PIN_RESET PB5      // Pin 1 - Reset (shared with PIN_RESET)
#define ISP_PIN_SCK PB2        // Pin 7 - Serial Clock (shared with PIN_LED_1ER)
#define ISP_PIN_MISO PB1       // Pin 6 - Master In Slave Out (shared with PIN_LED_5ER)
#define ISP_PIN_MOSI PB0       // Pin 5 - Master Out Slave In (shared with PIN_LED_4ER)
#define ISP_PIN_VCC 8          // Pin 8 - Programming voltage (+3V or +5V)
#define ISP_PIN_GND 4          // Pin 4 - Ground

#endif

// ============================================================================
// BUILD TYPE DETECTION
// ============================================================================
#ifdef PRODUCTION_BUILD
#define IS_PRODUCTION_BUILD 1
#define IS_DEBUG_BUILD 0
#else // DEBUG_BUILD
#define IS_PRODUCTION_BUILD 0
#define IS_DEBUG_BUILD 1
#endif

// ============================================================================
// FEATURE CONFIGURATION
// ============================================================================

// LED System Configuration
#define LED_COUNT 4
#define LED_BRIGHTNESS_DEFAULT 30 // Reduced from 30 - dimmer baseline for better breath visibility
#define LED_BRIGHTNESS_MIN 10     // Reduced from 10 - allows even dimmer base

// Audio System Configuration
// Note: BUZZER_PIN defined in pin assignments above
// Audio frequency and volume settings (duplicated from audio.h)
#define AUDIO_MAX_FREQUENCY 4000 // Hz
#define AUDIO_MIN_FREQUENCY 100  // Hz
#define AUDIO_DEFAULT_VOLUME 128 // 0-255

// Note separation - small gap between notes to distinguish identical consecutive notes
#define AUDIO_NOTE_GAP_MS 50 // Milliseconds of silence between notes (prevents identical notes from blending)

// ============================================================================
// AUDIO SYSTEM CONFIGURATION
// ============================================================================

// Song Selection (Compile-time configuration)
// Enable/disable individual songs to save Flash memory
// Set to 0 to disable songs and save Flash memory

// Christmas Songs
#define ENABLE_OH_CHRISTMAS_TREE 1 // Classic German Christmas carol
#define ENABLE_SILENT_NIGHT 1      // Traditional Christmas favorite
#define ENABLE_JINGLE_BELLS 1      // Fun Christmas tune
#define ENABLE_NOEL 1              // Christmas carol "The First Noel"
#define ENABLE_GLING_KLOECKCHEN 1  // German Christmas song "Kling, Glöckchen"

// Theme Songs
#define ENABLE_ZELDA_THEME 1    // Legend of Zelda main theme
#define ENABLE_IMPERIAL_MARCH 1 // Star Wars Imperial March (Darth Vader)

// Song Rotation Configuration
#define ENABLE_SONG_ROTATION 1      // Enable automatic song rotation through enabled songs
#define SONG_ROTATION_MODE 1        // 0=random selection, 1=sequential order

// Startup Configuration
#define PLAY_STARTUP_MELODY 0 // Play current rotation song at startup (1=enabled, 0=disabled)

// ============================================================================
// PERSISTENT STORAGE CONFIGURATION
// ============================================================================

// EEPROM Storage Addresses
#define EEPROM_ADDR_SONG_ROTATION_INDEX 0x00 // Persistent song rotation index (1 byte)

// ============================================================================
// SENSOR SYSTEM CONFIGURATION
// ============================================================================

// Breath Detection Thresholds
// Lower values = more sensitive, higher values = less sensitive
// Note: Baseline is now dynamically calibrated, only thresholds are static
#define BREATH_LIGHT_THRESHOLD 1       // Light breath threshold for LED response
#define BREATH_STRONG_THRESHOLD 50     // Strong breath threshold for audio trigger - more sensitive for reliable detection (50 for easy)
#define BREATH_STRONG_MIN_DURATION 100 // Minimum duration (ms) strong threshold must be held to trigger audio - realistic with 10ms updates
#define SONG_COOLDOWN_DURATION 3000    // Cooldown period (ms) after song ends - allows charge dissipation

// LED Breath Response - Simple linear mapping for consistent behavior
#define LED_BREATH_MIN_BOOST 40  // Minimum LED boost (0-255) for barely detectable breath
#define LED_BREATH_MAX_BOOST 200 // Maximum LED boost (0-255) for strong breath (below audio trigger)

// ============================================================================
// LIGHTING SYSTEM CONFIGURATION
// ============================================================================

// Candle Effect Base Brightness - Percentage values that scale with LED_BRIGHTNESS_DEFAULT
// These percentages are multiplied by LED_BRIGHTNESS_DEFAULT to get actual brightness values
#define CANDLE_TIP_BRIGHTNESS_PCT 140   // Tip LED brightness as % of LED_BRIGHTNESS_DEFAULT
#define CANDLE_UPPER_BRIGHTNESS_PCT 75  // Upper LED brightness as % of LED_BRIGHTNESS_DEFAULT
#define CANDLE_MIDDLE_BRIGHTNESS_PCT 50 // Middle LED brightness as % of LED_BRIGHTNESS_DEFAULT
#define CANDLE_BASE_BRIGHTNESS_PCT 40   // Base LED brightness as % of LED_BRIGHTNESS_DEFAULT

// Audio-Reactive Light Effect Configuration
// LED ring activation based on musical note ranges during song playback
// LOGICAL MAPPING: Higher notes light up lower-numbered rings (top to bottom)
// SIMPLE CONTROL: Direct pin manipulation - LEDs are either ON or OFF (no PWM)
#define AUDIO_REACTIVE_ENABLE 1 // Enable audio-reactive light effect during songs (1=enabled, 0=disabled)

// Note thresholds for LED ring activation (using note definitions from audio.h)
// LED rings light up when played note frequency falls in their range
//
// 4-LED MAPPING (optimized for Silent Night coverage):
// LED_3ER used during songs (microphone inactive during audio output)
// LED_1ER (Top/Tip):    Highest notes C5 and above (C5, D5+)
// LED_3ER (Upper):      High-mid notes A4 to B4 (A4, B4)
// LED_4ER (Middle):     Mid notes F4 to G4 (G4)
// LED_5ER (Base/Bottom): Low notes E4 and below (E4)
//
#define AUDIO_NOTE_LED_1ER_MIN NOTE_C5 // LED_1ER (tip): C5 (523 Hz) and above - highest notes (C5, D5+)
#define AUDIO_NOTE_LED_3ER_MIN NOTE_A4 // LED_3ER (upper): A4 (440 Hz) to B4 (494 Hz) - high-mid notes (A4, B4)
#define AUDIO_NOTE_LED_3ER_MAX NOTE_B4
#define AUDIO_NOTE_LED_4ER_MIN NOTE_F4 // LED_4ER (middle): F4 (349 Hz) to G4 (392 Hz) - mid notes (G4)
#define AUDIO_NOTE_LED_4ER_MAX NOTE_G4
#define AUDIO_NOTE_LED_5ER_MAX NOTE_E4 // LED_5ER (base): E4 (330 Hz) and below - bass notes (E4)

// Candle Flicker Configuration
#define CANDLE_UPDATE_INTERVAL_MS 130 // Candle flicker update frequency in milliseconds (lower = faster flicker)
#define CANDLE_FLICKER_SCALE 25       // Percentage of normal flicker intensity (25% = gentler for low brightness)

#endif // CONFIG_H_