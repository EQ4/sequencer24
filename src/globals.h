#ifndef SEQ24_GLOBALS_H
#define SEQ24_GLOBALS_H

/*
 *  This file is part of seq24/sequencer24.
 *
 *  seq24 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq24 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq24; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          globals.h
 *
 *  This module declares/defines just some of the global (gasp!) variables
 *  in this application.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-25
 * \updates       2015-08-02
 * \license       GNU GPLv2 or above
 *
 */

#include "easy_macros.h"                // with platform_macros.h, too

#include <string>
#include <gtkmm/main.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/accelkey.h>

using namespace std;                   // will eventually remove this

/**
 *  This collection of global variables describes some facets of the
 *  "Patterns Panel" or "Sequences Window", which is visually presented by
 *  the Gtk::Window-derived class called mainwnd.
 *
 *  The Patterns Panel contains an 8-by-4 grid of "pattern boxes" or
 *  "sequence boxes".  All of the patterns in this grid comprise what is
 *  called a "set" (in the musical sense) or a "screen set".
 *
 * \note
 *    This set of variables might be better off placed in a object that the
 *    mainwnd class and its clients can access a little more safely and with
 *    a lot more clarity for the human reader.
 */

/**
 *  Number of rows in the Patterns Panel.  The current value is 4, and
 *  probably won't change, since other values depend on it.
 */

const int c_mainwnd_rows = 4;

/**
 *  Number of columns in the Patterns Panel.  The current value is 4, and
 *  probably won't change, since other values depend on it.
 */

const int c_mainwnd_cols = 8;

/**
 *  Number of patterns/sequences in the Patterns Panel, also known as a
 *  "set" or "screen set".  This value is 4 x 8 = 32.
 */

const int c_seqs_in_set = c_mainwnd_rows * c_mainwnd_cols;

/**
 *  Number of group-mute tracks that can be support, which is
 *  c_seqs_in_set squared, or 1024.
 */

const int c_gmute_tracks = c_seqs_in_set * c_seqs_in_set;

/**
 *  Maximum number of screen sets that can be supported.  Basically, that
 *  the number of times the Patterns Panel can be filled.  32 sets can be
 *  created.
 */

const int c_max_sets = 32;

/**
 *  The maximum number of patterns supported is given by the number of
 *  patterns supported in the panel (32) times the maximum number of sets
 *  (32), or 1024 patterns.
 *
 *  It is basically defined in the same manner as c_max_sequence..
 */

const int c_total_seqs = c_seqs_in_set * c_max_sets;

/**
 *  Another variable representing the maximum number of patterns/sequences.
 *  It is basically defined in the same manner as c_total_seqs.
 */

const int c_max_sequence = c_mainwnd_rows * c_mainwnd_cols * c_max_sets;

/**
 *  Provides the timing resolution of a MIDI sequencer, known as "pulses
 *  per quarter note.  For this application, 192 is the default, and it
 *  doesn't change.
 */

const int c_ppqn = 192;

/**
 *  Provides the default number BPM (beats per minute), which describes
 *  the overall speed at which the sequencer will play a tune.  The
 *  default value is 120.
 */

const int c_bpm = 120;

/**
 *  Provides the maximum number of MIDI buss definitions supported in the
 *  ~/.seq24usr file.
 */

const int c_maxBuses = 32;

/**
 *  The trigger width in milliseconds.  This value is 4 ms.
 */

const int c_thread_trigger_width_ms = 4;

/**
 *  The trigger lookahead in milliseconds.  This value is 2 ms.
 */

const int c_thread_trigger_lookahead_ms = 2;

/**
 *  Constants for the mainwid class.  The c_text_x and c_text_y constants
 *  help define the "seqarea" size.
 */

const int c_text_x = 6;
const int c_text_y = 12;

/*
 *  Compare these two constants to c_seqarea_seq_x(y) in mainwid.h.
 */

const int c_seqarea_x = c_text_x * 15;
const int c_seqarea_y = c_text_y * 5;

const int c_mainwid_border = 0;
const int c_mainwid_spacing = 2;
const int c_control_height = 0;
const int c_mainwid_x =
(
    (c_seqarea_x + c_mainwid_spacing) * c_mainwnd_cols -
        c_mainwid_spacing + c_mainwid_border * 2
);
const int c_mainwid_y =
(
    (c_seqarea_y + c_mainwid_spacing) * c_mainwnd_rows +
         c_control_height + c_mainwid_border * 2
);

