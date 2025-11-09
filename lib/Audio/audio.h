/*
 * audio.h - Audio system for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "../../config/config.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ============================================================================
// AUDIO CONFIGURATION
// ============================================================================

// Audio system parameters
#define AUDIO_MAX_FREQUENCY 4000 // Maximum frequency in Hz
#define AUDIO_MIN_FREQUENCY 100  // Minimum frequency in Hz
#define AUDIO_DEFAULT_VOLUME 128 // Default volume (0-255)

    // ============================================================================
    // MELODY DEFINITIONS
    // ============================================================================

    typedef enum
    {
        MELODY_NONE = 0,
        MELODY_OH_CHRISTMAS_TREE,
        MELODY_SILENT_NIGHT,
        MELODY_JINGLE_BELLS,
        MELODY_ZELDA_THEME,
        MELODY_NOEL,
        MELODY_IMPERIAL_MARCH,
        MELODY_GLING_KLOECKCHEN,
        MELODY_COUNT
    } melody_id_t;

    // Note structure for melodies
    typedef struct
    {
        uint16_t frequency; // Frequency in Hz (0 = rest)
        uint16_t duration;  // Duration in milliseconds
    } audio_note_t;

    // Melody structure
    typedef struct
    {
        const audio_note_t *notes;
        uint8_t note_count;
        bool loop;
    } audio_melody_t;

    // Song configuration structure for individual playback settings
    typedef struct
    {
        uint8_t duty_cycle_percent; // Duty cycle percentage (10-100)
        uint16_t speed_percent;     // Speed percentage (25-1000, 100 = normal speed)
    } song_config_t;

    // ============================================================================
    // AUDIO SYSTEM FUNCTIONS
    // ============================================================================

    // Core blocking audio functions
    bool audio_init(void);                                                                                      // Initialize audio system and load persistent state
    void audio_play_melody_blocking(melody_id_t melody_id, uint8_t duty_cycle_percent, uint16_t speed_percent); // Play melody with blocking execution
    void audio_play_next_melody(void);                                                                          // Play next song in rotation (blocking)
    void audio_play_current_melody(void);                                                                       // Play current song in rotation (blocking)
    song_config_t audio_get_song_config(melody_id_t melody_id);                                                 // Get individual song configuration

    // Song state tracking functions
    bool audio_is_song_playing(void);     // Check if a song is currently playing
    bool audio_is_cooldown_expired(void); // Check if post-song cooldown has expired

    // ============================================================================
    // BUILT-IN MELODIES AND EFFECTS
    // ============================================================================

    // Christmas melodies
    extern const audio_melody_t melody_oh_christmas_tree;
    extern const audio_melody_t melody_silent_night;
    extern const audio_melody_t melody_jingle_bells;
    extern const audio_melody_t melody_noel;

    // Theme songs
    extern const audio_melody_t melody_zelda_theme;
    extern const audio_melody_t melody_imperial_march;
    extern const audio_melody_t melody_gling_kloeckchen;

    // Audio effects removed - focusing on core melody playback only

// Musical note frequencies (in Hz)
// Lower octave notes
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247

// Fourth octave notes
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988

// Special notes
#define NOTE_REST 0

// Note durations (in milliseconds)
#define QUARTER_NOTE 500
#define EIGHTH_NOTE 250
#define HALF_NOTE 1000
#define WHOLE_NOTE 2000

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H_