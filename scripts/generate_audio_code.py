#!/usr/bin/env python3
"""
MusicXML to C Code Generator for BlinkyTree
Converts MusicXML song files to C audio data structures

Copyright (c) 2025 monkeyToneCircuits
Licensed under CC-BY-NC 4.0
"""

import os
import sys
import yaml
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import List, Dict, Tuple, Optional

# Note frequency mapping (from audio.h - fine-tuned for ATtiny85 buzzer)
NOTE_FREQUENCIES = {
    'C': {3: 131, 4: 172, 5: 344, 6: 690},
    'CS': {3: 139, 4: 182, 5: 365, 6: 730},
    'D': {3: 147, 4: 192, 5: 386, 6: 773},
    'DS': {3: 156, 4: 204, 5: 408, 6: 818},
    'E': {3: 165, 4: 216, 5: 433, 6: 866},
    'F': {3: 175, 4: 229, 5: 459, 6: 918},
    'FS': {3: 185, 4: 243, 5: 486, 6: 972},
    'G': {3: 128, 4: 257, 5: 515, 6: 1030},
    'GS': {3: 136, 4: 273, 5: 546, 6: 1092},
    'A': {3: 144, 4: 290, 5: 580, 6: 1160},
    'AS': {3: 153, 4: 307, 5: 614, 6: 1228},
    'B': {3: 162, 4: 326, 5: 652, 6: 0},
}

# Duration constants from audio.h
SIXTEENTH_NOTE = 125
EIGHTH_NOTE = 250
QUARTER_NOTE = 500
HALF_NOTE = 1000
WHOLE_NOTE = 2000

class MusicXMLParser:
    """Parses MusicXML files and extracts note data"""
    
    def __init__(self, filepath: Path):
        self.filepath = filepath
        self.tree = ET.parse(filepath)
        self.root = self.tree.getroot()
        self.divisions = 1  # Will be set from the first measure
        self.tempo = 120  # Default BPM
        
    def parse(self) -> List[Tuple[int, int]]:
        """
        Parse MusicXML and return list of (frequency, duration) tuples
        Returns: List of (frequency_hz, duration_ms) tuples
        """
        notes = []
        
        # Find all measures
        for measure in self.root.findall('.//measure'):
            # Get divisions if specified in this measure
            divisions_elem = measure.find('.//divisions')
            if divisions_elem is not None:
                self.divisions = int(divisions_elem.text)
            
            # Process all notes in measure
            for note in measure.findall('.//note'):
                freq, duration = self._parse_note(note)
                if freq is not None and duration is not None:
                    # Check if this note is tied to the previous note
                    tie = note.find('tie')
                    if tie is not None and tie.get('type') == 'start':
                        # This note starts a tie - just add it normally
                        notes.append((freq, duration))
                    elif tie is not None and tie.get('type') == 'stop':
                        # This note is tied from previous - merge durations
                        if notes and notes[-1][0] == freq:
                            # Same frequency - extend previous note duration
                            prev_freq, prev_duration = notes[-1]
                            notes[-1] = (prev_freq, prev_duration + duration)
                        else:
                            # Different frequency (shouldn't happen) - add as new note
                            notes.append((freq, duration))
                    else:
                        # No tie - normal note
                        notes.append((freq, duration))
        
        return notes
    
    def _parse_note(self, note_elem) -> Tuple[Optional[int], Optional[int]]:
        """Parse a single note element"""
        # Skip notes from staves other than 1 (bass clef in piano arrangements)
        staff_elem = note_elem.find('staff')
        if staff_elem is not None and int(staff_elem.text) != 1:
            return (None, None)
        
        # Check if it's a rest
        if note_elem.find('rest') is not None:
            duration = self._get_duration(note_elem)
            return (0, duration)  # 0 frequency = rest
        
        # Get pitch
        pitch_elem = note_elem.find('pitch')
        if pitch_elem is None:
            return (None, None)
        
        step = pitch_elem.find('step').text
        octave = int(pitch_elem.find('octave').text)
        alter = pitch_elem.find('alter')
        
        # Handle sharps/flats
        note_name = step
        if alter is not None:
            alter_val = int(alter.text)
            if alter_val == 1:  # Sharp
                note_name += 'S'
            elif alter_val == -1:  # Flat - convert to equivalent sharp
                note_name = self._flat_to_sharp(step)
        
        # Get frequency
        freq = NOTE_FREQUENCIES.get(note_name, {}).get(octave, 0)
        if freq == 0:
            print(f"Warning: Unknown note {note_name}{octave} in {self.filepath.name}", file=sys.stderr)
            return (None, None)
        
        # Get duration
        duration = self._get_duration(note_elem)
        
        return (freq, duration)
    
    def _get_duration(self, note_elem) -> int:
        """Calculate note duration in milliseconds"""
        duration_elem = note_elem.find('duration')
        if duration_elem is None:
            return QUARTER_NOTE
        
        # Duration in divisions
        duration_divs = int(duration_elem.text)
        
        # Calculate milliseconds
        # quarter note = self.divisions divisions
        # tempo is in quarter notes per minute
        quarter_note_ms = 60000 / self.tempo
        duration_ms = (duration_divs / self.divisions) * quarter_note_ms
        
        # Round to nearest standard duration for consistency
        return int(round(duration_ms))
    
    def _flat_to_sharp(self, note: str) -> str:
        """Convert flat to equivalent sharp"""
        flat_to_sharp = {
            'B': 'AS',  # Bb = A#
            'E': 'DS',  # Eb = D#
            'A': 'GS',  # Ab = G#
            'D': 'CS',  # Db = C#
            'G': 'FS',  # Gb = F#
        }
        return flat_to_sharp.get(note, note)


