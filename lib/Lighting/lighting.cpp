/*
 * lighting.cpp - LED lighting effects for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 *
 * Feature-configurable LED effects system
 * Uses config.h feature flags to control compilation
 */

#include "lighting.h"
#include <string.h>
#include "../../config/config.h"
#include "../Sensors/sensors.h"
#include "../Hardware/hardware.h"
#if FEATURE_AUDIO_OUTPUT
#include "../Audio/audio.h"
#endif

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

typedef struct
{
    bool initialized;
    lighting_effect_t current_effect;
    uint32_t last_update_time;
    uint8_t effect_speed;
    uint8_t led_states[LED_COUNT_MAX];
    uint16_t effect_counter;
    uint8_t candle_intensity_boost; // 0-100 additional intensity from microphone
} lighting_state_t;

static lighting_state_t g_lighting_state;

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// Batch LED update removed - using direct hardware_led_set calls for simplicity

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

bool lighting_init(void)
{
    // Initialize lighting state to default values
    g_lighting_state.current_effect = LIGHTING_EFFECT_NONE;
    g_lighting_state.effect_counter = 0;
    g_lighting_state.last_update_time = 0;
    g_lighting_state.candle_intensity_boost = 0; // Initialize microphone boost

    // Initialize all LED states to off
    for (uint8_t i = 0; i < LED_COUNT_MAX; i++)
    {
        g_lighting_state.led_states[i] = 0;
    }

#ifdef FEATURE_CANDLE_EFFECT
    lighting_set_effect(LIGHTING_EFFECT_CANDLE);
#elif FEATURE_BREATHING_EFFECT
    lighting_set_effect(LIGHTING_EFFECT_BREATHING);
#elif FEATURE_CANDLE_EFFECT_KRANZ
    lighting_set_effect(LIGHTING_EFFECT_CANDLE_KRANZ);
#endif



    return true;
}

// Helper function for non-blocking delay with PWM updates
static void startup_delay_ms(uint16_t ms)
{
    uint32_t start = hardware_get_millis();
    while ((hardware_get_millis() - start) < ms)
    {
        hardware_update();  // Keep PWM running during delay
        _delay_us(10);      // Small delay to prevent excessive CPU usage
    }
}

void lighting_startup_animation(void)
{
#if ENABLE_STARTUP_ANIMATION
    // Bottom to top LED build-up sequence with final flash
    // Total duration: ~1000ms
    
    uint8_t buildup_brightness = STARTUP_BUILDUP_BRIGHTNESS;  // 1/3 brightness
    
    // Turn off all LEDs first
    hardware_led_all_off();
    startup_delay_ms(50);
    
    // Step 1: Bottom ring (LED_5ER) - 150ms
    hardware_led_set(LED_5ER_RING, buildup_brightness);
    startup_delay_ms(STARTUP_STEP_DELAY_MS);
    
    // Step 2: Middle ring (LED_4ER) - 300ms total
    hardware_led_set(LED_4ER_RING, buildup_brightness);
    startup_delay_ms(STARTUP_STEP_DELAY_MS);
    
    // Step 3: Upper ring (LED_3ER) - 450ms total
    hardware_led_set(LED_3ER_RING, buildup_brightness);
    startup_delay_ms(STARTUP_STEP_DELAY_MS);
    
    // Step 4: Top LED (LED_1ER) - 600ms total
    hardware_led_set(LED_1ER_RING, buildup_brightness);
    startup_delay_ms(STARTUP_STEP_DELAY_MS);
    
    // Step 5: Turn off all LEDs for dramatic pause
    hardware_led_all_off();
    startup_delay_ms(STARTUP_DARK_PAUSE_MS);
    
    // Animation complete - LEDs will be controlled by normal effects
    // Total time: ~1300ms
#endif
}

