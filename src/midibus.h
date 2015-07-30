#ifndef SEQ24_MIDIBUS_H
#define SEQ24_MIDIBUS_H

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
 * \file          midibus.h
 *
 *  This module declares/defines the base class for MIDI I/O under Linux.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-07-30
 * \license       GNU GPLv2 or above
 *
 *  The midibus module is the Linux version of the midibus module.
 *  There's almost enough commonality to be worth creating a base class
 *  for both classes.
 *
 *  We moved the mastermidibus class into its own module.
 */

#include "midibus_common.h"

#if HAVE_LIBASOUND                     // covers this whole module

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

/**
 *  Provides a class for handling the MIDI buss on Linux.
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

    int m_id;

    /**
     *  The type of clock to use.
     */

    clock_e m_clock_type;

    /**
     *  TBD
     */

    bool m_inputing;

    /**
     *  ALSA sequencer client handle.
     */

    snd_seq_t * const m_seq;

    /**
     *  Destination address of client.
     */

    const int m_dest_addr_client;

    /**
     *  Destination port of client.
     */

    const int m_dest_addr_port;

    /**
     *  Local address of client.
     */

    const int m_local_addr_client;

    /**
     *  Local port of client.
     */

    int m_local_addr_port;

    /**
     *  Another ID of the MIDI queue?
     */

    int m_queue;

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

public:

    midibus
    (
        int a_localclient,
        int a_destclient,
        int a_destport,
        snd_seq_t * a_seq,
        const char * a_client_name,
        const char * a_port_name,
        int a_id,
        int a_queue
    );
    midibus
    (
        int a_localclient,
        snd_seq_t * a_seq,
        int a_id,
        int a_queue
    );

    ~midibus ();

    bool init_out ();
    bool init_in ();
    bool deinit_in ();
    bool init_out_sub ();
    bool init_in_sub ();
    void print ();
    std::string get_name ();
    int get_id ();

    void play (event * a_e24, unsigned char a_channel);
    void sysex (event * a_e24);

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

    /**
     *  \getter m_dest_addr_client
     *      The address of client.
     */

    int get_client ()
    {
        return m_dest_addr_client;
    }

    /**
     *  \getter m_dest_addr_port
     */

    int get_port ()
    {
        return m_dest_addr_port;
    };

    static void set_clock_mod (int a_clock_mod);
    static int get_clock_mod  ();

private:

    void lock ();
    void unlock ();

};

#endif  // HAVE_LIBASOUND

#endif  // SEQ24_MIDIBUS_H

/*
 * midibus.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
