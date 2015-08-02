#ifndef SEQ24_SEQ24SEQ_H
#define SEQ24_SEQ24SEQ_H

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
 * \file          seq24seq.h
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

/*
#include <gtkmm/window.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/window.h>
#include <gtkmm/table.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/widget.h>
#include <gtkmm/adjustment.h>
*/

#include <gdk/gdkevents.h>              // GdkEventButton & GdkEventMotion

// #include "globals.h"
// #include "sequence.h"
// #include "seqkeys.h"
// #include "seqdata.h"

using namespace Gtk;

//  class GdkEventButton;
//  class GdkEventMotion;

namespace Gtk
{
}

class seqevent;

/**
 *  This structure implement the normal interaction methods for Seq24.
 */

struct Seq24SeqEventInput
{
    bool m_adding;

    Seq24SeqEventInput () :
        m_adding    (false)
    {
        // Empty body
    }
    void set_adding (bool a_adding, seqevent & ths);
    bool on_button_press_event (GdkEventButton * a_ev, seqevent & ths);
    bool on_button_release_event (GdkEventButton * a_ev, seqevent & ths);
    bool on_motion_notify_event (GdkEventMotion * a_ev, seqevent & ths);
};

#endif   // SEQ24_SEQ24SEQ_H

/*
 * seq24seq.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
