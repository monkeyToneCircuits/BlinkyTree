/*
 * audio.cpp - Audio system for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 *
 * Clean ATtiny85 implementation without native testing
 */

#include "audio.h"

#if FEATURE_AUDIO_OUTPUT

#include "../Hardware/hardware.h"
#if FEATURE_MICROPHONE_SENSOR
#include "../Sensors/sensors.h"
#endif
#include "../Lighting/lighting.h"
#include <avr/pgmspace.h>
#include <util/delay.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

typedef struct
{
    bool initialized;
    uint8_t song_rotation_index; // Current song in rotation for persistent state
    bool song_currently_playing; // Track if a song is currently being played
    uint32_t song_end_time;      // When the current song finished
    uint32_t cooldown_end_time;  // When the cooldown period ends (song_end_time + SONG_COOLDOWN_DURATION)
} audio_state_t;

static audio_state_t g_audio_state;

// ============================================================================
// SONG CONFIGURATION TABLE - Individual playback settings per song
// ============================================================================

// Default song configurations with individual speed, duty cycle, and transposition settings
// Transposition: negative = lower, positive = higher, 0 = no change
static const song_config_t song_configs[] = {
    [MELODY_NONE] = {50, 100, 0},              // Default fallback
    [MELODY_OH_CHRISTMAS_TREE] = {75, 150, 9}, // 75% duty, 150% speed, +9 semitones - classic gentle Christmas song
    [MELODY_SILENT_NIGHT] = {75, 150, 8},      // 75% duty, slower speed, no transpose - peaceful lullaby feel
    [MELODY_JINGLE_BELLS] = {85, 180, 6},      // 85% duty, faster speed, no transpose - upbeat and energetic
    [MELODY_NOEL] = {80, 140, 8},              // 80% duty, slower speed, no transpose - reverent and traditional
    [MELODY_GLING_KLOECKCHEN] = {85, 170, 8},  // 85% duty, faster speed, no transpose - cheerful children's song
    [MELODY_KOMMET_IHR_HIRTEN] = {80, 160, 4}, // 80% duty, moderate speed, no transpose - traditional pastoral carol
    [MELODY_SCHNEEFLOECKCHEN] = {75, 140, 7},  // 75% duty, slower speed, +7 semitones - gentle German children's song
    [MELODY_TEST_TONE] = {80, 100, 0},         // 80% duty, normal speed, no transpose - test/calibration tone
};

#define SONG_CONFIGS_COUNT (sizeof(song_configs) / sizeof(song_config_t))

// ============================================================================
// SONG COMPILER SELECTION 
//============================================================================

// Forward declaration for blocking bit-banging helper
static void audio_play_tone_blocking_configurable(uint16_t frequency, uint16_t duration_ms, uint8_t duty_cycle_percent);

#if ENABLE_SONG_ROTATION
// List of enabled songs for rotation (compile-time generated)
static const melody_id_t enabled_songs[] = {
#if ENABLE_OH_CHRISTMAS_TREE
    MELODY_OH_CHRISTMAS_TREE,
#endif
#if ENABLE_SILENT_NIGHT
    MELODY_SILENT_NIGHT,
#endif
#if ENABLE_JINGLE_BELLS
    MELODY_JINGLE_BELLS,
#endif
#if ENABLE_NOEL
    MELODY_NOEL,
#endif
#if ENABLE_GLING_KLOECKCHEN
    MELODY_GLING_KLOECKCHEN,
#endif
#if ENABLE_KOMMET_IHR_HIRTEN
    MELODY_KOMMET_IHR_HIRTEN,
#endif
#if ENABLE_SCHNEEFLOECKCHEN
    MELODY_SCHNEEFLOECKCHEN,
#endif
#if ENABLE_TEST_TONE
    MELODY_TEST_TONE,
#endif
};

#define ENABLED_SONGS_COUNT (sizeof(enabled_songs) / sizeof(melody_id_t))
#endif

// ============================================================================
// BUILT-IN MELODIES (stored in PROGMEM)
// ============================================================================

#if ENABLE_OH_CHRISTMAS_TREE