def sanitize_identifier(name: str) -> str:
    """Convert filename to valid C identifier"""
    # Remove extension
    name = name.replace('.musicxml', '')
    # Replace hyphens and spaces with underscores
    name = name.replace('-', '_').replace(' ', '_')
    # Remove other invalid characters
    name = ''.join(c if c.isalnum() or c == '_' else '_' for c in name)
    # Ensure it doesn't start with a number
    if name[0].isdigit():
        name = 'SONG_' + name
    return name.upper()


def generate_melody_enum(songs: Dict) -> str:
    """Generate melody_id_t enum"""
    lines = [
        "typedef enum",
        "{",
        "    MELODY_NONE = 0,",
    ]
    
    for song_name in songs.keys():
        identifier = sanitize_identifier(song_name)
        lines.append(f"    MELODY_{identifier},")
    
    lines.append("    MELODY_TEST_TONE,")
    lines.append("    MELODY_COUNT")
    lines.append("} melody_id_t;")
    
    return '\n'.join(lines)


def generate_song_data(song_name: str, notes: List[Tuple[int, int]]) -> str:
    """Generate C array for a single song"""
    identifier = sanitize_identifier(song_name)
    
    lines = [
        f"// Song: {song_name} ({len(notes)} notes)",
        f"static const audio_note_t PROGMEM melody_{identifier.lower()}[] = {{",
    ]
    
    for freq, duration in notes:
        lines.append(f"    {{{freq}, {duration}}},")
    
    lines.append("};")
    lines.append("")
    
    return '\n'.join(lines)


def generate_config_array(config: Dict, all_songs: Dict) -> str:
    """Generate song_configs array with defaults for missing configurations"""
    yaml_songs = config.get('songs', {})
    
    # Default values
    DEFAULT_DUTY_CYCLE = 75
    DEFAULT_SPEED = 100
    DEFAULT_TRANSPOSE = 0
    
    lines = [
        "static const song_config_t song_configs[] = {",
        "    [MELODY_NONE] = {50, 100, 0},  // Default fallback",
    ]
    
    for song_name in all_songs.keys():
        identifier = sanitize_identifier(song_name)
        
        # Get configuration from YAML or use defaults
        song_config = yaml_songs.get(song_name, {})
        duty = song_config.get('duty_cycle', DEFAULT_DUTY_CYCLE)
        speed = song_config.get('speed', DEFAULT_SPEED)
        transpose = song_config.get('transpose', DEFAULT_TRANSPOSE)
        
        lines.append(f"    [MELODY_{identifier}] = {{{duty}, {speed}, {transpose}}},  // {song_name}")
    
    lines.append("    [MELODY_TEST_TONE] = {80, 100, 0},  // Test tone")
    lines.append("};")
    
    return '\n'.join(lines)


def generate_enabled_songs(config: Dict, all_songs: Dict) -> str:
    """Generate enabled_songs array - enable all songs by default unless explicitly disabled"""
    yaml_songs = config.get('songs', {})
    
    # Default: enable songs unless explicitly set to false in YAML
    enabled_list = []
    for song_name in all_songs.keys():
        song_config = yaml_songs.get(song_name, {})
        # If song is not in YAML, default to enabled=true
        # If song is in YAML, use the 'enabled' value (default to true if not specified)
        is_enabled = song_config.get('enabled', True)
        if is_enabled:
            enabled_list.append(song_name)
    
    enabled_count = len(enabled_list)
    
    lines = [
        "#if ENABLE_SONG_ROTATION",
        "const melody_id_t enabled_songs[] = {",
    ]
    
    for song_name in enabled_list:
        identifier = sanitize_identifier(song_name)
        lines.append(f"    MELODY_{identifier},")
    
    lines.append("};")
    lines.append(f"const uint8_t ENABLED_SONG_COUNT = {enabled_count};")
    lines.append("#endif")
    
    return '\n'.join(lines)


