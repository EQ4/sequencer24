/*
 *
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
 *
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

#include "easy_macros.h"
#include "event.h"
#include "string.h"

/**
 *  This constructor simply initializes all of the class members.
 */

event::event()
 :
    m_timestamp (0),
    m_status    (EVENT_NOTE_OFF),
    m_data      (),
    m_sysex     (nullptr),
    m_size      (0),
    m_linked    (nullptr),
    m_has_link  (false),
    m_selected  (false),
    m_marked    (false),
    m_painted   (false)
{
    m_data[0] = 0;
    m_data[1] = 0;
}

/**
 *  This destructor explicitly deletes m_sysex and sets it to null.
 */

event::~event()
{
    if (not_nullptr(m_sysex))
        delete [] m_sysex;

    m_sysex = nullptr;
}

/**
 *  If the current timestamp equal the event's timestamp, then this
 *  function returns true if the current rank is greater than the event's
 *  rank.
 *
 *  Otherwise, it returns true if the current timestamp is greater than
 *  the event's timestamp.
 */

bool
event::operator > (const event & a_rhsevent)
{
    if (m_timestamp == a_rhsevent.m_timestamp)
    {
        return get_rank() > a_rhsevent.get_rank();
    }
    else
    {
        return m_timestamp > a_rhsevent.m_timestamp;
    }
}

/**
 *  If the current timestamp equal the event's timestamp, then this
 *  function returns true if the current rank is less than the event's
 *  rank.
 *
 *  Otherwise, it returns true if the current timestamp is less than
 *  the event's timestamp.
 */

bool
event::operator < (const event & a_rhsevent)
{
    if (m_timestamp == a_rhsevent.m_timestamp)
    {
        return (get_rank() < a_rhsevent.get_rank());
    }
    else
    {
        return (m_timestamp < a_rhsevent.m_timestamp);
    }
}

/**
 *  Returns true if the current timestamp is less than or equal to the
 *  given value.
 *
 *  Note that this function doesn't really fit in with the operator > and
 *  operator < functions that take an event argument, but it is the
 *  complementary function for operator > taking a long argument..
 */

bool
event::operator <= (unsigned long a_rhslong)
{
    return m_timestamp <= a_rhslong;
}

/**
 *  Returns true if the current timestamp is greater than the given value.
 */

bool
event::operator > (unsigned long a_rhslong)
{
    return m_timestamp > a_rhslong;
}

/**
 * \getter m_timestamp
 */

long
event::get_timestamp()
{
    return m_timestamp;
}

/**
 * \setter m_timestamp
 */

void
event::set_timestamp (unsigned long a_time)
{
    m_timestamp = a_time;
}

/**
 *  Calculates the value of the timestamp modulo the given parameter.
 *
 * \param a_mod
 *      The value to mod the timestamp against.
 *
 * \return
 *      Returns a value ranging from 0 to a_mod-1.
 */

void
event::mod_timestamp (unsigned long a_mod)
{
    m_timestamp %= a_mod;
}

/**
 *  Sets the m_status member to the value of a_status.  If a_status is a
 *  non-channel event, then the channel portion of the status is cleared.
 */

void
event::set_status (char a_status)
{
    if ((unsigned char) a_status >= 0xF0)
    {
        m_status = (char) a_status;
    }
    else
    {
        /*
         * Do a bitwise AND to clear the channel portion of the status.
         */

        m_status = (char) (a_status & EVENT_CLEAR_CHAN_MASK);
    }
}

/**
 *  Sets m_status to EVENT_MIDI_CLOCK;
 */

void
event::make_clock()
{
    m_status = (unsigned char) EVENT_MIDI_CLOCK;
}

/**
 *  Clears the most-significant-bit of the a_D1 parameter, and sets it
 *  into the first byte of m_data.
 */

void
event::set_data (char a_D1)
{
    m_data[0] = a_D1 & 0x7F;
}

/**
 *  Clears the most-significant-bit of both parameters, and sets them
 *  into the first and second bytes of m_data.
 */

void
event::set_data (char a_D1, char a_D2)
{
    m_data[0] = a_D1 & 0x7F;
    m_data[1] = a_D2 & 0x7F;
}

/**
 *  Increments the second data byte (m_data[1]) and clears the most
 *  significant bit.
 */

void
event::increment_data2 (void)
{
    m_data[1] = (m_data[1] + 1) & 0x7F;
}

/**
 *  Decrements the second data byte (m_data[1]) and clears the most
 *  significant bit.
 */

void
event::decrement_data2 (void)
{
    m_data[1] = (m_data[1] - 1) & 0x7F;
}

/**
 *  Increments the first data byte (m_data[1]) and clears the most
 *  significant bit.
 */

void
event::increment_data1 (void)
{
    m_data[0] = (m_data[0] + 1) & 0x7F;
}

/**
 *  Decrements the first data byte (m_data[1]) and clears the most
 *  significant bit.
 */

void
event::decrement_data1(void)
{
    m_data[0] = (m_data[0] - 1) & 0x7F;
}

/**
 *  Retrieves the two data bytes from m_data[] and copies each into its
 *  respective parameter.
 */

void
event::get_data (unsigned char * D0, unsigned char * D1)
{
    *D0 = m_data[0];
    *D1 = m_data[1];
}

/**
 * \getter m_status
 */

unsigned char
event::get_status()
{
    return m_status;
}

/**
 *  Deletes and clears out the SYSEX buffer.
 */

void
event::start_sysex(void)
{
    if (not_nullptr(m_sysex))
        delete [] m_sysex;

    m_sysex = nullptr;
    m_size = 0;
}

