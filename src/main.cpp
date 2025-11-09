/*
 * main.cpp
 * ATtiny85 BlinkyTree Christmas Tree v2.0
 * Feature-configurable implementation
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#include "../config/config.h"
#include "../lib/Hardware/hardware.h"
#include "../lib/Lighting/lighting.h"
#if FEATURE_AUDIO_OUTPUT
#include "../lib/Audio/audio.h"
#endif
#if FEATURE_MICROPHONE_SENSOR
#include "../lib/Sensors/sensors.h"
#endif
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    // Initialize hardware systems
    hardware_init();

    // Initialize lighting system - including Lighting mode (e.g. Candle Effect)
    lighting_init();
    
#if FEATURE_MICROPHONE_SENSOR
    // Initialize sensor system for breath detection
    sensors_init();
#endif

#if FEATURE_AUDIO_OUTPUT
    // Initialize audio system
    audio_init();
#endif

    // Play startup melody using current song in rotation
#if PLAY_STARTUP_MELODY
    audio_play_next_melody();
#endif

    while (1)
    {

        hardware_update();
        lighting_update();

        // Only update sensor system if no song is playing and cooldown has expired
        if (!audio_is_song_playing() && audio_is_cooldown_expired())
        {
            sensors_update(); // Re-enabled for Phase 2 testing
        }

        // Small delay to prevent excessive CPU usage
        _delay_us(10);
    }
}