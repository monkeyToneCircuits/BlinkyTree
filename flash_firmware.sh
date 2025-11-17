#!/bin/bash

echo "========================================"
echo "BlinkyTree Firmware Flasher"
echo "========================================"
echo ""

# Check if firmware.hex exists
if [ ! -f "firmware.hex" ]; then
    echo "ERROR: firmware.hex not found!"
    echo ""
    echo "Please make sure firmware.hex is in the same folder as this script."
    echo ""
    read -p "Press Enter to exit..."
    exit 1
fi

echo "Step 1: Checking for avrdude installation..."
if ! command -v avrdude &> /dev/null; then
    echo "avrdude not found!"
    echo ""
    echo "Please install avrdude:"
    echo "  Ubuntu/Debian: sudo apt-get install avrdude"
    echo "  Fedora/RHEL:   sudo dnf install avrdude"
    echo "  Arch Linux:    sudo pacman -S avrdude"
    echo "  macOS:         brew install avrdude"
    echo ""
    read -p "Press Enter to exit..."
    exit 1
fi

echo "Step 2: Detecting AVR programmer..."
echo ""
echo "Please ensure your ISP programmer is connected via USB."
echo "Supported programmers: USBasp, AVRISPv2, Arduino as ISP"
echo ""
read -p "Press Enter to continue..."

echo ""
echo "Step 3: Flashing firmware to ATtiny85..."
echo ""

# Try AVRISPv2 on USB first
echo "Trying AVRISPv2 on USB..."
avrdude -c avrisp2 -P usb -p attiny85 -U flash:w:firmware.hex:i 2>/dev/null
if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "SUCCESS! Firmware flashed successfully!"
    echo "========================================"
    echo ""
    echo "Your BlinkyTree is ready to use!"
    read -p "Press Enter to exit..."
    exit 0
fi

# Try AVRISPv2 on serial ports
echo ""
echo "AVRISPv2 USB not found, checking serial ports..."
for port in /dev/ttyUSB* /dev/ttyACM* /dev/cu.usb* /dev/cu.wchusbserial*; do
    if [ -e "$port" ]; then
        echo "Trying AVRISPv2 on $port..."
        avrdude -c avrisp2 -P "$port" -p attiny85 -U flash:w:firmware.hex:i 2>/dev/null
        if [ $? -eq 0 ]; then
            echo ""
            echo "========================================"
            echo "SUCCESS! Firmware flashed successfully!"
            echo "========================================"
            echo ""
            echo "Programmer found on $port"
            echo "Your BlinkyTree is ready to use!"
            read -p "Press Enter to exit..."
            exit 0
        fi
    fi
done

# Try USBasp
echo ""
echo "Trying USBasp programmer..."
avrdude -c usbasp -p attiny85 -U flash:w:firmware.hex:i 2>/dev/null
if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "SUCCESS! Firmware flashed successfully!"
    echo "========================================"
    echo ""
    echo "Your BlinkyTree is ready to use!"
    read -p "Press Enter to exit..."
    exit 0
fi

# Try Arduino as ISP on serial ports
echo ""
echo "Standard programmers not found, trying Arduino as ISP..."
for port in /dev/ttyUSB* /dev/ttyACM* /dev/cu.usb* /dev/cu.wchusbserial*; do
    if [ -e "$port" ]; then
        echo "Trying Arduino as ISP on $port..."
        avrdude -c avrisp -P "$port" -b 19200 -p attiny85 -U flash:w:firmware.hex:i 2>/dev/null
        if [ $? -eq 0 ]; then
            echo ""
            echo "========================================"
            echo "SUCCESS! Firmware flashed successfully!"
            echo "========================================"
            echo ""
            echo "Programmer found on $port"
            echo "Your BlinkyTree is ready to use!"
            read -p "Press Enter to exit..."
            exit 0
        fi
    fi
done

# All attempts failed
echo ""
echo "========================================"
echo "FLASH FAILED"
echo "========================================"
echo ""
echo "Possible issues:"
echo "  1. Programmer not connected properly"
echo "  2. ATtiny85 not powered (check battery/power supply)"
echo "  3. USB permissions issue (try with sudo)"
echo "  4. Wrong programmer type"
echo ""
echo "USB Permissions Fix (Linux):"
echo "  Create file: /etc/udev/rules.d/99-usbasp.rules"
echo "  Add line: SUBSYSTEM==\"usb\", ATTR{idVendor}==\"16c0\", ATTR{idProduct}==\"05dc\", MODE=\"0666\""
echo "  Run: sudo udevadm control --reload-rules"
echo "  Reconnect programmer"
echo ""
echo "For more help, see FLASHING_GUIDE.md"
echo ""
read -p "Press Enter to exit..."
exit 1