/**
 *  The height of the data-entry area for velocity, aftertouch, and other
 *  controllers, as well as note on and off velocity.  This value looks to
 *  be in pixels.
 */

const int c_dataarea_y = 128;

/**
 *  The width of the 'bar', presumably the line that ends a measure, in
 *  pixels.
 */

const int c_data_x = 2;

/**
 *  The dimensions of each key of the virtual keyboard at the left of the
 *  piano roll.
 */

const int c_key_x = 16;
const int c_key_y = 8;

/**
 *  The number of MIDI keys, as well as keys in the virtual keyboard.
 */

const int c_num_keys = 128;

/**
 *  The dimensions and offset of the virtual keyboard at the left of the
 *  piano roll.
 */

const int c_keyarea_y = c_key_y * c_num_keys + 1;
const int c_keyarea_x = 36;
const int c_keyoffset_x = c_keyarea_x - c_key_x;


/**
 *  The height of the piano roll is the same as the height of the virtual
 *  keyboard area.
 */

const int c_rollarea_y = c_keyarea_y;

/**
 *  The height of the events bar, which shows the little squares that
 *  represent the position of each event.
 */

const int c_eventarea_y = 16;

/**
 *  The dimensions of the little squares that represent the position of
 *  each event.
 */

const int c_eventevent_y = 10;
const int c_eventevent_x = 5;

/**
 *  The time scale window on top of the piano roll, in pixels.
 */

const int c_timearea_y = 18;

/**
 *  The number of MIDI notes in what?  This value is used in the sequence
 *  module.  It looks like it is the maximum number of notes that
 *  seq24/sequencer24 can have playing at one time.  In other words, only
 *  256 simultaneously-playing notes can be managed.
 */

const int c_midi_notes = 256;

/**
 *  Provides the default string for the name of a pattern or sequence.
 */

const std::string c_dummy("Untitled");

/**
 *  Provides the maximum size of sequence, and the default size.
 *  It is the maximum number of beats in a sequence.
 */

const int c_maxbeats = 0xFFFF;

/**
 *  Provides midifile tags.  Need to find out what these are for.
 */

const unsigned long c_midibus =         0x24240001;
const unsigned long c_midich =          0x24240002;
const unsigned long c_midiclocks =      0x24240003;
const unsigned long c_triggers =        0x24240004;
const unsigned long c_notes =           0x24240005;
const unsigned long c_timesig =         0x24240006;
const unsigned long c_bpmtag =          0x24240007;
const unsigned long c_triggers_new =    0x24240008;
const unsigned long c_midictrl =        0x24240010;

// Comment: Not sure why we went to 10 above, this might need a different
//          value.

const unsigned long c_mutegroups =      0x24240009;

/**
 *  Provides the various font sizes for the default font.
 */

const char c_font_6_12[] = "-*-fixed-medium-r-*--12-*-*-*-*-*-*";
const char c_font_8_13[] = "-*-fixed-medium-r-*--13-*-*-*-*-*-*";
const char c_font_5_7[]  = "-*-fixed-medium-r-*--7-*-*-*-*-*-*";

/**
 *  Values used in the menu to tell setState() what to do.
 */

const int c_adding = 0;
const int c_normal = 1;
const int c_paste  = 2;

/**
 *  Provides the redraw time when recording, in ms.  Can Windows actually
 *  draw faster? :-D
 */

#ifdef PLATFORM_WINDOWS
const int c_redraw_ms = 20;
#else
const int c_redraw_ms = 40;
#endif

/**
 *  Provides constants for the perform (performance) editor.
 */

const int c_names_x = 6 * 24;
const int c_names_y = 22;
const int c_perf_scale_x = 32;  // units are ticks per pixel

/**
 *  Provides the maximum number of instruments that can be defined in the
 *  ~/.seq24usr file.
 */

const int c_max_instruments = 64;

/**
 *  These global values seemed to be use mainly in the options,
 *  optionsfile, perform, seq24, and userfile modules.
 */

extern bool global_showmidi;
extern bool global_priority;
extern bool global_stats;
extern bool global_pass_sysex;
extern bool global_with_jack_transport;
extern bool global_with_jack_master;
extern bool global_with_jack_master_cond;
extern bool global_jack_start_mode;
extern bool global_manual_alsa_ports;

extern Glib::ustring global_filename;
extern Glib::ustring global_jack_session_uuid;
extern Glib::ustring last_used_dir;

