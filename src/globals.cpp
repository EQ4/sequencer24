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
 * \file          globals.cpp
 *
 *  This module declares/defines just some of the global (gasp!) variables
 *  in this application.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-08-07
 * \updates       2015-08-27
 * \license       GNU GPLv2 or above
 *
 */

#include "globals.h"

/*
 * Most of these variables were declared and used in other modules, but
 * are now consolidated here.
 */

bool global_legacy_format = false;     /* new 2015-08-16 */
bool global_lash_support = false;      /* new 2015-08-27 */
bool global_manual_alsa_ports = false;
bool global_showmidi = false;
bool global_priority = false;
bool global_device_ignore = false;
int global_device_ignore_num = 0;
bool global_stats = false;
bool global_pass_sysex = false;
std::string global_filename = "";
std::string global_last_used_dir = "/";
std::string global_config_filename = ".seq24rc";
std::string global_user_filename = ".seq24usr";
bool global_print_keys = false;
bool global_is_pattern_playing = false;

bool global_with_jack_transport = false;
bool global_with_jack_master = false;
bool global_with_jack_master_cond = false;
bool global_jack_start_mode = true;
std::string global_jack_session_uuid = "";

interaction_method_e global_interactionmethod = e_seq24_interaction;
user_midi_bus_definition   global_user_midi_bus_definitions[c_max_busses];
user_instrument_definition global_user_instrument_definitions[c_max_instruments];

/*
 * globals.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