static const audio_note_t PROGMEM oh_christmas_tree_notes[] = {
    // Measure 0 (pickup): "Oh"
    {NOTE_D4, QUARTER_NOTE},
    // Measure 1: "Tannenbaum, oh"
    {NOTE_G4, EIGHTH_NOTE + QUARTER_NOTE},   // dotted eighth
    {NOTE_G4, SIXTEENTH_NOTE},
    {NOTE_G4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    // Measure 2: "Tannenbaum, wie"
    {NOTE_B4, EIGHTH_NOTE + QUARTER_NOTE},   // dotted eighth
    {NOTE_B4, SIXTEENTH_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    // Measure 3: "schön sind Deine"
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_FS4, QUARTER_NOTE},
    // Measure 4: "Blätter. Du"
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_G4, QUARTER_NOTE},
    {0, EIGHTH_NOTE},                        // rest
    {NOTE_D5, EIGHTH_NOTE},
    // Measure 5: "grünst nicht nur zur"
    {NOTE_D5, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    {NOTE_E5, QUARTER_NOTE + EIGHTH_NOTE},   // dotted quarter
    {NOTE_D5, EIGHTH_NOTE},
    // Measure 6: "Sommerszeit nein"
    {NOTE_D5, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_C5, QUARTER_NOTE + EIGHTH_NOTE},   // dotted quarter
    {NOTE_C5, EIGHTH_NOTE},
    // Measure 7: "auch im Winter"
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_D5, QUARTER_NOTE + EIGHTH_NOTE},   // dotted quarter
    {NOTE_C5, EIGHTH_NOTE},
    // Measure 8: "wenn es schneit. Oh"
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_D4, QUARTER_NOTE},
    // Measure 9: "Tannenbaum, oh"
    {NOTE_G4, EIGHTH_NOTE + QUARTER_NOTE},   // dotted eighth
    {NOTE_G4, SIXTEENTH_NOTE},
    {NOTE_G4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    // Measure 10: "Tannenbaum, wie"
    {NOTE_B4, EIGHTH_NOTE + QUARTER_NOTE},   // dotted eighth
    {NOTE_B4, SIXTEENTH_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    // Measure 11: "grün sind Deine"
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_FS4, QUARTER_NOTE},
    // Measure 12: "Blätter."
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_G4, HALF_NOTE}
};

const audio_melody_t melody_oh_christmas_tree = {
    oh_christmas_tree_notes,
    sizeof(oh_christmas_tree_notes) / sizeof(audio_note_t),
    false};
#endif

#if ENABLE_SILENT_NIGHT
// Silent Night
static const audio_note_t PROGMEM silent_night_notes[] = {
    {NOTE_G4, QUARTER_NOTE + EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_G4, QUARTER_NOTE},
    {NOTE_E4, HALF_NOTE + QUARTER_NOTE},
    {NOTE_G4, QUARTER_NOTE + EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_G4, QUARTER_NOTE},
    {NOTE_E4, HALF_NOTE + QUARTER_NOTE},
    {NOTE_D5, HALF_NOTE},
    {NOTE_D5, QUARTER_NOTE},
    {NOTE_B4, HALF_NOTE + QUARTER_NOTE},
    {NOTE_C5, HALF_NOTE},
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_G4, HALF_NOTE + QUARTER_NOTE}};

const audio_melody_t melody_silent_night = {
    silent_night_notes,
    sizeof(silent_night_notes) / sizeof(audio_note_t),
    false};
#endif

#if ENABLE_JINGLE_BELLS

static const audio_note_t PROGMEM jingle_bells_notes[] = {
    // "Jingle bells, jingle bells"
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, HALF_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, HALF_NOTE},
    // "jingle all the way"
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_D5, QUARTER_NOTE},
    {NOTE_G4, QUARTER_NOTE + EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_B4, QUARTER_NOTE + HALF_NOTE}, // dotted half note
    // "Oh! What fun it"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_C5, QUARTER_NOTE + EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    // "is to ride in a"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    // "one horse open sleigh"
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_A4, HALF_NOTE},
    {NOTE_D5, HALF_NOTE}};

