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
 * \file          event.cpp
 *
 *  This module declares/defines the base class for MIDI events.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-06
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
    m_data      (),                 // small array
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
        return get_rank() > a_rhsevent.get_rank();
    else
        return m_timestamp > a_rhsevent.m_timestamp;
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
        return (get_rank() < a_rhsevent.get_rank());
    else
        return (m_timestamp < a_rhsevent.m_timestamp);
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
event::increment_data2 ()
{
    m_data[1] = (m_data[1] + 1) & 0x7F;
}

/**
 *  Decrements the second data byte (m_data[1]) and clears the most
 *  significant bit.
 */

void
event::decrement_data2 ()
{
    m_data[1] = (m_data[1] - 1) & 0x7F;
}

/**
 *  Increments the first data byte (m_data[1]) and clears the most
 *  significant bit.
 */

void
event::increment_data1 ()
{
    m_data[0] = (m_data[0] + 1) & 0x7F;
}

/**
 *  Decrements the first data byte (m_data[1]) and clears the most
 *  significant bit.
 */

void
event::decrement_data1 ()
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
 *  Deletes and clears out the SYSEX buffer.
 */

void
event::start_sysex ()
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

/*
 * event.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
