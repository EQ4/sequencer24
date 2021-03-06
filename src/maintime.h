#ifndef SEQ24_MAINTIME_H
#define SEQ24_MAINTIME_H

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
 * \file          maintime.h
 *
 *  This module declares/defines the base class for the "time" progress
 *  window.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-09-10
 * \license       GNU GPLv2 or above
 *
 */

#include <gtkmm/drawingarea.h>

/**
 *  This class provides the drawing of the progress bar at the top of the
 *  main window, along with the "pills" that move in time with the
 *  measures.
 */

class maintime : public Gtk::DrawingArea
{

private:

    Glib::RefPtr<Gdk::GC> m_gc;
    Glib::RefPtr<Gdk::Window> m_window;
    Gdk::Color m_black;
    Gdk::Color m_white;
    Gdk::Color m_grey;
    long m_tick;

public:

    maintime ();
    int idle_progress (long a_ticks);

private:        // callbacks

    void on_realize ();
    bool on_expose_event (GdkEventExpose * a_ev);

};

#endif   // SEQ24_MAINTIME_H

/*
 * maintime.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
