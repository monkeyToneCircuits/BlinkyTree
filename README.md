# BlinkyTree

An interactive ATtiny85-powered Christmas tree with LED candle effects and breath-controlled music playback.

<p align="center">
  <img src="media/BlinkyTree_Front.jpg" width="70%" alt="BlinkyTree in Action" />
</p>

**Get your kit at [BlinkyParts.com](https://shop.blinkyparts.com/de/BlinkyTree-Interaktiver-Weihnachtsbaum-Loetbausatz/blink23179)!**

## Demo

🎥 **[Watch the demo video](media/BlinkyTree_ShowRun.mp4)** - Shows candle effect, breath detection, and melody playback with LED visualization

## Features

- 🕯️ **Realistic Candle Effect** - Four LED rings flicker independently
- 🎤 **Breath Detection** - Blow to trigger songs or increase flicker intensity  
- 🎵 **Music Playback** - Built-in buzzer plays Christmas melodies
- 🎨 **LED Light Show** - LEDs react to note frequencies
- ⚙️ **Fully Customizable** - Add your own songs via GitHub Actions (no coding required!)

## 🎵 Customize Your Songs (No Setup Required!)

Customize songs directly on GitHub - no local development environment needed!

1. **Fork this repository** on GitHub
2. **Edit `config.yaml`** to enable/disable songs and adjust settings
3. **Add MusicXML files** to the `songs/` folder (export from MuseScore, Finale, etc.)
4. **Push changes** - GitHub Actions automatically builds your firmware
5. **Download the `.hex` file** from Actions → Artifacts
6. **Flash to your BlinkyTree** - See [FLASHING_GUIDE.md](FLASHING_GUIDE.md)

### Song Configuration

Edit `config.yaml` to customize each song:

```yaml
songs:
  JINGLE_BELLS:
    enabled: true
    duty_cycle: 80      # Volume (10-100)
    speed: 150          # Tempo (25-200, 100=original)
    transpose: 0        # Pitch shift (-12 to +12 semitones)
```

### Adding New Songs

1. Export your melody as MusicXML (MuseScore: File → Export → MusicXML)
   - ⚠️ **Important:** In MuseScore, export as **Uncompressed MusicXML** (not .mxl)
   - File extension should be `.musicxml`
2. Upload the `.musicxml` file to the `songs/` folder
3. Add an entry in `config.yaml` with the same filename (without extension)
4. Commit and push - GitHub Actions builds your custom firmware!

**Requirements:** Single melody line, note range G3-B5, standard durations

## Hardware

**ATtiny85 Pinout:**
```
      ┌─────────────┐
 RESET│1  •      8 │ VCC (+3-5V)
  MIC │2         7 │ LED_1ER
BUZZER│3         6 │ LED_5ER (PWM)
  GND │4         5 │ LED_4ER (PWM)
      └─────────────┘

Pin Mapping:
Pin 1: PB5 (RESET) - ISP Programming
Pin 2: PB3 (MIC/LED_3ER) - Shared/Time-multiplexed
Pin 3: PB4 (BUZZER)
Pin 5: PB0 (LED_4ER) - Hardware PWM
Pin 6: PB1 (LED_5ER) - Hardware PWM  
Pin 7: PB2 (LED_1ER) - Software PWM
```

**Components:**
- ATtiny85 microcontroller (8MHz internal)
- 4 LED rings (1, 3, 4, 5 LEDs)
- Electret microphone with peak detector
- Piezo buzzer
- Power: 3-5V (battery or USB)

## Quick Start

### Option 1: Flash Pre-Built Firmware

1. Download the latest firmware from [Releases](../../releases)
2. Follow the [FLASHING_GUIDE.md](FLASHING_GUIDE.md)
3. Use the included `flash_firmware.bat` (Windows) or `flash_firmware.sh` (Linux/macOS)

### Option 2: Build Locally

**Prerequisites:**
- PlatformIO Core or VS Code with PlatformIO extension
- Python 3.11+ with PyYAML (`pip install pyyaml`)

**Build and Upload:**
```bash
pio run                    # Build firmware
pio run --target upload    # Upload to ATtiny85 (requires ISP programmer)
```

## Flashing

### Quick Flash

**Windows:** Double-click `flash_firmware.bat`  
**Linux/macOS:** Run `./flash_firmware.sh`

The scripts auto-detect your ISP programmer and flash automatically.

### Manual Flash

```bash
# USBasp
avrdude -c usbasp -p attiny85 -U flash:w:firmware.hex:i

# AVRISPv2  
avrdude -c avrisp2 -P usb -p attiny85 -U flash:w:firmware.hex:i
```

**Need help?** See the complete [FLASHING_GUIDE.md](FLASHING_GUIDE.md) for:
- ISP programmer setup and connections
- Driver installation (Windows)
- Fuse configuration
- Detailed troubleshooting

## How It Works

### Startup
1. Hardware initialization (LEDs, microphone, buzzer)
2. Microphone calibration
3. Candle effect starts automatically

### Operation

**Idle:** Four LED rings flicker independently with realistic candle simulation

**Light Breath:** Candle flicker intensity increases proportionally

**Strong Breath:** Triggers a random song from your enabled playlist

## Configuration

### Song Settings (`config.yaml`)

Edit `config.yaml` to customize songs - this is what most users will need:

```yaml
songs:
  JINGLE_BELLS:
    enabled: true
    duty_cycle: 80      # Volume/tone (10-100)
    speed: 150          # Tempo (25-200, 100=original)
    transpose: 0        # Pitch shift (-12 to +12 semitones)
```

### Advanced Hardware Settings (`config/config.h`)

For advanced users who need to modify hardware behavior:

```c
// Breath Sensitivity (song trigger)
#define BREATH_STRONG_THRESHOLD 150    // Strong breath threshold for melody trigger
```

Lower values = more sensitive, triggers songs more easily

(if set too easy the Tree will start songs by itself)

## Project Structure

```
BlinkyTree/
├── config.yaml                    # Song configuration (edit this!)
├── songs/*.musicxml               # MusicXML song files
├── src/main.cpp                   # Main application
├── lib/
│   ├── Audio/                     # Audio system
│   │   ├── audio.h/cpp           # Core audio engine
│   │   └── audio_songs_generated.* # Auto-generated from MusicXML
│   ├── Lighting/                  # LED effects
│   ├── Hardware/                  # GPIO/PWM/ADC abstraction
│   └── Sensors/                   # Microphone processing
├── scripts/
│   ├── generate_audio_code.py    # MusicXML → C converter
│   └── pre_build.py              # Build automation
├── flash_firmware.bat/.sh         # Automated flashing scripts
└── FLASHING_GUIDE.md             # Detailed flashing instructions
```

## Troubleshooting

**LEDs not working:**
- Check power supply (3-5V)
- Verify LED polarity

**Microphone not responding:**
- Adjust `BREATH_STRONG_THRESHOLD` in `config/config.h` (lower = more sensitive)
- Check microphone circuit connection (Pin 2)
- Verify calibration during startup

**Upload fails:**
- Verify ISP connections (all 6 pins)
- Check programmer COM port in `platformio.ini`

See [FLASHING_GUIDE.md](FLASHING_GUIDE.md) for more details.

## Default Memory Usage

- **Flash:** ~5KB / 8KB (60-70%) 
- **RAM:** ~150 bytes / 512 bytes (25-30%)

## Resources

- **Hardware:** [BlinkyParts.com Kit](https://shop.blinkyparts.com/de/BlinkyTree-Interaktiver-Weihnachtsbaum-Loetbausatz/blink23179)
- **Flashing Guide:** [FLASHING_GUIDE.md](FLASHING_GUIDE.md)
- **ATtiny85 Datasheet:** [Microchip](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf)
- **PlatformIO:** [docs.platformio.org](https://docs.platformio.org/)

## License

**CC-BY-NC 4.0** - Non-commercial use only

You may share and adapt this project with attribution to **monkeyToneCircuits**.  
See [LICENSE](LICENSE) for details.

---

**Made with ❤️ by monkeyToneCircuits**
