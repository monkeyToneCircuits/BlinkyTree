@echo off
echo ========================================
echo BlinkyTree Firmware Flasher
echo ========================================
echo.

REM Check if firmware.hex exists
if not exist "firmware.hex" (
    echo ERROR: firmware.hex not found!
    echo.
    echo Please make sure firmware.hex is in the same folder as this script.
    echo.
    pause
    exit /b 1
)

echo Step 1: Checking for PlatformIO installation...
where pio >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo PlatformIO not found in PATH.
    echo.
    echo Please install PlatformIO Core:
    echo    pip install platformio
    echo.
    echo Or use the PlatformIO IDE extension in VS Code.
    echo.
    pause
    exit /b 1
)

echo Step 2: Detecting AVR programmer...
echo.
echo Please ensure your ISP programmer is connected via USB.
echo Supported programmers: USBasp, AVRISPv2, Arduino as ISP
echo.
pause

echo Step 3: Flashing firmware to ATtiny85...
echo.

REM Try with PlatformIO's avrdude (most common setup)
set AVRDUDE_PATH=%USERPROFILE%\.platformio\packages\tool-avrdude\avrdude.exe
set AVRDUDE_CONF=%USERPROFILE%\.platformio\packages\tool-avrdude\avrdude.conf

if exist "%AVRDUDE_PATH%" (
    echo Using PlatformIO's avrdude...
    echo.
    echo Trying USBasp programmer first...
    "%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c usbasp -p attiny85 -U flash:w:firmware.hex:i
    
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo SUCCESS! Firmware flashed successfully!
        echo ========================================
        echo.
        echo Your BlinkyTree is ready to use!
        pause
        exit /b 0
    )
    
    echo.
    echo USBasp failed, trying AVRISPv2...
    "%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c avrisp2 -P usb -p attiny85 -U flash:w:firmware.hex:i
    
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo SUCCESS! Firmware flashed successfully!
        echo ========================================
        echo.
        echo Your BlinkyTree is ready to use!
        pause
        exit /b 0
    )
    
    echo.
    echo ========================================
    echo FLASH FAILED
    echo ========================================
    echo.
    echo Possible issues:
    echo  - Programmer not connected properly
    echo  - Wrong programmer type
    echo  - ATtiny85 not powered
    echo  - USB driver issues
    echo.
    echo Please check connections and try again.
    echo.
    pause
    exit /b 1
) else (
    echo PlatformIO's avrdude not found.
    echo.
    echo Please install PlatformIO Core first:
    echo    pip install platformio
    echo.
    pause
    exit /b 1
)
