#ifndef SEQ24_MIDIBUS_COMMON_H
#define SEQ24_MIDIBUS_COMMON_H

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
 * \file          midibus_common.h
 *
 *  This module declares/defines the elements that are common to the Linux
 *  and Windows implmentations of midibus.
 *
 * \library       sequencer24 application
 * \author        Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-05
 * \license       GNU GPLv2 or above
 *
 */

#include <string>
// #include "platform_macros.h"

// #ifdef PLATFORM_WINDOWS
// #include "configwin32.h"
// #else
// #include "config.h"
// #endif

#include "event.h"
#include "mutex.h"
#include "globals.h"

/*
 * Multiple forward references
 */

class mastermidibus;
class midibus;
class sequence;

/**
 *  Manifest global constants.
 *
 *  These constants were also defined in midibus_portmidi.h, but we made
 *  them common to both implementations here.
 */

const int c_midibus_output_size = 0x100000;     // 1048576
const int c_midibus_input_size =  0x100000;     // 1048576
const int c_midibus_sysex_chunk = 0x100;        //     256

/**
 *  A clock enumeration.  Not sure yet what these mean.
 *
 *  This enumeration Was also defined in midibus_portmidi.h, but we put it
 *  into this common module to avoid duplication.
 *
 * \var e_clock_off
 *
 * \var e_clock_pos
 *
 * \var e_clock_mod
 */

enum clock_e
{
    e_clock_off,
    e_clock_pos,
    e_clock_mod
};

#endif  // SEQ24_MIDIBUS_COMMON_H

/*
 * midibus_common.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
