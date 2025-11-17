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
// NOTE DEFINITIONS
// ============================================================================

// Musical note frequencies (in Hz) - Fine-tuned for ATtiny85 buzzer
// Lower octave notes
#define NOTE_G3 128
#define NOTE_GS3 136
#define NOTE_A3 144
#define NOTE_AS3 153
#define NOTE_B3 162

// Fourth octave notes
#define NOTE_C4 172
#define NOTE_CS4 182
#define NOTE_D4 192
#define NOTE_DS4 204
#define NOTE_E4 216
#define NOTE_F4 229
#define NOTE_FS4 243
#define NOTE_G4 257 
#define NOTE_GS4 273
#define NOTE_A4 290 
#define NOTE_AS4 307
#define NOTE_B4 326
#define NOTE_C5 344
#define NOTE_CS5 365
#define NOTE_D5 386
#define NOTE_DS5 408
#define NOTE_E5 433
#define NOTE_F5 459
#define NOTE_FS5 486
#define NOTE_G5 515
#define NOTE_GS5 546
#define NOTE_A5 580
#define NOTE_AS5 614
#define NOTE_B5 652
#define NOTE_C6 690
#define NOTE_CS6 730
#define NOTE_D6 773
#define NOTE_DS6 818
#define NOTE_E6 866
#define NOTE_F6 918
#define NOTE_FS6 972
#define NOTE_G6 1030
#define NOTE_GS6 1092
#define NOTE_A6 1160
#define NOTE_AS6 1228

// Special notes
#define NOTE_REST 0

// Note durations (in milliseconds)
#define SIXTEENTH_NOTE 125
#define EIGHTH_NOTE 250
#define QUARTER_NOTE 500
#define HALF_NOTE 1000
#define WHOLE_NOTE 2000

// ============================================================================
// DATA STRUCTURES
// ============================================================================

// Note structure for melodies
typedef struct
{
    uint16_t frequency; // Frequency in Hz (0 = rest)
    uint16_t duration;  // Duration in milliseconds
} audio_note_t;

// Song configuration structure for individual playback settings
typedef struct
{
    uint8_t duty_cycle_percent;  // Duty cycle percentage (10-100)
    uint16_t speed_percent;      // Speed percentage (25-1000, 100 = normal speed)
    int8_t transpose_semitones;  // Transpose by semitones (-12 to +12, 0 = no transposition)
} song_config_t;

// ============================================================================
// MELODY ENUMERATION (will be included from generated file)
// ============================================================================

// This will be provided by audio_songs_generated.h
#include "audio_songs_generated.h"

// ============================================================================
// AUDIO SYSTEM FUNCTIONS
// ============================================================================

// Core blocking audio functions
bool audio_init(void);
void audio_play_melody_blocking(melody_id_t melody_id, uint8_t duty_cycle_percent, uint16_t speed_percent, int8_t transpose_semitones);
void audio_play_next_melody(void);
void audio_play_current_melody(void);
song_config_t audio_get_song_config(melody_id_t melody_id);

// Song state tracking functions
bool audio_is_song_playing(void);
bool audio_is_cooldown_expired(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H_
