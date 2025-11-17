# BlinkyTree

An interactive ATtiny85-powered Christmas tree with LED effects and breath-controlled interaction.

<p align="center">
  <img src="media/BlinkyTree_Front.jpg" width="70%" alt="BlinkyTree in Action" />
</p>

**Copyright (c) 2025 monkeyToneCircuits**  
**Licensed under CC-BY-NC 4.0** - See [LICENSE](LICENSE) file for details.

## Demo Video

<video src="media/BlinkyTree_ShowRun.mp4" width="400" controls></video>

*Video shows the candle effect, breath detection, and melody playback with LED frequency visualization.*

## Overview

BlinkyTree is my small project I invented a few years back that finally made it into the world! :)

It's an interactive Christmas tree that flickers in a candle-like manner. When you blow onto the microphone, the tree kicks off one of its songs, playing it on the buzzer. The LED rings blink in correspondence to the frequency of the notes in the songs.

Have fun with it, and if you want to build your own, you can get yours at [BlinkyParts.com](https://shop.blinkyparts.com/de/BlinkyTree-Interaktiver-Weihnachtsbaum-Loetbausatz/blink23179)!

It's built around the ATtiny85 microcontroller.

## 🎵 Customize Your Songs!

**New in v2.0:** You can now customize which songs play on your BlinkyTree without recompiling locally!

1. **Fork this repository** on GitHub
2. **Edit `config.yaml`** to:
   - Enable/disable songs
   - Adjust playback settings (speed, pitch, duty cycle)
3. **Add your own songs** by placing MusicXML files in the `songs/` folder
4. **Push your changes** - GitHub Actions automatically builds your custom firmware
5. **Download the `.hex` file** from the Actions artifacts and flash to your ATtiny85!

See the [Adding Custom Songs](#adding-custom-songs) section for detailed instructions.

## Features

- **Realistic Candle Effect**: Four LED rings flicker independently like real candles
- **Breath Detection**: Microphone sensor detects blowing
  - Light breath → Increased candle flickering intensity
  - Strong breath → Plays a Christmas melody
- **Audio Playback**: Built-in buzzer plays festive songs (Jingle Bells, Silent Night, etc.)
- **Modular Design**: Feature-configurable firmware for different hardware variants
- **Low Power**: Optimized for battery operation (60% Flash, 29% RAM usage)

## Project Structure

```
BlinkyTree/
├── src/
│   └── main.cpp                   # Main application loop
├── lib/
│   ├── Hardware/                  # Hardware abstraction (GPIO, PWM, ADC, timing)
│   ├── Lighting/                  # LED effects and candle simulation
│   ├── Audio/                     # Melody playback and tone generation
│   │   ├── audio.h/cpp            # Core audio system
│   │   └── audio_songs_generated.* # Auto-generated from MusicXML (do not edit)
│   └── Sensors/                   # Microphone sensor processing
├── songs/                         # MusicXML song files
│   ├── JINGLE_BELLS.musicxml
│   ├── Stille_Nacht_ChipVersion.musicxml
│   └── ...
├── scripts/
│   ├── generate_audio_code.py    # MusicXML → C code converter
│   └── pre_build.py               # PlatformIO pre-build script
├── config/
│   └── config.h                   # Low-level hardware configuration
├── config.yaml                    # Song configuration (edit this!)
├── platformio.ini                 # Build configuration
└── README.md
```

**New in v2.0:** Songs are now defined in `config.yaml` and `songs/*.musicxml` files, automatically converted to C code during build.

## Hardware Requirements
The set containing all components and pcbs can be bought here:
[BlinkyParts.com](https://www.blinkyparts.com)

### Components
- ATtiny85 microcontroller (8MHz internal oscillator)
- 4 LED rings (1, 3, 4, 5 LEDs respectively)
- Electret microphone with peak detector circuit
- Piezo buzzer or small speaker
- ISP programmer (USBasp, Arduino as ISP, etc.)
- Power supply: 3-5V DC (battery or USB)

### ATtiny85 Pinout

```
      ┌─────────────┐
 RESET│1  •      8 │ VCC (+3V to +5V)
  MIC │2         7 │ LED_1ER
BUZZER│3         6 │ LED_5ER (PWM)
  GND │4         5 │ LED_4ER (PWM)
      └─────────────┘
```


## Software Setup

### Prerequisites

1. **Install PlatformIO**
   - Open VS Code
   - Install the "PlatformIO IDE" extension
   - Restart VS Code

2. **Install AVR Toolchain** (automatic via PlatformIO)
   - The toolchain installs automatically when you first build

### Using Command Line

```bash
# Build firmware
pio run

# Upload to ATtiny85 (requires ISP programmer)
pio run --target upload

```

### Flashing from GitHub Actions Artifacts

If you've customized your firmware using GitHub Actions, you'll receive a `.hex` file. Here's how to flash it:

#### Prerequisites
- **ISP Programmer**: USBasp, Atmel ICE, Arduino as ISP, or similar
- **avrdude**: Command-line tool for AVR programming
  - Windows: Download from [AVR Downloads](https://github.com/avrdudes/avrdude/releases)
  - Linux: `sudo apt-get install avrdude`
  - macOS: `brew install avrdude`

#### Step 1: Connect ISP Programmer

Connect your ISP programmer to the 6-pin ISP header on the BlinkyTree PCB:

```
ISP Header Pinout:
┌─────────────┐
│ MISO    VCC │ 1 2
│ SCK     MOSI│ 3 4
│ RESET   GND │ 5 6
└─────────────┘
```

**Pin Mapping to ATtiny85:**
- **MISO** → PB1 (Pin 6)
- **MOSI** → PB0 (Pin 5)
- **SCK** → PB2 (Pin 7)
- **RESET** → RESET (Pin 1)
- **VCC** → Power (3-5V)
- **GND** → Ground

#### Step 2: Flash the Hex File

**Using avrdude (Universal Method):**

```bash
# For USBasp programmer:
avrdude -c usbasp -p attiny85 -U flash:w:firmware.hex:i

# For Arduino as ISP:
avrdude -c avrisp -P COM6 -b 19200 -p attiny85 -U flash:w:firmware.hex:i

# For Atmel ICE:
avrdude -c atmelice_isp -p attiny85 -U flash:w:firmware.hex:i

# Verify after flashing:
avrdude -c usbasp -p attiny85 -U flash:v:firmware.hex:i
```

**Using PlatformIO (if you have the project):**

1. Place your custom `.hex` file in `.pio/build/attiny85_debug/`
2. Rename it to `firmware.hex`
3. Run: `pio run --target upload`

#### Step 3: Set Fuses (One-Time Setup)

**Note:** If you purchased your BlinkyTree from [BlinkyParts.com](https://shop.blinkyparts.com/de/BlinkyTree-Interaktiver-Weihnachtsbaum-Loetbausatz/blink23179), the fuses are already configured correctly and you can skip this step.

For new or blank ATtiny85 chips, you need to set the fuses correctly:

```bash
avrdude -c usbasp -p attiny85 -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m
```

**Fuse Settings Explained:**
- **lfuse = 0xE2**: 8MHz internal oscillator, no clock division
- **hfuse = 0xDF**: RESET enabled, brown-out detection at 1.8V
- **efuse = 0xFF**: Self-programming disabled

#### Troubleshooting Flash Process

**"Device not found" or "Initialization failed":**
- Check all 6 ISP connections (especially GND and VCC)
- Verify programmer is recognized by your computer (`lsusb` on Linux)
- Ensure target has power (measure 3-5V on VCC pin)
- Try slower programming speed: `avrdude -c usbasp -B 10 -p attiny85 ...`

**"Verification error":**
- Poor connection quality - check jumper wires
- Interference from other circuits - disconnect buzzer/LEDs during programming
- Flash corruption - re-download hex file

**USBasp driver issues (Windows):**
- Install Zadig driver: [zadig.akeo.ie](https://zadig.akeo.ie/)
- Select USBasp device, install WinUSB or libusb-win32 driver

### ISP Programmer Connection
At the bottom of the PCB there is a connector prepared for a standard ISP programming adapter with 6 pins. 


## Behavior

### Startup

1. System initializes hardware (LEDs, microphone, buzzer)
2. Microphone calibrates baseline noise level
3. Candle effect starts automatically
4. Optional startup melody plays (configurable)

### Normal Operation

**Ambient Mode:**
- All four LED rings flicker independently with candle-like randomness
- Soft, warm ambiance

**Light Breath Detected:**
- Candle flicker intensity increases (0-50% boost)
- Proportional to breath strength
- Smooth transitions

**Strong Breath Detected:**
- Triggers Christmas melody playback
- One of seven songs plays (rotates through list)
- Cooldown period prevents immediate re-triggering

### Available Melodies

- Oh Christmas Tree
- Silent Night
- Jingle Bells
- Zelda Theme
- The First Noel
- Imperial March
- Kling Glöckchen

## Configuration

### Feature Flags (`config/config.h`)

```c
// Lighting Effects (choose ONE)
#define FEATURE_CANDLE_EFFECT 1        // Realistic candle flickering
#define FEATURE_BREATHING_EFFECT 0     // Smooth breathing animation
#define FEATURE_CANDLE_EFFECT_KRANZ 0  // Alternative candle mode

// Sensors and Audio
#define FEATURE_MICROPHONE_SENSOR 1    // Enable breath detection
#define FEATURE_AUDIO_OUTPUT 1         // Enable melody playback

// Storage
#define FEATURE_EEPROM_SETTINGS 1      // Save settings to EEPROM
```

### Breath Sensitivity Tuning

Adjust thresholds in `config/config.h`:

```c
#define BREATH_LIGHT_THRESHOLD 50      // Light breath trigger
#define BREATH_STRONG_THRESHOLD 150    // Strong breath (melody trigger)
```

Lower values = more sensitive  
Higher values = less sensitive

## Memory Usage

- Flash: ~5KB (60-70% of 8KB)
- RAM: ~150 bytes (25-30% of 512 bytes)

Plenty of room for customization!

## Troubleshooting

### Upload Fails
- Verify ISP programmer is connected correctly
- Check COM port in `platformio.ini` matches your programmer
- Ensure ATtiny85 has power (3-5V)
- Try slower baud rate: `upload_speed = 9600`

### LEDs Don't Light Up
- Check LED polarity (anode/cathode)
- Verify current-limiting resistors (220Ω recommended)
- Test with brightness test mode (see `lighting.h`)

### Microphone Not Responding
- Verify microphone circuit connection to Pin 2 (PB3)
- Check baseline calibration value (should be ~200)
- Adjust `BREATH_LIGHT_THRESHOLD` lower for higher sensitivity

## Development

### Adding New Melodies

1. Define notes in `lib/Audio/audio.cpp`:
```c
static const audio_note_t melody_my_song[] PROGMEM = {
    {NOTE_C4, 500}, {NOTE_E4, 500}, {NOTE_G4, 1000},
    {0, 0} // Terminator
};
```

2. Add to melody list and register in `audio_init()`

### Custom LED Effects

1. Add effect enum in `lib/Lighting/lighting.h`
2. Implement update function in `lib/Lighting/lighting.cpp`
3. Call via `lighting_set_effect(YOUR_EFFECT)`

## Adding Custom Songs (v2.0+)

### Quick Start with GitHub Actions

The easiest way to customize your BlinkyTree is to use GitHub Actions - no local setup required!

1. **Fork this repository** on GitHub
2. **Edit `config.yaml`** in the GitHub web interface:
   - Enable/disable songs (`enabled: true/false`)
   - Adjust `duty_cycle` (10-100) for volume/tone
   - Adjust `speed` (25-200) for tempo
   - Adjust `transpose` (-12 to +12) for pitch shifting
3. **Commit changes** - GitHub Actions builds your firmware automatically
4. **Download `.hex` file** from Actions → Artifacts
5. **Flash to ATtiny85** using your AVR programmer

### Adding New Songs from MusicXML

#### Create/Export MusicXML
- Use **MuseScore** (free): File → Export → MusicXML
- Or **Finale/Sibelius** with MusicXML export
- Requirements: Single melody, G3-B5 range, standard note durations

#### Add to Repository
1. Upload `.musicxml` file to `songs/` folder
2. Add entry to `config.yaml`:
   ```yaml
   songs:
     My_Song:
       enabled: true
       duty_cycle: 80
       speed: 150
       transpose: 0
   ```
3. Commit → Wait for build → Download firmware

### Configuration Parameters

- **duty_cycle** (10-100%): Buzzer PWM duty cycle (higher = louder)
- **speed** (25-200%): Playback tempo (100% = original)
- **transpose** (-12 to +12): Semitone pitch shift

### Local Development

```bash
pip install pyyaml platformio
pio run                    # Build
pio run --target upload    # Upload to ATtiny85
```

The build system automatically parses MusicXML and generates C code.

## Resources

- [ATtiny85 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [AVR Libc Reference](https://www.nongnu.org/avr-libc/user-manual/index.html)

## Hardware Design

*PCB design and schematic documentation coming soon.*

## License

This project is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License (CC-BY-NC 4.0).

You are free to:
- **Share** — copy and redistribute the material
- **Adapt** — remix, transform, and build upon the material

Under the terms:
- **Attribution** — Give appropriate credit to monkeyToneCircuits
- **NonCommercial** — Not for commercial use

See [LICENSE](LICENSE) for full details.

---

**Made with ❤️ by monkeyToneCircuits**