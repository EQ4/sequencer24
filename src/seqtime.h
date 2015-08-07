#ifndef SEQ24_SEQTIME_H
#define SEQ24_SEQTIME_H

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
 * \file          seqtime.h
 *
 *  This module declares/defines the base class for ...
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-06
 * \license       GNU GPLv2 or above
 *
 */

#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>

#include "globals.h"

namespace Gtk
{
    class Adjustment;
}

class sequence;

/**
 *    This class implements the piano time, whatever that is.
 */

class seqtime: public Gtk::DrawingArea
{

private:

    Glib::RefPtr<Gdk::GC> m_gc;
    Glib::RefPtr<Gdk::Window> m_window;
    Gdk::Color m_black;
    Gdk::Color m_white;
    Gdk::Color m_grey;
    Glib::RefPtr<Gdk::Pixmap> m_pixmap;
    int m_window_x;
    int m_window_y;
    Gtk::Adjustment * m_hadjust;
    sequence * m_seq;
    int m_scroll_offset_ticks;
    int m_scroll_offset_x;

    /**
     * one pixel == m_zoom ticks
     */

    int m_zoom;

public:

    seqtime (sequence * a_seq, int a_zoom, Gtk::Adjustment * a_hadjust);

    void reset ();
    void redraw ();
    void set_zoom (int a_zoom);

private:

    void draw_pixmap_on_window ();
    void draw_progress_on_window ();
    void update_pixmap ();
    bool idle_progress ();
    void change_horz ();
    void update_sizes ();
    void force_draw ();

private:          // callbacks

    void on_realize ();
    bool on_expose_event (GdkEventExpose * a_ev);
    bool on_button_press_event (GdkEventButton * a_ev);
    bool on_button_release_event (GdkEventButton * a_ev);
    void on_size_allocate (Gtk::Allocation &);

};

#endif   // SEQ24_SEQTIME_H

/*
 * seqtime.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
