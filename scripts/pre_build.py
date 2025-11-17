#!/usr/bin/env python3
"""
Pre-build script for PlatformIO
Generates audio code from MusicXML files before compilation
"""

Import("env")
import subprocess
import sys
from pathlib import Path

# Get project directory
project_dir = Path(env["PROJECT_DIR"])
script_path = project_dir / "scripts" / "generate_audio_code.py"

print("=" * 60)
print("PRE-BUILD: Generating audio code from MusicXML files...")
print("=" * 60)

try:
    # Run the code generation script
    result = subprocess.run(
        [sys.executable, str(script_path)],
        cwd=str(project_dir),
        capture_output=True,
        text=True,
        check=True
    )
    
    print(result.stdout)
    
    if result.stderr:
        print("Warnings:", result.stderr, file=sys.stderr)
    
    print("=" * 60)
    print("Audio code generation complete!")
    print("=" * 60)
    
except subprocess.CalledProcessError as e:
    print("=" * 60)
    print("ERROR: Audio code generation failed!")
    print("=" * 60)
    print(e.stdout)
    print(e.stderr, file=sys.stderr)
    env.Exit(1)
except Exception as e:
    print("=" * 60)
    print(f"ERROR: {e}")
    print("=" * 60)
    env.Exit(1)