void lighting_update(void)
{
#if FEATURE_AUDIO_OUTPUT
    // Skip normal lighting effects when a song is playing to allow audio-reactive lighting
    if (audio_is_song_playing())
    {
        return;
    }
#endif

    uint32_t current_time = hardware_get_millis();

    g_lighting_state.last_update_time = current_time;

    // Handle current effect
    if (g_lighting_state.current_effect == LIGHTING_EFFECT_NONE)
    {
        return;
    }

    g_lighting_state.effect_counter++;

    switch (g_lighting_state.current_effect)
    {

    case LIGHTING_EFFECT_BREATHING:
    {
        // ISR-based breathing timing for rock-solid stability
        static uint8_t brightness_counter = 0;
        static uint8_t brightness_direction = 1; // 1 = up, 0 = down
        static uint32_t last_update_time = 0;

        // Update every 100ms using stable ISR timing
        if (current_time - last_update_time >= 100)
        {
            last_update_time = current_time;

            // Simple brightness stepping: 50-255 range = 205 levels
            // 20 steps per direction, so step size = 205/20 ≈ 10
            if (brightness_direction == 1)
            {
                // Breathing in
                brightness_counter += 10;
                if (brightness_counter >= 205) // Reached max (50+205=255)
                {
                    brightness_counter = 205;
                    brightness_direction = 0; // Start breathing out
                }
            }
            else
            {
                // Breathing out
                if (brightness_counter >= 10)
                {
                    brightness_counter -= 10;
                }
                else
                {
                    brightness_counter = 0;
                    brightness_direction = 1; // Start breathing in
                }
            }
        }

        uint8_t current_brightness = 50 + brightness_counter;

        for (uint8_t i = 0; i < LED_COUNT_MAX; i++)
        {
            hardware_led_set((led_id_t)i, current_brightness);
        }
    }
    break;

    case LIGHTING_EFFECT_CANDLE:
    {
        // Realistic candle flame physics - different behavior per ring level (ATtiny85+)
        static uint8_t random_seed = 42;      // Simple random number seed
        static uint32_t last_update_time = 0; // Timing control like ATtiny13

        // Update candle flicker at configurable interval
        if (current_time - last_update_time >= CANDLE_FLICKER_SPEED)
        {
            last_update_time = current_time;

            // Update random seed for this frame
            random_seed = (random_seed * 13 + 37) & 0xFF;

            // Common flicker components (shared air currents, etc.)
            uint8_t global_slow_wave = (g_lighting_state.effect_counter / 8) & 0x7F; // 0-127
            if (global_slow_wave > 63)
                global_slow_wave = 127 - global_slow_wave; // Triangle wave

            // Global wind effect (affects all rings) - increased frequency
            bool wind_gust = ((random_seed & 0x1F) == 0x1F); // ~3% chance (was ~1.5%)

            // === LED_1ER_RING (TIP) - Maximum flicker, most dramatic ===
            {
                uint8_t tip_seed = random_seed + 7;
                uint8_t fast_flicker = (tip_seed * 5) & 0x3F;                           // 0-63 range (most active)
                uint8_t medium_wave = (g_lighting_state.effect_counter / 1 + 3) & 0x1F; // Fast medium
                if (medium_wave > 15)
                    medium_wave = 31 - medium_wave;

                int16_t tip_brightness = (LED_BRIGHTNESS_DEFAULT * CANDLE_TIP_BRIGHTNESS_PCT) / 100; // Configurable base brightness
                tip_brightness += ((fast_flicker - 31) * CANDLE_FLICKER_INTENSITY) / 100;                // Scaled dramatic variation
                tip_brightness += ((medium_wave - 7) * 3 * CANDLE_FLICKER_INTENSITY) / 100;              // Scaled medium variation
                tip_brightness += ((global_slow_wave - 31) * CANDLE_FLICKER_INTENSITY) / 200;            // Scaled slow variation

                if (wind_gust)
                    tip_brightness -= (30 * CANDLE_FLICKER_INTENSITY) / 100; // Scaled wind effect

                // Apply microphone intensity boost - increased for better visibility (0-100 maps to 0-80 extra brightness)
                tip_brightness += (g_lighting_state.candle_intensity_boost * 4 / 5);

                // Clamp tip range: LED_BRIGHTNESS_MIN to configurable max (extended for microphone boost)
                if (tip_brightness < LED_BRIGHTNESS_MIN)
                    tip_brightness = LED_BRIGHTNESS_MIN;
                if (tip_brightness > 180)
                    tip_brightness = 180;

                hardware_led_set(LED_1ER_RING, (uint8_t)tip_brightness);
            }

            // === LED_3ER_RING (UPPER) - High flicker, second most active ===
            {
                uint8_t upper_seed = random_seed + 13;
                uint8_t fast_flicker = (upper_seed * 3) & 0x1F; // 0-31 range (high activity)
                uint8_t medium_wave = (g_lighting_state.effect_counter / 2 + 7) & 0x1F;
                if (medium_wave > 15)
                    medium_wave = 31 - medium_wave;

                int16_t upper_brightness = (LED_BRIGHTNESS_DEFAULT * CANDLE_UPPER_BRIGHTNESS_PCT) / 100; // Configurable base brightness
                upper_brightness += ((fast_flicker - 15) * CANDLE_FLICKER_INTENSITY) / 100;                  // Scaled variation
                upper_brightness += ((medium_wave - 7) * 2 * CANDLE_FLICKER_INTENSITY) / 100;                // Scaled medium variation
                upper_brightness += ((global_slow_wave - 31) * CANDLE_FLICKER_INTENSITY) / 300;              // Scaled slow variation

                if (wind_gust)
                    upper_brightness -= (25 * CANDLE_FLICKER_INTENSITY) / 100; // Scaled wind effect

                // Apply microphone intensity boost - increased (0-100 maps to 0-70 extra brightness)
                upper_brightness += (g_lighting_state.candle_intensity_boost * 7 / 10);

                // Clamp upper range: LED_BRIGHTNESS_MIN to configurable max (extended for microphone boost)
                if (upper_brightness < LED_BRIGHTNESS_MIN)
                    upper_brightness = LED_BRIGHTNESS_MIN;
                if (upper_brightness > 140)
                    upper_brightness = 140;

                hardware_led_set(LED_3ER_RING, (uint8_t)upper_brightness);
            }

            // === LED_4ER_RING (MIDDLE) - Medium flicker, more stable ===
            {
                uint8_t middle_seed = random_seed + 19;
                uint8_t fast_flicker = (middle_seed * 2) & 0x0F; // 0-15 range (moderate activity)
                uint8_t medium_wave = (g_lighting_state.effect_counter / 3 + 11) & 0x0F;
                if (medium_wave > 7)
                    medium_wave = 15 - medium_wave;

                int16_t middle_brightness = (LED_BRIGHTNESS_DEFAULT * CANDLE_MIDDLE_BRIGHTNESS_PCT) / 100; // Configurable base brightness
                middle_brightness += ((fast_flicker - 7) * CANDLE_FLICKER_INTENSITY) / 100;                    // Scaled variation
                middle_brightness += ((medium_wave - 3) * 2 * CANDLE_FLICKER_INTENSITY) / 100;                 // Scaled medium variation
                middle_brightness += ((global_slow_wave - 31) * CANDLE_FLICKER_INTENSITY) / 400;               // Scaled slow variation

                if (wind_gust)
                    middle_brightness -= (20 * CANDLE_FLICKER_INTENSITY) / 100; // Scaled wind effect

                // Apply microphone intensity boost - increased (0-100 maps to 0-60 extra brightness)
                middle_brightness += (g_lighting_state.candle_intensity_boost * 3 / 5);

                // Clamp middle range: LED_BRIGHTNESS_MIN to configurable max (extended for microphone boost)
                if (middle_brightness < LED_BRIGHTNESS_MIN)
                    middle_brightness = LED_BRIGHTNESS_MIN;
                if (middle_brightness > 100)
                    middle_brightness = 100;

                hardware_led_set(LED_4ER_RING, (uint8_t)middle_brightness);
            }

            // === LED_5ER_RING (BASE) - Gentle glow, most stable ===
            {
                uint8_t base_seed = random_seed + 23;
                uint8_t gentle_flicker = (base_seed) & 0x07; // 0-7 range (minimal activity)
                uint8_t slow_variation = (g_lighting_state.effect_counter / 6) & 0x07;
                if (slow_variation > 3)
                    slow_variation = 7 - slow_variation;

                int16_t base_brightness = (LED_BRIGHTNESS_DEFAULT * CANDLE_BASE_BRIGHTNESS_PCT) / 100; // Configurable base brightness
                base_brightness += ((gentle_flicker - 3) * 2 * CANDLE_FLICKER_INTENSITY) / 100;            // Scaled gentle variation
                base_brightness += ((slow_variation - 1) * 2 * CANDLE_FLICKER_INTENSITY) / 100;            // Scaled very slow variation
                base_brightness += ((global_slow_wave - 31) * CANDLE_FLICKER_INTENSITY) / 600;             // Scaled minimal slow variation

                if (wind_gust)
                    base_brightness -= (12 * CANDLE_FLICKER_INTENSITY) / 100; // Scaled wind effect

                // Apply microphone intensity boost - increased (0-100 maps to 0-40 extra brightness)
                base_brightness += (g_lighting_state.candle_intensity_boost * 2 / 5);

                // Clamp base range: LED_BRIGHTNESS_MIN to configurable max (extended for microphone boost)
                if (base_brightness < LED_BRIGHTNESS_MIN)
                    base_brightness = LED_BRIGHTNESS_MIN;
                if (base_brightness > 70)
                    base_brightness = 70;

                hardware_led_set(LED_5ER_RING, (uint8_t)base_brightness);
            }
        }
    }
    break;

    case LIGHTING_EFFECT_ADC_TEST:
    {
        // Read filtered ADC value from microphone sensor for stability
        uint16_t adc_value = hardware_microphone_read(); // 0-1023 (10-bit ADC)

        // Turn off all LEDs first
        hardware_led_set(LED_1ER_RING, 0);
        hardware_led_set(LED_3ER_RING, 0);
        hardware_led_set(LED_4ER_RING, 0);
        hardware_led_set(LED_5ER_RING, 0);

        // Progressive ADC-to-LED mapping for 1.1V reference
        // 600mV = ~560 ADC counts (600/1100*1023)
        // Base ring shows any signal above noise floor
        if (adc_value > 50) // Noise floor threshold (~50mV)
        {
            // Scale ADC (50-560) to brightness (5-255) for base ring
            uint16_t scaled = (adc_value > 560) ? 560 : adc_value;
            uint8_t base_brightness = 5 + ((scaled - 50) * 250) / 510;
            hardware_led_set(LED_5ER_RING, base_brightness);
        }

        // Progressive rings for higher signals
        if (adc_value > 150) // ~160mV
        {
            uint16_t scaled = (adc_value > 560) ? 560 : adc_value;
            uint8_t mid_brightness = 5 + ((scaled - 150) * 250) / 410;
            hardware_led_set(LED_4ER_RING, mid_brightness);
        }
        if (adc_value > 250) // ~270mV
        {
            uint16_t scaled = (adc_value > 560) ? 560 : adc_value;
            uint8_t upper_brightness = 5 + ((scaled - 250) * 250) / 310;
            hardware_led_set(LED_3ER_RING, upper_brightness);
        }
        if (adc_value > 400) // ~430mV - getting close to 600mV max
        {
            uint16_t scaled = (adc_value > 560) ? 560 : adc_value;
            uint8_t tip_brightness = 5 + ((scaled - 400) * 250) / 160;
            hardware_led_set(LED_1ER_RING, tip_brightness);
        }
    }
    break;

    default:
        break;
    }
}

