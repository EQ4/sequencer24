#ifndef SEQ24_FRUITYSEQROLL_H
#define SEQ24_FRUITYSEQROLL_H

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
 * \file          fruityseqroll.h
 *
 *  This module declares/defines the base class for seqroll interactions
 *  using the "fruity" mouse paradigm.
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
 *  Implements the fruity mouse interaction paradigm for the seqroll.
 */

struct FruitySeqRollInput
{
    bool m_adding;
    bool m_canadd;
    bool m_erase_painting;
    long m_drag_paste_start_pos[2];

    /**
     * Default constructor.
     */

    FruitySeqRollInput () :
        m_adding        (false),
        m_canadd        (true),
        m_erase_painting(false)
    {
        // Empty body
    }

    void updateMousePtr (seqroll & ths);

public:         // callbacks

    bool on_button_press_event (GdkEventButton * a_ev, seqroll & ths);
    bool on_button_release_event (GdkEventButton * a_ev, seqroll & ths);
    bool on_motion_notify_event (GdkEventMotion * a_ev, seqroll & ths);
};

#endif   // SEQ24_FRUITYSEQROLL_H

/*
 * fruityseqroll.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
