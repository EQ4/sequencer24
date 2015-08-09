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
 * \file          seqdata.cpp
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

#include <gtkmm/adjustment.h>

#include "font.h"
#include "seqdata.h"
#include "sequence.h"

/**
 *  Principal constructor.  In the constructor you can only allocate
 *  colors, get_window() returns 0 because we have not been realized.
 */

seqdata::seqdata
(
    sequence * a_seq,
    int a_zoom,
    Gtk::Adjustment * a_hadjust
) :
    // IDEA: m_text_font_5_7(Gdk_Font( c_font_5_7 )),
    m_gc                    (),
    m_window                (),
    m_black                 (Gdk::Color("black")),
    m_white                 (Gdk::Color("white")),
    m_grey                  (Gdk::Color("grey")),
    m_pixmap                (),
    m_window_x              (0),
    m_window_y              (0),
    m_current_x             (0),
    m_current_y             (0),
    m_drop_x                (0),
    m_drop_y                (0),
    m_hadjust               (a_hadjust),
    m_seq                   (a_seq),
    m_zoom                  (a_zoom),
    m_scroll_offset_ticks   (0),
    m_scroll_offset_x       (0),
    m_background_tile_x     (0),
    m_background_tile_y     (0),
    m_status                (0),
    m_cc                    (0),
    m_numbers               (),             // an array
    m_old                   (),
    m_dragging              (false)
{
    add_events
    (
        Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
        Gdk::POINTER_MOTION_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::SCROLL_MASK
    );

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(m_black);
    colormap->alloc_color(m_white);
    colormap->alloc_color(m_grey);
    set_flags(Gtk::CAN_FOCUS);
    set_double_buffered(false);
    set_size_request(10, c_dataarea_y);
}

/**
 *  Updates the sizes in the pixmap if the view is realized, and queues up
 *  a draw operation.  It creates a pixmap with window dimensions given by
 *  m_window_x and m_window_y.
 */

void
seqdata::update_sizes ()
{
    if (is_realized())
    {
        m_pixmap = Gdk::Pixmap::create(m_window, m_window_x, m_window_y, -1);
        redraw();           // instead of update_pixmap(); queue_draw();
    }
}

/**
 *  This function calls update_size().  Then, regardless of whether the
 *  view is realized, updates the pixmap and queues up a draw operation.
 *
 * \note
 *      If it weren't for the is_realized() condition, we could just call
 *      update_sizes(), which does all this anyway.
 */

void
seqdata::reset ()
{
    update_sizes();
    redraw();       // use common code instead of update_pixmap(); queue_draw();
}

/**
 *  Sets the zoom to the given value and resets the view via the reset
 *  function.
 *
 *  This begs the question, do we have GUI access to the zoom setting?
 */

void
seqdata::set_zoom (int a_zoom)
{
    if (m_zoom != a_zoom)
    {
        m_zoom = a_zoom;
        reset();
    }
}

/**
 *  Sets the status to the given value, and the control to the optional
 *  given value, which defaults to 0, then calls redraw().
 */

void
seqdata::set_data_type (unsigned char a_status, unsigned char a_control = 0)
{
    m_status = a_status;
    m_cc = a_control;
    redraw();                   // this->redraw();   weird
}

/**
 *  Simply calls draw_events_on_pixmap().
 */

void
seqdata::update_pixmap ()
{
    draw_events_on_pixmap();
}

/**
 *  Draws events on the given drawable object.
 */

void
seqdata::draw_events_on (Glib::RefPtr<Gdk::Drawable> a_draw)
{
    long tick;
    unsigned char d0, d1;
    bool selected;
    int start_tick = m_scroll_offset_ticks ;
    int end_tick = (m_window_x * m_zoom) + m_scroll_offset_ticks;
    m_gc->set_foreground(m_white);
    a_draw->draw_rectangle(m_gc, true, 0, 0, m_window_x, m_window_y);
    m_gc->set_foreground(m_black);
    m_seq->reset_draw_marker();
    while (m_seq->get_next_event(m_status, m_cc, &tick, &d0, &d1, &selected))
    {
        if (tick >= start_tick && tick <= end_tick)
        {
            /* Turn into screen coordinates */

            int event_x = tick / m_zoom;            /* event_width=c_data_x */
            int event_height = d1;                  /* generate the value   */
            if (m_status == EVENT_PROGRAM_CHANGE ||
                    m_status == EVENT_CHANNEL_PRESSURE)
            {
                event_height = d0;
            }
            m_gc->set_line_attributes
            (
                2, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_MITER
            );
            a_draw->draw_line                       /* draw vert lines      */
            (
                m_gc, event_x -  m_scroll_offset_x + 1,
                c_dataarea_y - event_height, event_x -  m_scroll_offset_x + 1,
                c_dataarea_y
            );
            a_draw->draw_drawable
            (
                m_gc, m_numbers[event_height], 0, 0,
                event_x + 3 - m_scroll_offset_x, c_dataarea_y - 25, 6, 30
            );
        }
    }
}