const audio_melody_t melody_jingle_bells = {
    jingle_bells_notes,
    sizeof(jingle_bells_notes) / sizeof(audio_note_t),
    false};
#endif

#if ENABLE_NOEL
// The First Noel - Accurate MusicXML transcription
static const audio_note_t PROGMEM noel_notes[] = {
    // "The First Noel"
    {NOTE_E4, EIGHTH_NOTE},
    {NOTE_D4, EIGHTH_NOTE},
    {NOTE_C4, QUARTER_NOTE + EIGHTH_NOTE},
    {NOTE_D4, EIGHTH_NOTE},
    {NOTE_E4, EIGHTH_NOTE},
    {NOTE_F4, EIGHTH_NOTE},
    {NOTE_G4, HALF_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},

    // "The Angel did say"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_G4, HALF_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},

    // "was to certain poor shepherds in"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_G4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_B4, QUARTER_NOTE},

    // "fields as they lay"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_G4, QUARTER_NOTE},
    {NOTE_F4, QUARTER_NOTE},
    {NOTE_E4, HALF_NOTE},
    {NOTE_REST, QUARTER_NOTE}};

const audio_melody_t melody_noel = {
    noel_notes,
    sizeof(noel_notes) / sizeof(audio_note_t),
    false};
#endif

// Kling, Glöckchen, klingelingeling - Based on MusicXML from MuseScore
static const audio_note_t PROGMEM gling_kloeckchen_notes[] = {
    // Measure 1: "Kling, Glöckchen"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_AS4, EIGHTH_NOTE}, // Bb4

    // Measure 2: "klingelingeling"
    {NOTE_C5, 125}, // Sixteenth note duration (EIGHTH_NOTE/2)
    {NOTE_D5, 125},
    {NOTE_C5, 125},
    {NOTE_D5, 125},
    {NOTE_C5, QUARTER_NOTE},

    // Measure 3: "Kling, Glöckchen"
    {NOTE_AS4, QUARTER_NOTE}, // Bb4
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},

    // Measure 4: "kling!" + rest
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_REST, QUARTER_NOTE},

    // Measure 5: "Laßt mich"
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_F4, EIGHTH_NOTE},

    // Measure 6: "hören" + rest
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_REST, EIGHTH_NOTE},

    // Measure 7: "euer helles"
    {NOTE_AS4, EIGHTH_NOTE}, // Bb4
    {NOTE_AS4, EIGHTH_NOTE}, // Bb4
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_G4, EIGHTH_NOTE},

    // Measure 8: "Klingen" + rest
    {NOTE_AS4, QUARTER_NOTE}, // Bb4
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_REST, EIGHTH_NOTE},

    // Measure 9: "kleine"
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE}, // B natural

    // Measure 10: "Glöckchen" + rest
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_REST, EIGHTH_NOTE},

    // Measure 11: "klinget"
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_D5, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE}, // B natural

    // Measure 12: "fein!" + rest
    {NOTE_D5, QUARTER_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_REST, EIGHTH_NOTE}};

const audio_melody_t melody_gling_kloeckchen = {
    gling_kloeckchen_notes,
    sizeof(gling_kloeckchen_notes) / sizeof(audio_note_t),
    false};

#if ENABLE_KOMMET_IHR_HIRTEN
// Kommet, ihr Hirten - Traditional German Christmas carol (transposed up)
static const audio_note_t PROGMEM kommet_ihr_hirten_notes[] = {
    // Measure 1: "Kommet"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    // Measure 2: "ihr Hirten,"
    {NOTE_D5, EIGHTH_NOTE},
    {NOTE_AS4, EIGHTH_NOTE}, // A#4
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    // Measure 3: "ihr Männer"
    {NOTE_D5, EIGHTH_NOTE},
    {NOTE_AS4, EIGHTH_NOTE}, // A#4
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    // Measure 4: "und Frau'n."
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_F4, HALF_NOTE},
    // Measure 5: "Christus,"
    {NOTE_F4, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_F4, EIGHTH_NOTE},
    // Measure 6: "der Herr, ist"
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_F4, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_F4, EIGHTH_NOTE},
    // Measure 7: "heute geboren, den"
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_C4, EIGHTH_NOTE},
    {NOTE_F4, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_F4, EIGHTH_NOTE},
    // Measure 8: "Gott zum Heiland"
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    {NOTE_F4, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_F4, EIGHTH_NOTE},
    // Measure 9: "euch hat erkoren -"
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_C4, EIGHTH_NOTE},
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    // Measure 10: "Fürchte euch nicht."
    {NOTE_G4, EIGHTH_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_F4, HALF_NOTE}
};

