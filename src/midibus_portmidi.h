#ifndef SEQ24_MIDIBUS_PORTMIDI_H
#define SEQ24_MIDIBUS_PORTMIDI_H

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
 * \file          midibus_portmidi.h
 *
 *  This module declares/defines the base class for MIDI I/O for Windows.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-07-29
 * \license       GNU GPLv2 or above
 *
 *  The midibus_portmidi module is the Windows version of the midibus
 *  module.  There's almost enough commonality to be worth creating a base
 *  class for both classes, and it might be nice to put the mastermidibus
 *  classes into their own modules.
 */

#include "midibus_common.h"

#ifdef PLATFORM_WINDOWS                // covers this whole module

/**
 *  This class implements with Windows version of the midibus object.
 */

class midibus
{
    /**
     *  The master MIDI bus sets up the buss.
     */

    friend class mastermidibus;

private:

    /**
     *  TBD
     */

    static int m_clock_mod;

    /**
     *  The ID of the midibus object.
     */

    char m_id;

    /**
     *  TBD
     */

    char m_pm_num;

    /**
     *  The type of clock to use.
     */

    clock_e m_clock_type;

    /**
     *  TBD
     */

    bool m_inputing;

    /**
     *  The name of the MIDI buss.
     */

    std::string m_name;

    /**
     *  The last (most recent?  final?) tick.
     */

    long m_lasttick;

    /**
     *  Locking mutex.
     */

    mutex m_mutex;

    /**
     *  The PortMidiStream for the Windows implementation.
     */

    PortMidiStream * m_pms;

public:

    midibus (char a_id, char a_pm_num, const char * a_client_name);
    midibus(char a_id, int a_queue);

    ~midibus ();

    bool init_out ();
    bool init_in ();
    void print ();
    std::string get_name ();
    int get_id ();

    void play(event * a_e24, unsigned char a_channel);
    void sysex(event * a_e24);

    int poll_for_midi ();

    /*
     * Clock functions
     */

    void start ();
    void stop ();
    void clock (long a_tick);
    void continue_from (long a_tick);
    void init_clock (long a_tick);
    void set_clock (clock_e a_clocking);
    clock_e get_clock ();

    /**
     *  Input functions
     */

    void set_input (bool a_inputing);
    bool get_input ();
    void flush ();


    static void set_clock_mod(int a_clock_mod);
    static int get_clock_mod ();

private:

    void lock();
    void unlock();

};

#endif  // PLATFORM_WINDOWS

#endif  // SEQ24_MIDIBUS_PORTMIDI_H

/*
 * midibus_portmidi.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
