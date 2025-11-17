@echo off
setlocal enabledelayedexpansion
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

echo Step 2: Select your programmer type...
echo.
echo Which ISP programmer are you using?
echo.
echo   1. USBasp (most common, cheap programmer from Amazon/eBay)
echo   2. AVRISPv2 (official Atmel/Microchip programmer)
echo   3. Arduino as ISP (using Arduino board as programmer)
echo   4. Try all automatically
echo.
set /p PROGRAMMER_CHOICE="Enter your choice (1-4): "

echo.
echo Please ensure your ISP programmer is connected via USB
echo and the 6-pin cable is connected to the BlinkyTree.
echo.
pause

echo Step 3: Flashing firmware to ATtiny85...
echo.

REM Try with PlatformIO's avrdude (most common setup)
set AVRDUDE_PATH=%USERPROFILE%\.platformio\packages\tool-avrdude\avrdude.exe
set AVRDUDE_CONF=%USERPROFILE%\.platformio\packages\tool-avrdude\avrdude.conf

if not exist "%AVRDUDE_PATH%" (
    echo PlatformIO's avrdude not found.
    echo.
    echo Please install PlatformIO Core first:
    echo    pip install platformio
    echo.
    pause
    exit /b 1
)

REM Handle programmer selection
if "%PROGRAMMER_CHOICE%"=="1" goto :FLASH_USBASP
if "%PROGRAMMER_CHOICE%"=="2" goto :FLASH_AVRISP2
if "%PROGRAMMER_CHOICE%"=="3" goto :FLASH_ARDUINO
if "%PROGRAMMER_CHOICE%"=="4" goto :FLASH_AUTO
goto :FLASH_AUTO

:FLASH_USBASP
echo Using USBasp programmer...
echo.
"%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c usbasp -p attiny85 -U flash:w:firmware.hex:i
goto :CHECK_RESULT

:FLASH_AVRISP2
echo Using AVRISPv2 programmer...
echo.
"%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c avrisp2 -P usb -p attiny85 -U flash:w:firmware.hex:i
goto :CHECK_RESULT

:FLASH_ARDUINO
echo Using Arduino as ISP...
echo.
echo Detecting COM port...
for /f "tokens=1" %%a in ('wmic path Win32_SerialPort where "Description like '%%Arduino%%' or Description like '%%USB Serial%%'" get DeviceID ^| findstr "COM"') do set COM_PORT=%%a
if "%COM_PORT%"=="" (
    set /p COM_PORT="Could not auto-detect. Enter COM port (e.g., COM3): "
)
echo Using COM port: %COM_PORT%
"%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c avrisp -P %COM_PORT% -b 19200 -p attiny85 -U flash:w:firmware.hex:i
goto :CHECK_RESULT

:FLASH_AUTO
echo Trying AVRISPv2 on USB first...
echo.
"%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c avrisp2 -P usb -p attiny85 -U flash:w:firmware.hex:i 2>nul
if %ERRORLEVEL% EQU 0 goto :SUCCESS

echo.
echo AVRISPv2 USB not found, checking COM ports...
echo.
for /f "tokens=1" %%a in ('wmic path Win32_SerialPort get DeviceID ^| findstr "COM"') do (
    echo Trying AVRISPv2 on %%a...
    "%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c avrisp2 -P %%a -p attiny85 -U flash:w:firmware.hex:i 2>nul
    if !ERRORLEVEL! EQU 0 (
        echo Found AVRISPv2 on %%a
        goto :SUCCESS
    )
)

echo.
echo AVRISPv2 not found, trying USBasp...
echo.
"%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c usbasp -p attiny85 -U flash:w:firmware.hex:i 2>nul
if %ERRORLEVEL% EQU 0 goto :SUCCESS

echo.
echo Standard programmers not found, trying Arduino as ISP...
echo.
for /f "tokens=1" %%a in ('wmic path Win32_SerialPort where "Description like '%%Arduino%%' or Description like '%%USB Serial%%'" get DeviceID ^| findstr "COM"') do set COM_PORT=%%a
if not "%COM_PORT%"=="" (
    echo Found Arduino on %COM_PORT%
    "%AVRDUDE_PATH%" -C "%AVRDUDE_CONF%" -c avrisp -P %COM_PORT% -b 19200 -p attiny85 -U flash:w:firmware.hex:i
    if %ERRORLEVEL% EQU 0 goto :SUCCESS
)
goto :FAILURE

:CHECK_RESULT
if %ERRORLEVEL% EQU 0 goto :SUCCESS
goto :FAILURE

:SUCCESS
echo.
echo ========================================
echo SUCCESS! Firmware flashed successfully!
echo ========================================
echo.
echo Your BlinkyTree is ready to use!
pause
exit /b 0

:FAILURE
echo.
echo ========================================
echo FLASH FAILED
echo ========================================
echo.
echo Possible issues:
echo  1. Programmer not connected properly
echo  2. Wrong programmer type selected
echo  3. ATtiny85 not powered (check battery/power supply)
echo  4. USB driver issues (especially for USBasp on Windows)
echo.
echo For USBasp on Windows:
echo  - Download and run Zadig from https://zadig.akeo.ie/
echo  - Install WinUSB or libusb-win32 driver for USBasp
echo.
echo For more help, see FLASHING_GUIDE.md
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