def generate_get_melody_data(songs: Dict) -> str:
    """Generate get_melody_data() function"""
    lines = [
        "const audio_note_t *get_melody_data(melody_id_t melody_id, uint8_t *note_count)",
        "{",
        "    switch (melody_id) {",
    ]
    
    for song_name in songs.keys():
        identifier = sanitize_identifier(song_name)
        var_name = identifier.lower()
        lines.append(f"    case MELODY_{identifier}:")
        lines.append(f"        *note_count = sizeof(melody_{var_name}) / sizeof(audio_note_t);")
        lines.append(f"        return melody_{var_name};")
        lines.append("")
    
    lines.append("    case MELODY_TEST_TONE:")
    lines.append("        *note_count = sizeof(melody_test_tone) / sizeof(audio_note_t);")
    lines.append("        return melody_test_tone;")
    lines.append("")
    lines.append("    case MELODY_NONE:")
    lines.append("    default:")
    lines.append("        *note_count = 0;")
    lines.append("        return NULL;")
    lines.append("    }")
    lines.append("}")
    
    return '\n'.join(lines)


def generate_hardware_config(config: Dict) -> str:
    """Generate hardware configuration header content from YAML"""
    hardware = config.get('hardware', {})
    
    # Mapping from YAML keys to C define names with fallback defaults
    param_mapping = {
        # LED system
        'led_brightness_default': ('LED_BRIGHTNESS_DEFAULT', 30),
        'candle_flicker_speed': ('CANDLE_FLICKER_SPEED', 150),  
        'candle_flicker_intensity': ('CANDLE_FLICKER_INTENSITY', 70),
        
        # Audio system
        'audio_max_frequency': ('AUDIO_MAX_FREQUENCY', 4000),
        'audio_min_frequency': ('AUDIO_MIN_FREQUENCY', 100),
        'audio_default_volume': ('AUDIO_DEFAULT_VOLUME', 128),
        'song_cooldown': ('SONG_COOLDOWN_MS', 3000),
        'note_separation': ('AUDIO_NOTE_GAP_MS', 50),
        'startup_melody': ('STARTUP_MELODY_ENABLED', False),
        
        # Sensor settings
        'breath_sensitivity': ('BREATH_SENSITIVITY', 1),
        'sensor_update_rate': ('SENSOR_UPDATE_RATE_MS', 5),
    }
    
    # Get values from YAML or use defaults
    defines = []
    
    # Group by category for better organization
    categories = {
        'Sensor Configuration': ['breath_sensitivity', 'sensor_update_rate'],
        'LED Configuration': ['led_brightness_default', 'candle_flicker_speed', 'candle_flicker_intensity'],
        'Audio Configuration': ['audio_max_frequency', 'audio_min_frequency', 'audio_default_volume', 'song_cooldown', 'note_separation', 'startup_melody']
    }
    
    for category, params in categories.items():
        defines.append(f'// {category}')
        for param in params:
            if param in param_mapping:
                c_define, default = param_mapping[param]
                yaml_value = hardware.get(param, default)
                
                # Handle boolean values
                if isinstance(yaml_value, bool):
                    c_value = 'true' if yaml_value else 'false'
                else:
                    c_value = str(yaml_value)
                
                defines.append(f'#define {c_define} {c_value}')
        defines.append('')
    
    # Remove the last empty line
    if defines and defines[-1] == '':
        defines.pop()
    
    define_block = '\n'.join(defines)

    return f'''#ifndef HARDWARE_CONFIG_GENERATED_H_
#define HARDWARE_CONFIG_GENERATED_H_

/*
 * hardware_config_generated.h - Auto-generated hardware configuration
 * Generated by scripts/generate_audio_code.py from config.yaml
 * 
 * DO NOT EDIT THIS FILE MANUALLY!
 * Modify config.yaml hardware section instead.
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 */

// ============================================================================
// HARDWARE CONFIGURATION - From config.yaml
// ============================================================================

{define_block}

#endif // HARDWARE_CONFIG_GENERATED_H_
'''


