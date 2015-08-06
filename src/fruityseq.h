#ifndef SEQ24_FRUITYSEQ_H
#define SEQ24_FRUITYSEQ_H

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
 * \file          fruityseq.h
 *
 *  This module declares/defines the mouse interactions for the "fruity"
 *  mode.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-08-02
 * \updates       2015-08-02
 * \license       GNU GPLv2 or above
 *
 */

class seqevent;

/**
 *  This structure implements the interaction methods for the "fruity"
 *  mode of operation.
 */

struct FruitySeqEventInput
{
    bool m_justselected_one;
    bool m_is_drag_pasting_start;
    bool m_is_drag_pasting;

    /**
     * Default constructor.
     */

    FruitySeqEventInput () :
        m_justselected_one      (false),
        m_is_drag_pasting_start (false),
        m_is_drag_pasting       (false)
    {
        // Empty body
    }

    void updateMousePtr (seqevent & ths);
    bool on_button_press_event (GdkEventButton * a_ev, seqevent & ths);
    bool on_button_release_event (GdkEventButton * a_ev, seqevent & ths);
    bool on_motion_notify_event (GdkEventMotion * a_ev, seqevent & ths);
};

#endif   // SEQ24_FRUITYSEQ_H

/*
 * fruityseq.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