const audio_melody_t melody_kommet_ihr_hirten = {
    kommet_ihr_hirten_notes,
    sizeof(kommet_ihr_hirten_notes) / sizeof(audio_note_t),
    false};
#endif

#if ENABLE_SCHNEEFLOECKCHEN
// "Schneeflöckchen, Weißröckchen" - German Christmas children's song
// Key: C major
// Time: 3/4
// Traditional gentle melody
static const audio_note_t PROGMEM schneefloeckchen_notes[] = {
    // Measure 0 (pickup): "Schnee-"
    {NOTE_B4, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    // Measure 1: "flöckchen,"
    {NOTE_D5, QUARTER_NOTE},
    {NOTE_D5, QUARTER_NOTE},
    {NOTE_E5, QUARTER_NOTE},
    // Measure 2: "Weißröckchen da"
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    {NOTE_A4, EIGHTH_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    // Measure 3: "kommst du geschneit,"
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_D5, QUARTER_NOTE},
    // Measure 4: "du"
    {NOTE_B4, HALF_NOTE},
    {NOTE_B4, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    // Measure 5: "kommst aus den Wolken"
    {NOTE_D5, QUARTER_NOTE},
    {NOTE_D5, QUARTER_NOTE},
    {NOTE_G5, QUARTER_NOTE},
    // Measure 6: "Wolken dein"
    {NOTE_FS5, QUARTER_NOTE},
    {NOTE_E5, QUARTER_NOTE},
    {NOTE_D5, EIGHTH_NOTE},
    {NOTE_C5, EIGHTH_NOTE},
    // Measure 7: "Weg ist so"
    {NOTE_B4, QUARTER_NOTE},
    {NOTE_C5, QUARTER_NOTE},
    {NOTE_A4, QUARTER_NOTE},
    // Measure 8: "weit."
    {NOTE_G4, HALF_NOTE}
};

const audio_melody_t melody_schneefloeckchen = {
    schneefloeckchen_notes,
    sizeof(schneefloeckchen_notes) / sizeof(audio_note_t),
    false};
#endif

#if ENABLE_TEST_TONE
// Test tone - Single 5-second note for frequency calibration
// Change NOTE_A4 (440 Hz) to any frequency you want to test
static const audio_note_t PROGMEM test_tone_notes[] = {
    {NOTE_FS6, 7000}  // 5 second A4 (440 Hz) - modify frequency as needed
};

const audio_melody_t melody_test_tone = {
    test_tone_notes,
    sizeof(test_tone_notes) / sizeof(audio_note_t),
    false};
#endif



// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

static const audio_melody_t *get_melody_data(melody_id_t melody_id)
{
    switch (melody_id)
    {
#if ENABLE_OH_CHRISTMAS_TREE
    case MELODY_OH_CHRISTMAS_TREE:
        return &melody_oh_christmas_tree;
#endif
#if ENABLE_SILENT_NIGHT
    case MELODY_SILENT_NIGHT:
        return &melody_silent_night;
#endif
#if ENABLE_JINGLE_BELLS
    case MELODY_JINGLE_BELLS:
        return &melody_jingle_bells;
#endif
#if ENABLE_NOEL
    case MELODY_NOEL:
        return &melody_noel;
#endif
    case MELODY_GLING_KLOECKCHEN:
        return &melody_gling_kloeckchen;
#if ENABLE_KOMMET_IHR_HIRTEN
    case MELODY_KOMMET_IHR_HIRTEN:
        return &melody_kommet_ihr_hirten;
#endif
#if ENABLE_SCHNEEFLOECKCHEN
    case MELODY_SCHNEEFLOECKCHEN:
        return &melody_schneefloeckchen;
#endif
#if ENABLE_TEST_TONE
    case MELODY_TEST_TONE:
        return &melody_test_tone;
#endif
    default:
        return NULL;
    }
}

// Transpose a frequency by semitones (positive = up, negative = down)
// Uses fixed-point arithmetic to avoid floating point operations
// Formula: new_freq = old_freq * 2^(semitones/12)
static uint16_t transpose_frequency(uint16_t frequency, int8_t semitones)
{
    if (semitones == 0 || frequency == 0)
    {
        return frequency; // No transposition or rest note
    }

    // Note frequency lookup table - uses actual frequencies from audio.h
    // This ensures transposition uses the fine-tuned buzzer frequencies
    static const uint16_t PROGMEM note_frequencies[] = {
        NOTE_G3,  // 0
        NOTE_GS3, // 1
        NOTE_A3,  // 2
        NOTE_AS3, // 3
        NOTE_B3,  // 4
        NOTE_C4,  // 5
        NOTE_CS4, // 6
        NOTE_D4,  // 7
        NOTE_DS4, // 8
        NOTE_E4,  // 9
        NOTE_F4,  // 10
        NOTE_FS4, // 11
        NOTE_G4,  // 12
        NOTE_GS4, // 13
        NOTE_A4,  // 14
        NOTE_AS4, // 15
        NOTE_B4,  // 16
        NOTE_C5,  // 17
        NOTE_CS5, // 18
        NOTE_D5,  // 19
        NOTE_DS5, // 20
        NOTE_E5,  // 21
        NOTE_F5,  // 22
        NOTE_FS5, // 23
        NOTE_G5,  // 24
        NOTE_GS5, // 25
        NOTE_A5,  // 26
        NOTE_AS5, // 27
        NOTE_B5,  // 28
        NOTE_C6,  // 29
        NOTE_CS6, // 30
        NOTE_D6,  // 31
        NOTE_DS6, // 32
        NOTE_E6   // 33
    };
    
    const uint8_t note_count = sizeof(note_frequencies) / sizeof(uint16_t);

    // Find closest matching note in the table
    int8_t note_index = -1;
    uint16_t min_diff = 65535;
    
    for (uint8_t i = 0; i < note_count; i++)
    {
        uint16_t table_freq = pgm_read_word(&note_frequencies[i]);
        uint16_t diff = (frequency > table_freq) ? (frequency - table_freq) : (table_freq - frequency);
        
        if (diff < min_diff)
        {
            min_diff = diff;
            note_index = i;
        }
    }

    // If no match found or out of bounds, use ratio-based fallback
    if (note_index < 0)
    {
        return frequency;
    }

    // Apply transposition by shifting in the note table
    int8_t transposed_index = note_index + semitones;
    
    // Clamp to valid range
    if (transposed_index < 0)
        transposed_index = 0;
    if (transposed_index >= note_count)
        transposed_index = note_count - 1;

    return pgm_read_word(&note_frequencies[transposed_index]);
}

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

bool audio_init(void)
{
    memset(&g_audio_state, 0, sizeof(g_audio_state));

    // Load persistent song rotation index
#if ENABLE_SONG_ROTATION
    g_audio_state.song_rotation_index = hardware_eeprom_read_byte(EEPROM_ADDR_SONG_ROTATION_INDEX);

    // Validate the index (in case EEPROM is uninitialized or corrupted)
    if (ENABLED_SONGS_COUNT > 0 && g_audio_state.song_rotation_index >= ENABLED_SONGS_COUNT)
    {
        g_audio_state.song_rotation_index = 0;
        hardware_eeprom_write_byte(EEPROM_ADDR_SONG_ROTATION_INDEX, 0);
    }
#endif

    // Initialize song tracking state
    g_audio_state.song_currently_playing = false;
    g_audio_state.song_end_time = 0;
    g_audio_state.cooldown_end_time = 0;

    g_audio_state.initialized = true;
    return true;
}

// Non-blocking functions removed - using blocking-only approach

void audio_play_melody_blocking(melody_id_t melody_id, uint8_t duty_cycle_percent, uint16_t speed_percent, int8_t transpose_semitones)
{
    const audio_melody_t *melody_data = get_melody_data(melody_id);
    if (!melody_data || melody_data->note_count == 0)
    {
        return;
    }

    // Mark song as playing
    g_audio_state.song_currently_playing = true;

    // Validate parameters
    if (duty_cycle_percent > 100)
        duty_cycle_percent = 100;
    if (duty_cycle_percent < 10)
        duty_cycle_percent = 10; // Minimum for audible sound
    if (speed_percent < 25)
        speed_percent = 25; // Minimum 25% speed
    if (speed_percent > 10000)
        speed_percent = 10000; // Maximum 10000% speed (100x faster!)

    // Clamp transposition to valid range
    if (transpose_semitones < -12)
        transpose_semitones = -12;
    if (transpose_semitones > 12)
        transpose_semitones = 12;

    // Initialize audio hardware (ensure buzzer pin is output with maximum drive)
    DDRB |= (1 << BUZZER_PIN);   // Set as output
    PORTB &= ~(1 << BUZZER_PIN); // Start LOW for clean signal

    // Play each note using bit-banging with configurable parameters
    for (uint8_t i = 0; i < melody_data->note_count; i++)
    {
        audio_note_t note;
        memcpy_P(&note, &melody_data->notes[i], sizeof(audio_note_t));

        // Apply transposition to frequency (0 = rest, unchanged)
        uint16_t transposed_freq = transpose_frequency(note.frequency, transpose_semitones);

        // Apply speed scaling to duration
        uint16_t scaled_duration = ((uint32_t)note.duration * 100) / speed_percent;

        // Trigger audio-reactive lighting based on ORIGINAL note frequency (not transposed)
        // This keeps the light show matched to the musical pitch relationships as written
        lighting_audio_reactive_note(note.frequency);

        // Use configurable bit-banging method
        audio_play_tone_blocking_configurable(transposed_freq, scaled_duration, duty_cycle_percent);

        // Turn off audio-reactive LEDs during note gaps
        // Add small gap between notes to distinguish identical consecutive notes
        // Skip gap after the last note
        if (i < melody_data->note_count - 1)
        {
            lighting_audio_reactive_off(); // Turn off LEDs during gaps
            uint16_t gap_duration = ((uint32_t)AUDIO_NOTE_GAP_MS * 100) / speed_percent;
            audio_play_tone_blocking_configurable(0, gap_duration, duty_cycle_percent); // 0 frequency = silence
        }
    }

    // Turn off all audio-reactive LEDs when song ends
    lighting_audio_reactive_off();

    // Restore microphone configuration after LED_3ER audio mode
#if FEATURE_MICROPHONE_SENSOR
    hardware_microphone_init(); // Reinitialize microphone (restores PB3 pin config)
#endif

    // Mark song as finished and set cooldown timing
    uint32_t current_time = hardware_get_millis();
    g_audio_state.song_currently_playing = false;
    g_audio_state.song_end_time = current_time;
    g_audio_state.cooldown_end_time = current_time + SONG_COOLDOWN_DURATION;

    // Force sensor recalibration after audio playback to handle shared pin charge buildup
#if FEATURE_MICROPHONE_SENSOR
    sensors_force_recalibration();
#endif
}

// Configurable bit-banging function with duty cycle parameter
static void audio_play_tone_blocking_configurable(uint16_t frequency, uint16_t duration_ms, uint8_t duty_cycle_percent)
{
    if (frequency == 0)
    {
        // Rest (silence)
        for (uint16_t i = 0; i < duration_ms; i++)
        {
            _delay_ms(1);
        }
        return;
    }

    // Calculate timing based on frequency
    uint32_t period_us = 1000000UL / frequency;

    // Calculate high and low times based on duty cycle percentage
    uint32_t high_time_us = (period_us * duty_cycle_percent) / 100;
    uint32_t low_time_us = period_us - high_time_us;

    const uint8_t delay_step_us = 5;

    // Calculate delay loops for high and low phases
    uint32_t high_loops = high_time_us / delay_step_us;
    uint32_t low_loops = low_time_us / delay_step_us;
    uint8_t high_remainder = high_time_us % delay_step_us;
    uint8_t low_remainder = low_time_us % delay_step_us;

    uint32_t total_cycles = (uint32_t)frequency * duration_ms / 1000UL;

    for (uint32_t i = 0; i < total_cycles; i++)
    {
        // High phase (configurable duty cycle)
        PORTB |= (1 << BUZZER_PIN);
        for (uint32_t d = 0; d < high_loops; d++)
        {
            _delay_us(delay_step_us);
        }
        for (uint8_t r = 0; r < high_remainder; r++)
        {
            __asm__ __volatile__("nop\nnop\nnop\nnop");
        }

        // Low phase (remainder of period)
        PORTB &= ~(1 << BUZZER_PIN);
        for (uint32_t d = 0; d < low_loops; d++)
        {
            _delay_us(delay_step_us);
        }
        for (uint8_t r = 0; r < low_remainder; r++)
        {
            __asm__ __volatile__("nop\nnop\nnop\nnop");
        }
    }
    PORTB &= ~(1 << BUZZER_PIN); // Ensure buzzer is off
}

void audio_play_next_melody(void)
{
#if ENABLE_SONG_ROTATION
    if (ENABLED_SONGS_COUNT == 0)
    {
        return; // No songs enabled
    }

#if SONG_ROTATION_MODE == 0
    // Random mode
    g_audio_state.song_rotation_index = hardware_get_millis() % ENABLED_SONGS_COUNT;
#else
    // Sequential mode
    g_audio_state.song_rotation_index = (g_audio_state.song_rotation_index + 1) % ENABLED_SONGS_COUNT;
#endif

    // Save the new rotation index to EEPROM for persistence across resets
    hardware_eeprom_write_byte(EEPROM_ADDR_SONG_ROTATION_INDEX, g_audio_state.song_rotation_index);

    melody_id_t next_melody = enabled_songs[g_audio_state.song_rotation_index];
    // Use individual song configuration for optimal playback
    song_config_t config = audio_get_song_config(next_melody);
    audio_play_melody_blocking(next_melody, config.duty_cycle_percent, config.speed_percent, config.transpose_semitones);
#else
    // Rotation disabled, play default song with its individual configuration
    song_config_t config = audio_get_song_config(MELODY_OH_CHRISTMAS_TREE);
    audio_play_melody_blocking(MELODY_OH_CHRISTMAS_TREE, config.duty_cycle_percent, config.speed_percent, config.transpose_semitones);
#endif
}

void audio_play_current_melody(void)
{
#if ENABLE_SONG_ROTATION
    if (ENABLED_SONGS_COUNT == 0)
    {
        return; // No songs enabled
    }

    // Validate current index
    if (g_audio_state.song_rotation_index >= ENABLED_SONGS_COUNT)
    {
        g_audio_state.song_rotation_index = 0;
    }

    melody_id_t current_melody = enabled_songs[g_audio_state.song_rotation_index];
    // Use individual song configuration for optimal playback
    song_config_t config = audio_get_song_config(current_melody);
    audio_play_melody_blocking(current_melody, config.duty_cycle_percent, config.speed_percent, config.transpose_semitones);
#else
    // Rotation disabled, play default song with its individual configuration
    song_config_t config = audio_get_song_config(MELODY_OH_CHRISTMAS_TREE);
    audio_play_melody_blocking(MELODY_OH_CHRISTMAS_TREE, config.duty_cycle_percent, config.speed_percent, config.transpose_semitones);
#endif
}

song_config_t audio_get_song_config(melody_id_t melody_id)
{
    // Validate melody ID and return appropriate config
    if (melody_id < SONG_CONFIGS_COUNT)
    {
        return song_configs[melody_id];
    }

    // Return default config for invalid melody ID
    return song_configs[MELODY_NONE];
}

bool audio_is_song_playing(void)
{
    return g_audio_state.song_currently_playing;
}

bool audio_is_cooldown_expired(void)
{
    uint32_t current_time = hardware_get_millis();
    return (current_time >= g_audio_state.cooldown_end_time);
}

#endif // FEATURE_AUDIO_OUTPUT