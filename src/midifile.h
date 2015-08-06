#ifndef SEQ24_MIDIFILE_H
#define SEQ24_MIDIFILE_H

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
 * \file          midifile.h
 *
 *  This module declares/defines the base class for MIDI files.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-05
 * \license       GNU GPLv2 or above
 *
 */

#include <string>
#include <list>
#include <vector>

class perform;                          // forward reference

class midifile
{

private:

    /**
     *  Holds the position in the MIDI file.
     */

    int m_pos;

    /**
     *  The unchanging name of the MIDI file.
     */

    const std::string m_name;

    /**
     *  This vector of characters holds our MIDI data.  We could also use
     *  a string of characters, unsigned.
     */

    std::vector<unsigned char> m_data;

    /**
     *  Provides a list of characters.
     */

    std::list<unsigned char> m_char_list;

public:

    midifile (const std::string &);
    ~midifile ();

    bool parse (perform * a_perf, int a_screen_set);
    bool write (perform * a_perf);

private:

    unsigned long read_long ();
    unsigned short read_short ();
    unsigned char read_byte ();
    unsigned long read_var ();

    void write_long (unsigned long);
    void write_short (unsigned short);
    void write_byte (unsigned char);

};

#endif   // SEQ24_MIDIFILE_H

/*
 * midifile.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
