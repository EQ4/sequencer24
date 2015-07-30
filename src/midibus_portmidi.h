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

    midibus * m_buses_out[c_maxBuses];

    /**
     *  Input MIDI busses.
     */

    midibus * m_buses_in[c_maxBuses];

    /**
     *  MIDI buss announcer?
     */

    midibus * m_bus_announce;

    /**
     *  Active output MIDI busses.
     */

    bool m_buses_out_active[c_maxBuses];

    /**
     *  Active input MIDI busses.
     */

    bool m_buses_in_active[c_maxBuses];

    /**
     *  Output MIDI buss initialization.
     */

    bool m_buses_out_init[c_maxBuses];

    /**
     *  Input MIDI buss initialization.
     */

    bool m_buses_in_init[c_maxBuses];

    /**
     *  Clock initialization.
     */

    clock_e m_init_clock[c_maxBuses];

    /**
     *  Input initialization?
     */

    bool m_init_input[c_maxBuses];

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

    int get_num_out_buses ();
    int get_num_in_buses ();

    void set_bpm (int a_bpm);
    void set_ppqn (int a_ppqn);

    /**
     * \getter m_bpm
     */

    int get_bpm ()
    {
        return m_bpm;
    }

    /**
     * \getter m_ppqn
     */

    int get_ppqn ()
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

    bool is_dumping ()
    {
        return m_dumping_input;
    }

    /**
     * \getter m_seq
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

    void lock ();
    void unlock ();

};

#endif  // PLATFORM_WINDOWS

#endif  // SEQ24_MIDIBUS_PORTMIDI_H

/*
 * midibus_portmidi.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