/**
 *  Appends SYSEX data to a new buffer.  First, a buffer of size
 *  m_size+a_size is created.  The existing SYSEX data (stored in m_sysex)
 *  is copied to this buffer.  Then the data represented by a_data and
 *  a_size is appended to that data buffer.  Then the original SYSEX
 *  buffer, m_sysex, is deleted, and m_sysex is assigned to the new
 *  buffer..
 *
 * \warning
 *      This function does not check any pointers.
 *
 * \param a_data
 *      Provides the additional SYSEX data.
 *
 * \param a_size
 *      Provides the size of the additional SYSEX data.
 *
 * \return
 *      Returns false if there was an EVENT_SYSEX_END byte in the appended
 *      data.
 */

bool
event::append_sysex (unsigned char * a_data, long a_size)
{
    bool result = true;
    unsigned char * buffer = new unsigned char[m_size + a_size];
    memcpy(buffer, m_sysex, m_size);                // copy old and append
    memcpy(&buffer[m_size], a_data, a_size);
    delete [] m_sysex;
    m_size = m_size + a_size;
    m_sysex = buffer;
    for (int i = 0; i < a_size; i++)
    {
        if (a_data[i] == EVENT_SYSEX_END)
        {
            result = false;
            break;                                  // done, already false now
        }
    }
    return result;
}

/**
 * \getter m_sysex
 */

unsigned char *
event::get_sysex(void)
{
    return m_sysex;
}

/**
 * \setter m_size
 */

void
event::set_size (long a_size)
{
    m_size = a_size;
}

/**
 * \getter m_size
 */

long
event::get_size (void)
{
    return m_size;
}

/**
 *  Sets the note velocity, with is held in the second data byte,
 *  m_data[1].
 */

void
event::set_note_velocity (int a_vel)
{
    m_data[1] = a_vel & 0x7F;
}

/**
 *  Returns true if m_status is EVENT_NOTE_ON.
 */

bool
event::is_note_on ()
{
    return m_status == EVENT_NOTE_ON;
}

/**
 *  Returns true if m_status is EVENT_NOTE_OFF.
 */

bool
event::is_note_off ()
{
    return m_status == EVENT_NOTE_OFF;
}

/**
 *  Assuming m_data[] holds a note, get the note number, which is in the
 *  first data byte, m_data[0].
 */

unsigned char
event::get_note ()
{
    return m_data[0];
}

/**
 *  Sets the note number, clearing off the most-significant-bit and
 *  assigning it to the first data byte, m_data[0].
 */

void
event::set_note (char a_note)
{
    m_data[0] = a_note & 0x7F;
}

/**
 * \getter m_data[1], the note velocity.
 */

unsigned char
event::get_note_velocity()
{
    return m_data[1];
}

/**
 *  Prints out the timestamp, data size, the current status byte,
 *  any SYSEX data if present, or the two data bytes for the status byte.
 */

void
event::print()
{
    printf("[%06ld] [%04lX] %02X ", m_timestamp, m_size, m_status);
    if (m_status == EVENT_SYSEX)
    {
        for (int i = 0; i < m_size; i++)
        {
            if (i % 16 == 0)
                printf("\n    ");

            printf("%02X ", m_sysex[i]);
        }
        printf("\n");
    }
    else
    {
        printf("%02X %02X\n", m_data[0], m_data[1]);
    }
}

/**
 *  Returns the rank of the current m_status byte.
 *
 *  The ranking, from high to low, is note off, note on, aftertouch,
 *  channel pressure, and pitch wheel, control change, and program
 *  changes.
 */

int
event::get_rank (void) const
{
    switch (m_status)
    {
    case EVENT_NOTE_OFF:
        return 0x100;

    case EVENT_NOTE_ON:
        return 0x090;

    case EVENT_AFTERTOUCH:
    case EVENT_CHANNEL_PRESSURE:
    case EVENT_PITCH_WHEEL:
        return 0x050;

    case EVENT_CONTROL_CHANGE:
        return 0x010;

    case EVENT_PROGRAM_CHANGE:
        return 0x000;

    default:
        return 0;
    }
}

/**
 *  Sets m_has_link and sets m_link to the provided event pointer.
 */

void
event::link (event * a_event)
{
    m_has_link = true;
    m_linked = a_event;
}

/**
 * \getter m_linked
 */

event *
event::get_linked ()
{
    return m_linked;
}

/**
 * \getter m_has_link
 */

bool
event::is_linked ()
{
    return m_has_link;
}

/**
 * \setter m_has_link
 */

void
event::clear_link ()
{
    m_has_link = false;
}

/**
 * \setter m_selected
 */

void
event::select ()
{
    m_selected = true;
}

/**
 * \setter m_selected
 */

void
event::unselect ()
{
    m_selected = false;
}

/**
 * \getter m_selected
 */

bool
event::is_selected ()
{
    return m_selected;
}

/**
 * \setter m_painted
 */

void
event::paint ()
{
    m_painted = true;
}

/**
 * \setter m_painted
 */

void
event::unpaint ()
{
    m_painted = false;
}

/**
 * \getter m_painted
 */

bool
event::is_painted ()
{
    return m_painted;
}

/**
 * \setter m_marked
 */

void
event::mark ()
{
    m_marked = true;
}

/**
 * \setter m_marked
 */

void
event::unmark ()
{
    m_marked = false;
}

/**
 * \getter m_marked
 */

bool
event::is_marked ()
{
    return m_marked;
}

/*
 * event.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