/**
 *  Simply calls draw_events_on() for this object's built-in pixmap.
 */

void
seqdata::draw_events_on_pixmap ()
{
    draw_events_on(m_pixmap);
}

/**
 *  Simply queues up a draw operation.
 */

void
seqdata::draw_pixmap_on_window ()
{
    queue_draw();
}

/**
 *  Draws events on this object's built-in window and pixmap.
 *  This drawing is done only if there is no dragging in progress, to
 *  guarantee no flicker.
 */

int
seqdata::idle_redraw ()
{
    if (! m_dragging)
    {
        draw_events_on(m_window);
        draw_events_on(m_pixmap);
    }
    return true;
}

/**
 *  This function takes screen coordinates, and give us the horizontaol
 *  tick value based on the current zoom, returned vias the second
 *  paramater.
 */

void
seqdata::convert_x (int a_x, long * a_tick)
{
    *a_tick = a_x * m_zoom;
}

/**
 *  This function takes two points, and returns an Xwin rectangle,
 *  returned via the last four parameters.
 *  It checks the mins/maxes, then fills in x,y, and width, height.
 */

void
seqdata::xy_to_rect
(
    int a_x1,  int a_y1,
    int a_x2,  int a_y2,
    int * a_x,  int * a_y,
    int * a_w,  int * a_h
)
{
    if (a_x1 < a_x2)
    {
        *a_x = a_x1;
        *a_w = a_x2 - a_x1;
    }
    else
    {
        *a_x = a_x2;
        *a_w = a_x1 - a_x2;
    }
    if (a_y1 < a_y2)
    {
        *a_y = a_y1;
        *a_h = a_y2 - a_y1;
    }
    else
    {
        *a_y = a_y2;
        *a_h = a_y1 - a_y2;
    }
}

/**
 *  Handles a motion-notify event.  It converts the x,y of the mouse to
 *  ticks, then sets the events in the event-data-range, updates the
 *  pixmap, draws events in the window, and draws a line on the window.
 */

bool
seqdata::on_motion_notify_event (GdkEventMotion * a_p0)
{
    if (m_dragging)
    {
        int adj_x_min, adj_x_max, adj_y_min, adj_y_max;
        m_current_x = (int) a_p0->x + m_scroll_offset_x;
        m_current_y = (int) a_p0->y;
        if (m_current_x < m_drop_x)
        {
            adj_x_min = m_current_x;
            adj_y_min = m_current_y;
            adj_x_max = m_drop_x;
            adj_y_max = m_drop_y;
        }
        else
        {
            adj_x_max = m_current_x;
            adj_y_max = m_current_y;
            adj_x_min = m_drop_x;
            adj_y_min = m_drop_y;
        }

        long tick_s, tick_f;
        convert_x(adj_x_min, &tick_s);
        convert_x(adj_x_max, &tick_f);
        m_seq->change_event_data_range
        (
            tick_s, tick_f, m_status, m_cc,
            c_dataarea_y - adj_y_min - 1, c_dataarea_y - adj_y_max - 1
        );
        update_pixmap();
        draw_events_on(m_window);
        draw_line_on_window();
    }
    return true;
}

/*
 * ca 2015-07-24
 * Eliminate this annoying warning.  Will do it for Microsoft's bloddy
 * compiler later.
 */

#ifdef PLATFORM_GNU
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

/**
 *  Handles an on-leave notification event.
 */

bool
seqdata::on_leave_notify_event (GdkEventCrossing * p0)
{
    redraw();           // instead of update_pixmap(); queue_draw();
    return true;
}

/**
 *  Draws on vertical line on...
 */

void
seqdata::draw_line_on_window ()
{
    int x, y, w, h;
    m_gc->set_foreground(m_black);
    m_gc->set_line_attributes
    (
        1, Gdk::LINE_SOLID, Gdk::CAP_NOT_LAST, Gdk::JOIN_MITER
    );

    m_window->draw_drawable                         /* replace old */
    (
        m_gc, m_pixmap, m_old.x, m_old.y, m_old.x, m_old.y,
        m_old.width + 1, m_old.height + 1
    );
    xy_to_rect(m_drop_x, m_drop_y, m_current_x, m_current_y, &x, &y, &w, &h);
    x -= m_scroll_offset_x;
    m_old.x = x;
    m_old.y = y;
    m_old.width = w;
    m_old.height = h;
    m_gc->set_foreground(m_black);
    m_window->draw_line
    (
        m_gc, m_current_x - m_scroll_offset_x, m_current_y,
        m_drop_x - m_scroll_offset_x, m_drop_y
    );
}

