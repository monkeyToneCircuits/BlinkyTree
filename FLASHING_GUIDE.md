# How to Flash Your BlinkyTree Firmware

This guide is for users who want to upload custom firmware to their BlinkyTree without any programming experience.

## What You Need

1. **Your BlinkyTree** (purchased from BlinkyParts.com)
2. **ISP Programmer** - One of these:
   - USBasp (most common, ~$3-5 on Amazon/eBay)
   - AVRISPv2 (official Atmel programmer)
   - Arduino as ISP (use an Arduino board as programmer)
3. **USB Cable** to connect the programmer to your computer
4. **6-pin ISP cable** to connect programmer to BlinkyTree

## Step-by-Step Instructions

### 1. Download Your Custom Firmware

1. Go to your forked repository on GitHub
2. Click the **"Actions"** tab at the top
3. Click on the latest successful build (green checkmark ✓)
4. Scroll down to **"Artifacts"** section
5. Download the ZIP file (e.g., `BlinkyTree_firmware_20241117_123456.zip`)
6. **Extract** the ZIP file to a folder on your Desktop

You should now have a file called `firmware.hex`

### 2. Install PlatformIO (One-Time Setup)

**Option A: Quick Install (Recommended)**
1. Download and install Python from [python.org](https://www.python.org/downloads/)
   - ✅ Check "Add Python to PATH" during installation!
2. Open Command Prompt (Windows Key + R, type `cmd`, press Enter)
3. Type: `pip install platformio`
4. Wait for installation to complete

**Option B: VS Code Extension**
1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code → Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install

### 3. Connect Your ISP Programmer

**Physical Connection:**

1. **Prepare the PCB for programming:**
   - ⚠️ **IMPORTANT:** The BlinkyTree PCB has 4 solder bridges near the ISP header
   - These bridges are **cut by default** to reduce EMC interference during normal operation
   - **Before flashing:** You must solder these 4 bridges closed to connect the programmer
   - The bridges connect: MISO, MOSI, SCK, and RESET lines to the ISP header
   - After flashing (optional): You can cut them again for better EMC performance

2. **Connect ISP programmer to BlinkyTree:**
   - Locate the 6-pin header on the bottom of the BlinkyTree PCB
   - Connect the 6-pin ribbon cable from programmer to BlinkyTree
   - ⚠️ Check pin 1 orientation (usually marked with a triangle or dot)

```
ISP Header (BlinkyTree PCB): 

⚠️ If the Connector is on the right side, the notch should face to you

┌─────────────┐
│ MISO    VCC │ Pin 1-2
│ SCK     MOSI│ Pin 3-4  
│ RESET   GND │ Pin 5-6
└─────────────┘

Solder Bridges (must be closed for programming):
[MISO] [MOSI] [SCK] [RESET]
```

3. **Connect programmer to computer:**
   - Plug the USB cable into your computer
   - Windows should recognize the device
   - If not, you may need to install drivers (see Troubleshooting below)

4. **Power the BlinkyTree:**
   - Either power via ISP programmer (if it supplies power)
   - Or use batteries/external power supply (3-5V)

### 4. Flash the Firmware

**Easy Method (Windows):**

1. Copy `flash_firmware.bat` to the same folder as `firmware.hex`
2. Double-click `flash_firmware.bat`
3. Follow the on-screen instructions
4. Wait for "SUCCESS!" message

**Manual Method (All Platforms):**

Open Command Prompt/Terminal in the folder with `firmware.hex` and run:

**For USBasp:**
```bash
pio device run -e attiny85_avrispv2 --target upload
```

Or directly with avrdude:
```bash
avrdude -c usbasp -p attiny85 -U flash:w:firmware.hex:i
```

**For AVRISPv2:**
```bash
avrdude -c avrisp2 -P usb -p attiny85 -U flash:w:firmware.hex:i
```

### 5. Test Your BlinkyTree

1. Disconnect the ISP programmer
2. Power on your BlinkyTree (batteries or USB)
3. LEDs should start with candle effect
4. Blow on the microphone to trigger a song!

---

## Troubleshooting

### "Verification error" or "Flash failed"

- **Check connections:** Make sure all 6 pins are properly connected
- **Check power:** Measure voltage on VCC pin (should be 3-5V)


### "avrdude: error: could not find USB device"

- Try a different USB port
- Check if programmer's LED is on
- On Linux, you may need `sudo` or udev rules
- Try unplugging and replugging the programmer

### "Target doesn't answer"

- **RESET pin might be disabled** (if previously flashed with wrong Fuses)
  - This is not recoverable with standard ISP programmer
  - Need high-voltage programmer (HV programmer)
  - If bought from BlinkyParts, this shouldn't happen

### Still not working?

- Post an issue on the [GitHub repository](https://github.com/monkeyToneCircuits/BlinkyTree/issues)
- Include error messages and programmer type
- Check hardware connections with multimeter

---

## For Advanced Users

### Direct avrdude Command

If you have avrdude installed system-wide:

```bash
avrdude -c usbasp -p attiny85 -U flash:w:firmware.hex:i -v
```

### Verify Flash

```bash
avrdude -c usbasp -p attiny85 -U flash:v:firmware.hex:i
```

### Read Current Fuses

```bash
avrdude -c usbasp -p attiny85 -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h
```

### Set Fuses (⚠️ Only if needed!)

**Standard configuration (RESET enabled):**
```bash
avrdude -c usbasp -p attiny85 -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m
```

**If bought from BlinkyParts.com:** Fuses are already set correctly, skip this!

---

## What's Next?

### Customize Your Songs

1. Fork the repository
2. Edit `config.yaml` to enable/disable songs
3. Add your own MusicXML files to `songs/` folder
4. Push changes → GitHub Actions builds new firmware automatically
5. Download and flash again!

### Join the Community

- ⭐ Star the repository on GitHub
- 🐛 Report bugs via Issues
- 💡 Share your custom songs and improvements

---

**Made with ❤️ by monkeyToneCircuits**  
Licensed under CC-BY-NC 4.0
