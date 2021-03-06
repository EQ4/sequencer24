#ifndef SEQ24_SEQDATA_H
#define SEQ24_SEQDATA_H

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
 * \file          seqdata.h
 *
 *  This module declares/defines the base class for plastering
 *  pattern/sequence data information in the data area of the pattern
 *  editor.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-06
 * \license       GNU GPLv2 or above
 *
 */

#include <gtkmm/drawingarea.h>

#include "globals.h"

namespace Gtk
{
    class Adjustment;
}

class sequence;

/**
 *    This class supports drawing piano-roll eventis on a window.
 */

class seqdata : public Gtk::DrawingArea
{

    friend class seqroll;
    friend class seqevent;

private:

    Glib::RefPtr<Gdk::GC> m_gc;
    Glib::RefPtr<Gdk::Window> m_window;
    Gdk::Color m_black;
    Gdk::Color m_white;
    Gdk::Color m_grey;
    Glib::RefPtr<Gdk::Pixmap> m_pixmap;
    int m_window_x;
    int m_window_y;
    int m_current_x;
    int m_current_y;
    int m_drop_x;
    int m_drop_y;
    Gtk::Adjustment * const m_hadjust;
    sequence * const m_seq;

    /**
     *  one pixel == m_zoom ticks
     */

    int m_zoom;

    int m_scroll_offset_ticks;
    int m_scroll_offset_x;
    int m_background_tile_x;
    int m_background_tile_y;

    /**
     *  What is the data window currently editing?
     */

    unsigned char m_status;
    unsigned char m_cc;
    Glib::RefPtr<Gdk::Pixmap> m_numbers[c_dataarea_y];
    GdkRectangle m_old;
    bool m_dragging;

public:

    seqdata (sequence * a_seq, int a_zoom,  Gtk::Adjustment * a_hadjust);

    void reset ();

    /**
     *  Updates the pixmap and queues up a redraw operation.  We need to
     *  make this an inline function and use it as common code.
     */

    void redraw ()
    {
        update_pixmap();
        queue_draw();
    }

    void set_zoom (int a_zoom);
    void set_data_type (unsigned char a_status, unsigned char a_control);
    int idle_redraw ();

private:

    void update_sizes ();
    void draw_events_on_pixmap ();
    void draw_pixmap_on_window ();
    void update_pixmap ();
    void draw_line_on_window ();
    void convert_x (int a_x, long *a_tick);
    void xy_to_rect
    (
      int a_x1,  int a_y1,
      int a_x2,  int a_y2,
      int * a_x,  int * a_y,
      int * a_w,  int * a_h
   );
    void draw_events_on (Glib::RefPtr<Gdk::Drawable> a_draw);
    void change_horz ();
    void force_draw ();

private:       // callbacks

    void on_realize ();
    bool on_expose_event (GdkEventExpose * a_ev);
    bool on_button_press_event (GdkEventButton * a_ev);
    bool on_button_release_event (GdkEventButton * a_ev);
    bool on_motion_notify_event (GdkEventMotion * a_p0);
    bool on_leave_notify_event (GdkEventCrossing * p0);
    bool on_scroll_event (GdkEventScroll * a_ev);
    void on_size_allocate (Gtk::Allocation &);

};

#endif   // SEQ24_SEQDATA_H

/*
 * seqdata.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