/**
 *  Change the scrolling offset on the x-axis, and redraw.
 */

void
seqdata::change_horz ()
{
    m_scroll_offset_ticks = (int) m_hadjust->get_value();
    m_scroll_offset_x = m_scroll_offset_ticks / m_zoom;
    update_pixmap();
    force_draw();
}

/**
 *  Force a redraw.
 */

void
seqdata::force_draw ()
{
    m_window->draw_drawable(m_gc, m_pixmap, 0, 0, 0, 0, m_window_x, m_window_y);
}

/**
 *  Implements the on-realization event, by calling the base-class version
 *  and then allocating the resources that could not be allocated in the
 *  constructor.  It also connects up the change_horz() function.
 */

void
seqdata::on_realize ()
{
    Gtk::DrawingArea::on_realize();
    m_window = get_window();
    m_gc = Gdk::GC::create(m_window);
    m_window->clear();
    m_hadjust->signal_value_changed().connect
    (
        mem_fun(*this, &seqdata::change_horz)
    );
    for (int i = 0; i < c_dataarea_y; ++i)
    {
        m_numbers[i] = Gdk::Pixmap::create(m_window, 6, 30, -1);
        m_gc->set_foreground(m_white);
        m_numbers[i]->draw_rectangle(m_gc, true, 0, 0, 6, 30);

        char val[5];
        snprintf(val, sizeof(val), "%3d\n", i);
        char num[6];
        memset(num, 0, 6);              // FIX!!!
        num[0] = val[0];
        num[2] = val[1];
        num[4] = val[2];

        p_font_renderer->render_string_on_drawable
        (
            m_gc, 0, 0, m_numbers[i], &num[0], font::BLACK
        );
        p_font_renderer->render_string_on_drawable
        (
            m_gc, 0, 8, m_numbers[i], &num[2], font::BLACK
        );
        p_font_renderer->render_string_on_drawable
        (
            m_gc, 0, 16, m_numbers[i], &num[4], font::BLACK
        );
    }
    update_sizes();
}

/**
 *  Implements the on-expose event.
 */

bool
seqdata::on_expose_event (GdkEventExpose * a_e)
{
    m_window->draw_drawable
    (
        m_gc, m_pixmap, a_e->area.x, a_e->area.y,
        a_e->area.x, a_e->area.y, a_e->area.width, a_e->area.height
    );
    return true;
}

/**
 *  Implements the on-scroll event.  This scroll event only handles basic
 *  scrolling, without any modifier keys such as GDK_CONTROL_MASK or
 *  GDK_SHIFT_MASK.
 */

bool
seqdata::on_scroll_event (GdkEventScroll * a_ev)
{
    guint modifiers;                    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask();
    if ((a_ev->state & modifiers) != 0)
        return false;

    if (a_ev->direction == GDK_SCROLL_UP)
        m_seq->increment_selected(m_status, m_cc);

    if (a_ev->direction == GDK_SCROLL_DOWN)
        m_seq->decrement_selected(m_status, m_cc);

    update_pixmap();
    queue_draw();
    return true;
}

/**
 *  Implement a button-press event.
 */

bool
seqdata::on_button_press_event (GdkEventButton * a_p0)
{
    if (a_p0->type == GDK_BUTTON_PRESS)
    {
        m_seq->push_undo();
        m_drop_x = (int) a_p0->x + m_scroll_offset_x; /* set values for line */
        m_drop_y = (int) a_p0->y;
        m_old.x = 0;                /* reset box that holds dirty redraw spot */
        m_old.y = 0;
        m_old.width = 0;
        m_old.height = 0;
        m_dragging = true;          /* we are potentially dragging now! */
    }
    return true;
}

/**
 *  Implement a button-release event.
 */

bool
seqdata::on_button_release_event (GdkEventButton * a_p0)
{
    m_current_x = (int) a_p0->x + m_scroll_offset_x;
    m_current_y = (int) a_p0->y;
    if (m_dragging)
    {
        long tick_s, tick_f;
        if (m_current_x < m_drop_x)
        {
            std::swap(m_current_x, m_drop_x);
            std::swap(m_current_y, m_drop_y);
        }
        convert_x(m_drop_x, &tick_s);
        convert_x(m_current_x, &tick_f);
        m_seq->change_event_data_range
        (
            tick_s, tick_f, m_status, m_cc,
            c_dataarea_y - m_drop_y - 1, c_dataarea_y - m_current_y - 1
        );
        m_dragging = false;     /* convert x,y to ticks, set events in range */
    }
    update_pixmap();
    queue_draw();
    return true;
}

/**
 *  Handle a size-allocation event.
 */

void
seqdata::on_size_allocate (Gtk::Allocation & a_r)
{
    Gtk::DrawingArea::on_size_allocate(a_r);
    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();
    update_sizes();
}

/*
 * seqdata.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