extern bool is_pattern_playing;
extern bool global_print_keys;

/**
 *  This structure corresponds to [user-midi-bus-0] definitions in the
 *  ~/.seq24usr file.
 */

struct user_midi_bus_definition
{
    std::string alias;
    int instrument[16];
};

/**
 *  This structure corresponds to [user-instrument-0] definitions in the
 *  ~/.seq24usr file.
 */

struct user_instrument_definition
{
    std::string instrument;
    bool controllers_active[128];
    std::string controllers[128];
};

/**
 *  Global arrays.
 */

extern user_midi_bus_definition
    global_user_midi_bus_definitions[c_maxBuses];

extern user_instrument_definition
    global_user_instrument_definitions[c_max_instruments];

/**
 *  Corresponds to the small number of musical scales that the application
 *  can handle.  Scales can be shown in the piano roll as gray bars for
 *  reference purposes.
 */

enum c_music_scales
{
    c_scale_off,
    c_scale_major,
    c_scale_minor,
    c_scale_size            // a "maximum" or "size of set" value.
};

/**
 *  Each value in the kind of scale is denoted by a true value in these
 *  arrays.
 */

const bool c_scales_policy[c_scale_size][12] =
{
    /* off = chromatic */
    {
        true, true, true, true, true, true,
        true, true, true, true, true, true
    },

    /* major */
    {
        true, false, true, false, true, true,
        false, true, false, true, false, true
    },

    /* minor */
    {
        true, false, true, true, false, true,
        false, true, true, false, true, false
    },
};

const int c_scales_transpose_up[c_scale_size][12] =
{
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},      /* off = chromatic */
    { 2, 0, 2, 0, 1, 2, 0, 2, 0, 2, 0, 1},      /* major */
    { 2, 0, 1, 2, 0, 2, 0, 1, 2, 0, 2, 0},      /* minor */
};

const int c_scales_transpose_dn[c_scale_size][12] =
{
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},  /* off = chromatic */
    { -1, 0, -2, 0, -2, -1, 0, -2, 0, -2, 0, -2},       /* major */
    { -2, 0, -2, -1, 0, -2, 0, -2, -1, 0, -2, 0},       /* minor */
};

const int c_scales_symbol[c_scale_size][12] =
{
    { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32},      /* off = chromatic */
    { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32},      /* major */
    { 32, 32, 32, 32, 32, 32, 32, 32, 129, 128, 129, 128},  /* minor */
};

// up 128
// down 129

/**
 *  The names of the supported scales.
 */

const char c_scales_text[c_scale_size][6] =
{
    "Off",
    "Major",
    "Minor"
};

/**
 *  Provides the entries for the Key dropdown menu in the Pattern Editor
 *  window.
 */

const char c_key_text[][3] =
{
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B"
};

/**
 *  Provides the entries for the Interval dropdown menu in the Pattern Editor
 *  window.
 */

const char c_interval_text[][3] =
{
    "P1",
    "m2",
    "M2",
    "m3",
    "M3",
    "P4",
    "TT",
    "P5",
    "m6",
    "M6",
    "m7",
    "M7",
    "P8",
    "m9",
    "M9",
    ""
};

/**
 *  Provides the entries for the Chord dropdown menu in the Pattern Editor
 *  window.  However, I have not seen this menu in the GUI!
 */

const char c_chord_text[][5] =
{
    "I",
    "II",
    "III",
    "IV",
    "V",
    "VI",
    "VII",
    "VIII"
};

/**
 *  Mouse actions, for the Pattern Editor.  Be sure to update seq24-doc
 *  to use this nomenclature.
 */

enum mouse_action_e
{
    e_action_select,
    e_action_draw,
    e_action_grow
};

/**
 *  Provides codes for the mouse-handling used by the application.
 */

enum interaction_method_e
{
    e_seq24_interaction,
    e_fruity_interaction,
    e_number_of_interactions    // keep this one last... a "size" value
};

/**
 *  Provides names for the mouse-handling used by the application.
 */

const char * const c_interaction_method_names[] =
{
    "seq24",
    "fruity",
    NULL
};

/**
 *  Provides descriptions for the mouse-handling used by the application.
 */

const char* const c_interaction_method_descs[] =
{
    "original seq24 method",
    "similar to a certain fruity sequencer we like",
    NULL
};

/**
 *
 */

extern interaction_method_e global_interactionmethod;

#endif  // SEQ24_GLOBALS_H

/*
 * globals.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