void lighting_set_effect(lighting_effect_t effect)
{

    g_lighting_state.current_effect = effect;
    g_lighting_state.effect_counter = 0;
}

void lighting_set_candle_intensity_boost(uint8_t boost)
{
    // Limit boost to 0-100 range
    if (boost > 100)
    {
        boost = 100;
    }
    g_lighting_state.candle_intensity_boost = boost;
}

// ============================================================================
// AUDIO-REACTIVE LIGHTING FUNCTIONS
// ============================================================================

void lighting_audio_reactive_note(uint16_t frequency)
{
    // Skip silent notes (frequency = 0)
    if (frequency == 0)
    {
        lighting_audio_reactive_off();
        return;
    }

    // Configure LED_3ER for audio mode (inlined - temporary override of mic setup)
    DDRB |= (1 << PIN_LED_3ER);   // PB3 as output for LED control during songs
    PORTB &= ~(1 << PIN_LED_3ER); // Ensure LED_3ER starts off

    // Turn off all audio-reactive LEDs first
    lighting_audio_reactive_off();

    // DEBUG: Uncomment the next 2 lines to light all LEDs for any note (test mode)
    // PORTB |= (1 << PIN_LED_1ER) | (1 << PIN_LED_3ER) | (1 << PIN_LED_4ER) | (1 << PIN_LED_5ER);
    // return;

    // Direct pin manipulation - simple and stable (4-LED setup)
    // Higher frequencies → lower ring numbers (top to bottom)

    // LED_1ER (Tip): Highest notes C5 and above
    if (frequency >= AUDIO_NOTE_LED_1ER_MIN)
    {
        PORTB |= (1 << PIN_LED_1ER); // Turn on LED_1ER directly
    }
    // LED_3ER (Upper): High-mid notes A4 to B4
    else if (frequency >= AUDIO_NOTE_LED_3ER_MIN && frequency <= AUDIO_NOTE_LED_3ER_MAX)
    {
        PORTB |= (1 << PIN_LED_3ER); // Turn on LED_3ER directly - OK during songs
    }
    // LED_4ER (Middle): Mid notes F4 to G4
    else if (frequency >= AUDIO_NOTE_LED_4ER_MIN && frequency <= AUDIO_NOTE_LED_4ER_MAX)
    {
        PORTB |= (1 << PIN_LED_4ER); // Turn on LED_4ER directly
    }
    // LED_5ER (Base): Low notes E4 and below
    else if (frequency <= AUDIO_NOTE_LED_5ER_MAX)
    {
        PORTB |= (1 << PIN_LED_5ER); // Turn on LED_5ER directly
    }
    // LED_3ER: Not used - shared with microphone
}

void lighting_audio_reactive_off(void)
{
    // Turn off audio-reactive LEDs directly - simple and fast (4-LED setup)
    PORTB &= ~(1 << PIN_LED_1ER); // Turn off LED_1ER
    PORTB &= ~(1 << PIN_LED_3ER); // Turn off LED_3ER - OK during songs
    PORTB &= ~(1 << PIN_LED_4ER); // Turn off LED_4ER
    PORTB &= ~(1 << PIN_LED_5ER); // Turn off LED_5ER
}
