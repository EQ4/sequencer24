#ifndef SEQ24_SEQ24SEQROLL_H
#define SEQ24_SEQ24SEQROLL_H

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
 * \file          seq24seqroll.h
 *
 *  This module declares/defines the base class for handling the Seq24
 *  mode of mouse interaction in the piano roll of the pattern/sequence
 *  editor.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-09
 * \license       GNU GPLv2 or above
 *
 */

class seqroll;

/**
 *  Implements the Seq24 mouse interaction paradigm for the seqroll.
 */

struct Seq24SeqRollInput
{
    bool m_adding;

    /**
     * Default constructor.
     */

    Seq24SeqRollInput () :
        m_adding    (false)
    {
        // Empty body
    }

    void set_adding (bool a_adding, seqroll & ths);

public:         // callbacks

    bool on_button_press_event (GdkEventButton * a_ev, seqroll & ths);
    bool on_button_release_event (GdkEventButton * a_ev, seqroll & ths);
    bool on_motion_notify_event (GdkEventMotion * a_ev, seqroll & ths);
};

#endif   // SEQ24_SEQ24SEQROLL_H

/*
 * seq24seqroll.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