def main():
    """Main code generation function"""
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    config_file = project_root / 'config.yaml'
    songs_dir = project_root / 'songs'
    output_dir = project_root / 'lib' / 'Audio'
    
    # Load configuration
    print(f"Loading configuration from {config_file}")
    with open(config_file, 'r') as f:
        config = yaml.safe_load(f)
    
    # Scan all MusicXML files in the songs directory
    print(f"\nScanning {songs_dir} for MusicXML files...")
    musicxml_files = list(songs_dir.glob('*.musicxml'))
    
    if not musicxml_files:
        print(f"Error: No MusicXML files found in {songs_dir}", file=sys.stderr)
        return 1
    
    print(f"Found {len(musicxml_files)} MusicXML file(s)")
    
    # Parse all MusicXML files
    song_data = {}
    for musicxml_file in sorted(musicxml_files):
        # Extract song name from filename (without .musicxml extension)
        song_name = musicxml_file.stem
        
        print(f"Parsing {song_name}...")
        parser = MusicXMLParser(musicxml_file)
        notes = parser.parse()
        song_data[song_name] = notes
        print(f"  -> {len(notes)} notes extracted")
    
    # Generate hardware configuration file
    print(f"\nGenerating hardware configuration...")
    hardware_config_content = generate_hardware_config(config)
    hardware_config_file = project_root / 'config' / 'hardware_config_generated.h'
    with open(hardware_config_file, 'w') as f:
        f.write(hardware_config_content)
    print(f"  Generated: {hardware_config_file}")
    
    # Generate song header file
    print("Generating audio_songs_generated.h...")
    header_content = f'''/*
 * audio_songs_generated.h - Auto-generated song definitions
 * Generated by scripts/generate_audio_code.py
 * 
 * DO NOT EDIT THIS FILE MANUALLY!
 * Modify config.yaml and MusicXML files in songs/ folder instead.
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 */

#ifndef AUDIO_SONGS_GENERATED_H_
#define AUDIO_SONGS_GENERATED_H_

#include <stdint.h>

// Note: This file is included from audio.h AFTER audio_note_t and song_config_t are defined
// Do not include audio.h here to avoid circular dependency

// ============================================================================
// MELODY ENUMERATION
// ============================================================================

{generate_melody_enum(song_data)}

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

const audio_note_t *get_melody_data(melody_id_t melody_id, uint8_t *note_count);
const song_config_t *get_song_config(melody_id_t melody_id);

// ============================================================================
// ENABLED SONGS DECLARATIONS
// ============================================================================

#if ENABLE_SONG_ROTATION
extern const melody_id_t enabled_songs[];
extern const uint8_t ENABLED_SONG_COUNT;
#endif

#endif // AUDIO_SONGS_GENERATED_H_
'''
    
    header_file = output_dir / 'audio_songs_generated.h'
    with open(header_file, 'w') as f:
        f.write(header_content)
    
    # Generate implementation file
    print("Generating audio_songs_generated.cpp...")
    
    # Generate all song data
    all_song_data = []
    for song_name, notes in song_data.items():
        all_song_data.append(generate_song_data(song_name, notes))
    
    # Add test tone
    all_song_data.append('''// Test tone: 5-second A4 note
static const audio_note_t PROGMEM melody_test_tone[] = {
    {440, 5000},  // A4 for 5 seconds
};
''')
    
    cpp_content = f'''/*
 * audio_songs_generated.cpp - Auto-generated song data
 * Generated by scripts/generate_audio_code.py
 * 
 * DO NOT EDIT THIS FILE MANUALLY!
 * Modify config.yaml and MusicXML files in songs/ folder instead.
 *
 * Copyright (c) 2025 monkeyToneCircuits
 * Licensed under CC-BY-NC 4.0
 */

#include "audio.h"
#include "audio_songs_generated.h"
#include "../../config/config.h"
#include <avr/pgmspace.h>

// ============================================================================
// MELODY DATA - Generated from MusicXML files
// All melody data stored in PROGMEM to save RAM
// ============================================================================

{chr(10).join(all_song_data)}

// ============================================================================
// SONG CONFIGURATIONS - From config.yaml (with defaults)
// ============================================================================

{generate_config_array(config, song_data)}

// ============================================================================
// ENABLED SONGS LIST - From config.yaml (enabled by default)
// ============================================================================

{generate_enabled_songs(config, song_data)}

// ============================================================================
// ACCESSOR FUNCTIONS
// ============================================================================

{generate_get_melody_data(song_data)}

const song_config_t *get_song_config(melody_id_t melody_id)
{{
    if (melody_id < MELODY_COUNT) {{
        return &song_configs[melody_id];
    }}
    return &song_configs[MELODY_NONE];
}}
'''
    
    cpp_file = output_dir / 'audio_songs_generated.cpp'
    with open(cpp_file, 'w') as f:
        f.write(cpp_content)
    
    print(f"\nCode generation complete!")
    print(f"  Generated: {hardware_config_file}")
    print(f"  Generated: {header_file}")
    print(f"  Generated: {cpp_file}")
    print(f"  Total songs: {len(song_data)}")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
