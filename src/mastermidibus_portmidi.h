#ifndef SEQ24_MASTERMIDIBUS_PORTMIDI_H
#define SEQ24_MASTERMIDIBUS_PORTMIDI_H

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
 * \file          mastermidibus_portmidi.h
 *
 *  This module declares/defines the base class for MIDI I/O for Windows.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-27
 * \license       GNU GPLv2 or above
 *
 *  The mastermidibus_portmidi module is the Windows version of the
 *  mastermidibus module.  There's almost enough commonality to be worth
 *  creating a base class for both classes. We moved the mastermidibus
 *  class into its own module, this one.
 */

#include "midibus_common.h"

#ifdef PLATFORM_WINDOWS                // covers this whole module

/**
 *  The class that "supervises" all of the midibus objects?
 */

class mastermidibus
{
private:

    /**
     *  The number of output busses.
     */

    int m_num_out_buses;

    /**
     *  The number of input busses.
     */

    int m_num_in_buses;

    /**
     *  Output MIDI busses.
     */

    midibus * m_buses_out[c_max_busses];

    /**
     *  Input MIDI busses.
     */

    midibus * m_buses_in[c_max_busses];

    /**
     *  MIDI buss announcer?
     */

    midibus * m_bus_announce;

    /**
     *  Active output MIDI busses.
     */

    bool m_buses_out_active[c_max_busses];

    /**
     *  Active input MIDI busses.
     */

    bool m_buses_in_active[c_max_busses];

    /**
     *  Output MIDI buss initialization.
     */

    bool m_buses_out_init[c_max_busses];

    /**
     *  Input MIDI buss initialization.
     */

    bool m_buses_in_init[c_max_busses];

    /**
     *  Clock initialization.
     */

    clock_e m_init_clock[c_max_busses];

    /**
     *  Input initialization?
     */

    bool m_init_input[c_max_busses];

    /**
     *  The ID of the MIDI queue.
     */

    int m_queue;

    /**
     *  Resolution in parts per quarter note.
     */

    int m_ppqn;

    /**
     *  BPM (beats per minute)
     */

    int m_bpm;

    /**
     *  The number of descriptors for polling.
     */

    int  m_num_poll_descriptors;

    /**
     *  Points to the list of descriptors for polling.
     */

    struct pollfd * m_poll_descriptors;

    /**
     *  For dumping MIDI input to a sequence for recording.
     */

    bool m_dumping_input;

    /**
     *  Points to the sequence object.
     */

    sequence * m_seq;

    /**
     *  The locking mutex.
     */

    mutex m_mutex;

public:

    mastermidibus ();
    ~mastermidibus ();

    void init ();

    /**
     * \getter m_num_out_buses
     */

    int get_num_out_buses () const
    {
        return m_num_out_buses;
    }

    /**
     * \getter m_num_in_buses
     */

    int get_num_in_buses () const
    {
        return m_num_in_buses;
    }

    void set_bpm (int a_bpm);
    void set_ppqn (int a_ppqn);

    /**
     * \getter m_bpm
     */

    int get_bpm () const
    {
        return m_bpm;
    }

    /**
     * \getter m_ppqn
     */

    int get_ppqn () const
    {
        return m_ppqn;
    }

    std::string get_midi_out_bus_name (int a_bus);
    std::string get_midi_in_bus_name (int a_bus);
    void print ();
    void flush ();
    void start ();
    void stop ();
    void clock (long a_tick);
    void continue_from (long a_tick);
    void init_clock (long a_tick);
    int poll_for_midi ();
    bool is_more_input ();
    bool get_midi_event (event *a_in);
    void set_sequence_input (bool a_state, sequence *a_seq);

    /**
     * \getter m_dumping_input
     */

    bool is_dumping () const
    {
        return m_dumping_input;
    }

    /**
     * \getter m_seq const
     */

    sequence * get_sequence ()
    {
        return m_seq;
    }

    void sysex (event * a_event);
    void play (unsigned char a_bus, event * a_e24, unsigned char a_channel);
    void set_clock (unsigned char a_bus, clock_e a_clock_type);
    clock_e get_clock (unsigned char a_bus);
    void set_input (unsigned char a_bus, bool a_inputing);
    bool get_input (unsigned char a_bus);

private:

    /**
     *  Mutex lock. Replaced by automutex.

    void lock ()
    {
        m_mutex.lock();
    }
     */

    /**
     *  Mutex unlock. Replaced by automutex.

    void unlock ()
    {
        m_mutex.unlock();
    }
     */

};

#endif  // PLATFORM_WINDOWS

#endif  // SEQ24_MASTERMIDIBUS_PORTMIDI_H

/*
 * mastermidibus_portmidi.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
