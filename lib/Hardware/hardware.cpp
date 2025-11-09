/*
 * hardware.cpp - Hardware abstraction layer for BlinkyTree
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#include "hardware.h"
#include <string.h>
#include <avr/eeprom.h>

// PRIVATE VARIABLES
static bool hardware_initialized = false;
static volatile uint32_t g_millis_counter = 0;
static uint8_t led_brightness_active[LED_COUNT_MAX] = {0};
static uint8_t led_brightness_pending[LED_COUNT_MAX] = {0};
static volatile bool buffer_swap_pending = false;
static volatile uint8_t pwm_counter = 0;
static const uint8_t led_pins[LED_COUNT_MAX] = {PIN_LED_1ER, PIN_LED_3ER, PIN_LED_4ER, PIN_LED_5ER};
static volatile bool led3_mic_reading = false;

// ============================================================================
// TIMER INTERRUPT FOR MILLIS COUNTER
// ============================================================================

// Timer0 compare match interrupt - 1ms tick
ISR(TIMER0_COMPA_vect) // ATtiny85 uses TIMER0_COMPA_vect
{
    g_millis_counter++;
}

// ============================================================================
// HARDWARE INITIALIZATION
// ============================================================================

void hardware_init(void)
{
    sei();

    // BOOST CPU FREQUENCY: Remove clock prescaler to run at full speed
    // ATtiny85: 8MHz internal oscillator, remove /8 prescaler = 8MHz
    CLKPR = (1 << CLKPCE); // Enable clock prescaler change
    CLKPR = 0;             // Set prescaler to 1 (no division) = 8MHz

#ifdef RESET_PIN_AS_IO
    // PRODUCTION BUILD: All pins are dedicated, no sharing needed
    DDRB |= (1 << PIN_LED_1ER) | (1 << PIN_LED_3ER) | (1 << PIN_LED_4ER) | (1 << PIN_LED_5ER);

#ifdef OLD_HARDWARE_REVISION
    // OLD HARDWARE: PB5 is BUZZER (former reset pin)
    PORTB &= ~(1 << BUZZER_PIN); // Ensure PB5 (buzzer) starts LOW, disable pull-up
#else
    // NEW HARDWARE: PB5 is LED_3ER (former reset pin)
    PORTB &= ~(1 << PIN_LED_3ER); // Ensure PB5 (LED_3ER) starts LOW, disable pull-up
#endif

#else
    // DEBUG BUILD: Initialize regular LED pins as outputs (exclude shared pin PB3/LED_3ER)
    DDRB |= (1 << PIN_LED_1ER) | (1 << PIN_LED_4ER) | (1 << PIN_LED_5ER);
    // Note: PIN_LED_3ER (PB3) is handled by shared-pin functions as needed
#endif


#if FEATURE_MICROPHONE_SENSOR
    // Initialize microphone ADC and configure PB3 pin
    hardware_microphone_init();
#endif

    // Restore Timer0 to 1ms tick for timing system

    // Timer0 CTC mode for 1ms interrupt - needed for lighting effects timing
    // IMPORTANT: Ensure Compare Output modes are disabled to allow normal I/O on PB0/PB1
    TCCR0A = (1 << WGM01);              // CTC mode, COM0A1:0=00, COM0B1:0=00 (normal I/O)
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64

    // Calculate OCR0A for 1ms interrupt based on CPU frequency
    
    // ATtiny85: 8MHz CPU, prescaler 64, target 1000Hz
    // OCR0A = (8,000,000 / (64 * 1000)) - 1 = 124
    OCR0A = 124;

    // Enable Timer Compare 
    TIMSK |= (1 << OCIE0A); // ATtiny85 uses TIMSK 

    sei();


    hardware_initialized = true;
}

void hardware_update(void)
{
    pwm_counter++; // Automatic rollover at 256 (uint8_t)

    // Double buffer: Swap buffers at start of PWM cycle to avoid glitches
    if (pwm_counter == 0 && buffer_swap_pending)
    {
        // Safe to swap at PWM cycle boundary (all LEDs just turned off)
        memcpy(led_brightness_active, led_brightness_pending, LED_COUNT_MAX);
        buffer_swap_pending = false;
    }

    // Direct PWM control for LEDs - conditional handling based on build type
#ifdef RESET_PIN_AS_IO
    // PRODUCTION BUILD: All pins dedicated, including LED_3ER on PB5
    // Clear only LED pins, preserve BUZZER_PIN (PB4) and MIC_INPUT (PB3) states
    uint8_t port_state = PORTB & ~((1 << PIN_LED_1ER) | (1 << PIN_LED_3ER) | (1 << PIN_LED_4ER) | (1 << PIN_LED_5ER));

    // Set all LED pins high if brightness > PWM counter (use active buffer)
    if (led_brightness_active[LED_1ER_RING] > pwm_counter)
        port_state |= (1 << PIN_LED_1ER);
    if (led_brightness_active[LED_3ER_RING] > pwm_counter)
        port_state |= (1 << PIN_LED_3ER); // PB5 in production
    if (led_brightness_active[LED_4ER_RING] > pwm_counter)
        port_state |= (1 << PIN_LED_4ER);
    if (led_brightness_active[LED_5ER_RING] > pwm_counter)
        port_state |= (1 << PIN_LED_5ER);

#else
    // DEBUG BUILD: Preserve BUZZER_PIN (PB4) and SHARED_PIN (PB3) states when updating other LEDs
    uint8_t port_state = PORTB & ~((1 << PIN_LED_1ER) | (1 << PIN_LED_4ER) | (1 << PIN_LED_5ER));

    // Set regular LED pins high if brightness > PWM counter (use active buffer)
    if (led_brightness_active[LED_1ER_RING] > pwm_counter)
        port_state |= (1 << PIN_LED_1ER);
    if (led_brightness_active[LED_4ER_RING] > pwm_counter)
        port_state |= (1 << PIN_LED_4ER);
    if (led_brightness_active[LED_5ER_RING] > pwm_counter)
        port_state |= (1 << PIN_LED_5ER);

    // LED_3ER (PB3) handled separately due to sharing with microphone
#endif

    // Update LEDs while preserving buzzer pin
    PORTB = port_state;

} // ============================================================================
// LED CONTROL FUNCTIONS
// ============================================================================

// hardware_led_init removed - was never called

void hardware_led_set(led_id_t led, uint8_t brightness)
{
    if (led >= LED_COUNT_MAX)
        return;

    // Store brightness in pending buffer - will be swapped on next PWM cycle
    led_brightness_pending[led] = brightness;
    buffer_swap_pending = true; // Signal that new values are ready
}

void hardware_led_all_off(void)
{
    PORTB &= ~((1 << PIN_LED_1ER) | (1 << PIN_LED_3ER) | (1 << PIN_LED_5ER));
    memset(led_brightness_active, 0, sizeof(led_brightness_active));
    memset(led_brightness_pending, 0, sizeof(led_brightness_pending));
    buffer_swap_pending = false;
}


void hardware_init_pwm(void)
{
    // Only set buzzer pin as output, do not configure Timer0 for PWM
    DDRB |= (1 << BUZZER_PIN);   // PB4 as output for bit-banging
    PORTB &= ~(1 << BUZZER_PIN); // Ensure buzzer starts LOW to prevent noise
}

// ============================================================================
// TIMING FUNCTIONS
// ============================================================================
uint32_t hardware_get_millis(void)
{
    uint32_t millis_copy;
    cli();
    millis_copy = g_millis_counter;
    sei();
    return millis_copy;
}


// ============================================================================
// MICROPHONE SENSOR (ADC)
// ============================================================================

void hardware_microphone_init(void)
{
#ifdef RESET_PIN_AS_IO
    // PRODUCTION BUILD: Microphone on dedicated PB3 (ADC3)
    DDRB &= ~(1 << PIN_MIC_INPUT);  // PB3 as input for ADC (dedicated)
    PORTB &= ~(1 << PIN_MIC_INPUT); // Disable pull-up (required for ADC input)

    // Configure ADC for microphone input on PB3 (ADC3)
    // Use internal 1.1V reference for stable operation regardless of battery voltage
    ADMUX = (1 << REFS1) | (1 << MUX1) | (1 << MUX0);                  // 1.1V internal reference, ADC3 (PB3)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler 128 for stable readings
#else
    // DEBUG BUILD: Configure PB3 (shared pin) as ADC input for microphone
    DDRB &= ~(1 << SHARED_PIN_MIC_LED);  // PB3 as input for ADC
    PORTB &= ~(1 << SHARED_PIN_MIC_LED); // Disable pull-up (required for ADC input)

    // Configure ADC for microphone input on PB3 (ADC3)
    // Use internal 1.1V reference for stable operation regardless of battery voltage
    ADMUX = (1 << REFS1) | (1 << MUX1) | (1 << MUX0);                  // 1.1V internal reference, ADC3 (PB3)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler 128 for stable readings
#endif

    // Allow ADC to settle with new reference
    _delay_ms(10);
}

uint16_t hardware_microphone_read(void)
{
    // PB3 is already set as input permanently, just read ADC
    _delay_us(5); // Brief settling delay

    // Single ADC conversion
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
    {
        // Wait for conversion complete
    }

    uint16_t result = ADC;
    return result;
}

uint16_t hardware_microphone_read_filtered(void)
{
    // Simple averaging filter - keep sampling fast to minimize LED disruption
    uint32_t sum = 0;
    const uint8_t samples = 8;

    for (uint8_t i = 0; i < samples; i++)
    {
        sum += hardware_microphone_read();
        _delay_us(50); // Reduced delay - faster sampling
    }

    return (uint16_t)(sum / samples);
}

// ============================================================================
// AUDIO OUTPUT (PWM)
// ============================================================================

void hardware_audio_set_frequency(uint16_t frequency)
{

    (void)frequency; // Suppress unused parameter warning for full audio system
    // For full audio system: use bit-banging like simple audio
    // Hardware PWM doesn't work well for low audio frequencies
    // We'll just store the frequency and let audio system handle the bit-banging

    // No hardware setup needed - audio.cpp will handle the bit-banging
    // Just ensure the pin is set as output
    DDRB |= (1 << BUZZER_PIN);

    // The actual tone generation will be handled in the audio_update() function
    // using the same proven bit-banging approach as audio_simple_play_tone_timed()
}

void hardware_audio_stop(void) { PORTB &= ~(1 << BUZZER_PIN); }


// ============================================================================
// EEPROM STORAGE (Persistent storage across resets)
// ============================================================================

uint8_t hardware_eeprom_read_byte(uint16_t address)
{
    return eeprom_read_byte((uint8_t *)address);
}

void hardware_eeprom_write_byte(uint16_t address, uint8_t data)
{
    // Only write if the data is different to preserve EEPROM life
    uint8_t current_value = eeprom_read_byte((uint8_t *)address);
    if (current_value != data)
    {
        eeprom_write_byte((uint8_t *)address, data);
    }
}
