#ifndef SEQ24_EVENT_H
#define SEQ24_EVENT_H

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
 * \file          event.h
 *
 *  This module declares/defines the base class for MIDI events.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-07-26
 * \license       GNU GPLv2 or above
 *
 */

#include <stdio.h>
#include "globals.h"

/**
 *  This highest bit of the status byte is always 1.
 */

const unsigned char  EVENT_STATUS_BIT       = 0x80;

/**
 *  The following events are channel messages.
 */

const unsigned char  EVENT_NOTE_OFF         = 0x80;
const unsigned char  EVENT_NOTE_ON          = 0x90;
const unsigned char  EVENT_AFTERTOUCH       = 0xA0;
const unsigned char  EVENT_CONTROL_CHANGE   = 0xB0;
const unsigned char  EVENT_PROGRAM_CHANGE   = 0xC0;
const unsigned char  EVENT_CHANNEL_PRESSURE = 0xD0;
const unsigned char  EVENT_PITCH_WHEEL      = 0xE0;

/**
 *  The following events have no channel.
 */

const unsigned char  EVENT_CLEAR_CHAN_MASK  = 0xF0;
const unsigned char  EVENT_MIDI_QUAR_FRAME  = 0xF1;     // not used
const unsigned char  EVENT_MIDI_SONG_POS    = 0xF2;
const unsigned char  EVENT_MIDI_SONG_SELECT = 0xF3;     // not used
const unsigned char  EVENT_MIDI_TUNE_SELECT = 0xF6;     // not used
const unsigned char  EVENT_MIDI_CLOCK       = 0xF8;
const unsigned char  EVENT_MIDI_START       = 0xFA;
const unsigned char  EVENT_MIDI_CONTINUE    = 0xFB;
const unsigned char  EVENT_MIDI_STOP        = 0xFC;
const unsigned char  EVENT_MIDI_ACTIVE_SENS = 0xFE;     // not used
const unsigned char  EVENT_MIDI_RESET       = 0xFF;     // not used

/**
 *  A MIDI System Exclusive (SYSEX) message starts with F0, followed
 *  by the manufacturer ID (how many? bytes), a number of data bytes, and
 *  ended by an F7.
 */

const unsigned char  EVENT_SYSEX            = 0xF0;
const unsigned char  EVENT_SYSEX_END        = 0xF7;

/**
 *  Provides events for management of MIDI events.
 *
 *  A MIDI event consists of 3 bytes:
 *
 *      -#  Status byte, 1sssnnn, where the sss bits specify the type of
 *          message, and the nnnn bits denote the channel number.
 *          The status byte always starts with 0.
 *      -#  The first data byte, 0xxxxxxx, where the data byte always
 *          start with 0, and the xxxxxxx values range from 0 to 127.
 *      -#  The second data byte, 0xxxxxxx.
 *
 *  This class may have too many member functions.
 */

class event
{

    friend class sequence;

private:

    /*
     *  Provides the MIDI timestamp in ticks.
     */

    unsigned long m_timestamp;

    /**
     *  This is status byte without the channel.  The channel will be
     *  appended on the MIDI bus.  The high nibble = type of event; The
     *  low nibble = channel.  Bit 7 is present in all status bytes.
     */

    unsigned char m_status;

    /**
     *  The two bytes of data for the MIDI event.  Remember that the
     *  most-significant bit of a data byte is always 0.
     */

    unsigned char m_data[2];

    /**
     *  Points to the data buffer for SYSEX messages.
     *
     *  This really ought to be a Boost or STD scoped pointer.
     */

    unsigned char * m_sysex;

    /**
     *  Gives the size of the SYSEX message.
     */

    long m_size;

    /**
     *  This event is used to link Note Ons and Offs together.
     */

    event * m_linked;

    /**
     *  Indicates that a link is...
     */

    bool m_has_link;

    /**
     *  Answers the question "is this event selected in editing */
    bool m_selected;

    /**
     *  Answers the question "is this event marked in processing */
    bool m_marked;

    /**
     *  Answers the question "is this event being painted */
    bool m_painted;

public:

    event ();
    ~event ();

    /*
     * Operator overloads
     */

    bool operator > (const event & rhsevent);
    bool operator < (const event & rhsevent);

    bool operator <= (unsigned long rhslong);
    bool operator > (unsigned long rhslong);

    void set_timestamp (unsigned long time);
    long get_timestamp ();
    void mod_timestamp (unsigned long a_mod);

    void set_status (char status);
    unsigned char get_status ();
    void set_data (char D1);
    void set_data (char D1, char D2);
    void get_data (unsigned char * D0, unsigned char * D1);
    void increment_data1 (void);
    void decrement_data1 (void);
    void increment_data2 (void);
    void decrement_data2 (void);

    void start_sysex (void);
    bool append_sysex (unsigned char *a_data, long size);
    unsigned char * get_sysex (void);

    void set_note (char a_note);

    void set_size (long a_size);
    long get_size (void);

    void link (event * event);
    event * get_linked ();
    bool is_linked ();
    void clear_link ();

    void paint ();
    void unpaint ();
    bool is_painted ();

    void mark ();
    void unmark ();
    bool is_marked ();

    void select ();
    void unselect ();
    bool is_selected ();

    void make_clock ();                // set status to MIDI clock

    unsigned char get_note ();         // gets note if it is note on/off
    unsigned char get_note_velocity ();
    void set_note_velocity (int a_vel);

    bool is_note_on ();                // returns true if status is set
    bool is_note_off ();

    void print ();

private:

    /**
     *  This function is used in sorting.  Sorting what?
     */

    int get_rank() const;
};

#endif   // SEQ24_EVENT_H

/*
 * event.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
