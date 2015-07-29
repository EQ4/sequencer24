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
 * \file          mainwid.cpp
 *
 *  This module declares/defines the base class for drawing
 *  patterns/sequences in the Patterns Panel grid.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-07-26
 * \license       GNU GPLv2 or above
 *
 */

#include "mainwid.h"
#include "seqedit.h"
#include "font.h"

/**
 *  These look like the "Sequence toggle keys" in the Options / Keyboard
 *  dialog, except that they are upper-case here, and lower-case in that
 *  configuration dialog.
 */

const char mainwid::m_seq_to_char[c_seqs_in_set] =
{
    '1', 'Q', 'A', 'Z',
    '2', 'W', 'S', 'X',
    '3', 'E', 'D', 'C',
    '4', 'R', 'F', 'V',
    '5', 'T', 'G', 'B',
    '6', 'Y', 'H', 'N',
    '7', 'U', 'J', 'M',
    '8', 'I', 'K', ','
};

/**
 *  This constructor sets a lot of the members, but not all.  And it asks
 *  for a size of c_mainwid_x by c_mainwid_y.  It adds GDK masks for
 *  button presses, releases, and motion, and key presses and focus
 *  changes.
 */

mainwid::mainwid (perform *a_p)
 :
    seqmenu         (a_p),
    m_gc            (),
    m_window        (),
    m_black         (Gdk::Color("black")),
    m_white         (Gdk::Color("white")),
    m_grey          (Gdk::Color("grey")),
//  m_dk_grey       (),                // not used
    m_background    (),
    m_foreground    (),
    m_pixmap        (),
//  m_old           (),                // not used
//  m_selected      (),                // not used
    m_screenset     (0),
    m_mainperf      (a_p),
//  m_clipboard     (),                // not used
    m_moving_seq    (),
    m_window_x      (c_mainwid_x),
    m_window_y      (c_mainwid_y),
    m_button_down   (false),
    m_moving        (false)
{
    using namespace Menu_Helpers;

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(m_black);
    colormap->alloc_color(m_white);
    colormap->alloc_color(m_grey);
    set_size_request(c_mainwid_x, c_mainwid_y);
    add_events
    (
        Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
        Gdk::KEY_PRESS_MASK | Gdk::BUTTON_MOTION_MASK | Gdk::FOCUS_CHANGE_MASK
    );
    set_double_buffered(false);
}

mainwid::~mainwid ()
{
    // empty body
}

/**
 *  This function fills the pixmap with sequences.
 */

void
mainwid::draw_sequences_on_pixmap ()
{
    for (int i = 0; i < c_mainwnd_rows * c_mainwnd_cols; i++)
    {
        draw_sequence_on_pixmap
        (
            i + (m_screenset * c_mainwnd_rows * c_mainwnd_cols)
        );
        m_last_tick_x[i + (m_screenset * c_mainwnd_rows * c_mainwnd_cols)] = 0;
    }
}

/**
 *  This function updates the background window, clearing it.
 */

void
mainwid::fill_background_window ()
{
    m_pixmap->draw_rectangle
    (
        this->get_style()->get_bg_gc(Gtk::STATE_NORMAL),
        true, 0, 0, m_window_x, m_window_y
    );
}

/**
 *  Is this a nullified callback?
 */

int
mainwid::timeout ()
{
    return true;
}

/**
 *  This function draws a specific pattern/sequence on the pixmap located
 *  in the main window of the application, the Patterns Panel.  The
 *  sequence is drawn only if it is in the current screen set (indicated
 *  by m_screenset.
 */

void
mainwid::draw_sequence_on_pixmap (int a_seq)
{
    if      /* Common code */
    (
        a_seq >= (m_screenset * c_mainwnd_rows * c_mainwnd_cols) &&
        a_seq < ((m_screenset + 1) * c_mainwnd_rows * c_mainwnd_cols)
    )
    {
        int i = (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        int j =  a_seq % c_mainwnd_rows;
        int base_x = (c_mainwid_border + (c_seqarea_x + c_mainwid_spacing) * i);
        int base_y = (c_mainwid_border + (c_seqarea_y + c_mainwid_spacing) * j);
        m_gc->set_foreground(m_black);
        m_pixmap->draw_rectangle                // outer border of box
        (
            m_gc, true, base_x, base_y, c_seqarea_x, c_seqarea_y
        );
        if (m_mainperf->is_active(a_seq))
        {
            sequence * seq = m_mainperf->get_sequence(a_seq);
            if (seq->get_playing())
            {
                m_last_playing[a_seq] = true;   // active and playing
                m_background = m_black;
                m_foreground = m_white;
            }
            else
            {
                m_last_playing[a_seq] = false;  // active and not playing
                m_background = m_white;
                m_foreground = m_black;
            }
            m_gc->set_foreground(m_background); // tricky reversal of color!
            m_pixmap->draw_rectangle            // ?????
            (
                m_gc, true, base_x+1, base_y+1, c_seqarea_x-2, c_seqarea_y-2
            );
            m_gc->set_foreground(m_foreground);

            char name[20];
            snprintf(name, sizeof name, "%.13s", seq->get_name());
            font::Color col = font::BLACK;
            if (m_foreground == m_black)
                col = font::BLACK;

            if (m_foreground == m_white)
                col = font::WHITE;

            p_font_renderer->render_string_on_drawable      // name of pattern
            (
                m_gc, base_x+c_text_x, base_y+4, m_pixmap, name, col
            );

            char str[20];
            if (m_mainperf->show_ui_sequence_key())
            {
                snprintf
                (
                    str, sizeof str, "%c",
                    (char) m_mainperf->lookup_keyevent_key(a_seq)
                );
                p_font_renderer->render_string_on_drawable  // shortcut key
                (
                    m_gc, base_x+c_seqarea_x-7, base_y + c_text_y*4 - 2,
                    m_pixmap, str, col
                );
            }
            snprintf
            (
                str, sizeof str, "%d-%d %ld/%ld",
                seq->get_midi_bus(), seq->get_midi_channel() + 1,
                seq->get_bpm(), seq->get_bw()
            );
            p_font_renderer->render_string_on_drawable      // bus, ch, etc.
            (
                m_gc, base_x + c_text_x, base_y + c_text_y*4 - 2,
                m_pixmap, str, col
            );

            int rectangle_x = base_x + c_text_x - 1;
            int rectangle_y = base_y + c_text_y + c_text_x - 1;
            if (seq->get_queued())
            {
                m_gc->set_foreground(m_grey);
                m_pixmap->draw_rectangle                    // inner box?
                (
                    m_gc, true, rectangle_x - 2, rectangle_y - 1,
                    c_seqarea_seq_x + 3, c_seqarea_seq_y + 3
                );
                m_foreground = m_black;
            }
            m_gc->set_foreground(m_foreground);
            m_pixmap->draw_rectangle                        // ditto, unqueued
            (
                m_gc, false, rectangle_x - 2, rectangle_y - 1,
                c_seqarea_seq_x + 3, c_seqarea_seq_y + 3
            );

            int lowest_note = seq->get_lowest_note_event();
            int highest_note = seq->get_highest_note_event();
            int height = highest_note - lowest_note + 2;
            int length = seq->get_length();
            long tick_s;
            long tick_f;
            int note;
            bool selected;
            int velocity;
            draw_type dt;

            seq->reset_draw_marker();
            while                           // draws the note marks in inner box
            (
                (
                    dt = seq->get_next_note_event
                    (
                        &tick_s, &tick_f, &note, &selected, &velocity
                    )
                ) != DRAW_FIN
            )
            {
                int note_y = c_seqarea_seq_y -
                     (c_seqarea_seq_y * (note + 1 - lowest_note)) / height;

                int tick_s_x = (tick_s * c_seqarea_seq_x)  / length;
                int tick_f_x = (tick_f * c_seqarea_seq_x)  / length;
                if (dt == DRAW_NOTE_ON || dt == DRAW_NOTE_OFF)
                    tick_f_x = tick_s_x + 1;

                if (tick_f_x <= tick_s_x)
                    tick_f_x = tick_s_x + 1;

                m_gc->set_foreground(m_foreground);
                m_pixmap->draw_line
                (
                    m_gc, rectangle_x + tick_s_x,
                    rectangle_y + note_y, rectangle_x + tick_f_x,
                    rectangle_y + note_y
                );
            }
        }
        else /* not active */
        {
            m_gc->set_foreground(m_grey);
            m_pixmap->draw_rectangle
            (
                this->get_style()->get_bg_gc(Gtk::STATE_NORMAL),
                true, base_x + 4, base_y, c_seqarea_x - 8, c_seqarea_y
            );
            m_pixmap->draw_rectangle
            (
                this->get_style()->get_bg_gc(Gtk::STATE_NORMAL),
                true, base_x + 1, base_y + 1, c_seqarea_x - 2, c_seqarea_y - 2
            );
        }
    }
}

/**
 *  This function draws something in the Patterns Panel.  The sequence is
 *  drawn only if it is in the current screen set (indicated by
 *  m_screenset.  However, if we comment out this code, we can't see any
 *  difference in the Patterns Panel, even when playback is ongoing!
 */

void
mainwid::draw_sequence_pixmap_on_window (int a_seq)         // effective?
{
    /*
     * Commenting out this whole function block seems to change nothing!!!
     */

    if      /* Common code */
    (
        a_seq >= (m_screenset  * c_mainwnd_rows * c_mainwnd_cols) &&
        a_seq < ((m_screenset + 1)  * c_mainwnd_rows * c_mainwnd_cols)
    )
    {
        int i = (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        int j =  a_seq % c_mainwnd_rows;
        int base_x = (c_mainwid_border + (c_seqarea_x + c_mainwid_spacing) * i);
        int base_y = (c_mainwid_border + (c_seqarea_y + c_mainwid_spacing) * j);
        m_window->draw_drawable
        (
            m_gc, m_pixmap, base_x, base_y, base_x, base_y,
            c_seqarea_x, c_seqarea_y
        );
    }
}

/**
 *  Draw the the given pattern/sequence again.
 */

void
mainwid::redraw (int a_sequence)
{
    draw_sequence_on_pixmap(a_sequence);
    draw_sequence_pixmap_on_window(a_sequence);         // effective?
}

/**
 *  Draw the cursors (long vertical bars) on each sequence, so that they
 *  follow the playing progress of each sequence in the mainwid (Patterns
 *  Panel.)
 */

void
mainwid::update_markers(int a_ticks)
{
    for (int i = 0; i < c_mainwnd_rows *  c_mainwnd_cols; i++)
    {
        draw_marker_on_sequence
        (
            i + (m_screenset * c_mainwnd_rows * c_mainwnd_cols), a_ticks
        );
    }
}

/**
 *  Does the actual drawing of one pattern/sequence position marker (a
 *  vertical bar.
 *
 *  More Common code.
 */

void
mainwid::draw_marker_on_sequence (int a_seq, int a_tick)
{
    if (m_mainperf->is_dirty_main(a_seq))
        update_sequence_on_window(a_seq);

    if (m_mainperf->is_active(a_seq))
    {
        sequence * seq = m_mainperf->get_sequence(a_seq);
        int i = (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        int j =  a_seq % c_mainwnd_rows;
        int base_x = (c_mainwid_border + (c_seqarea_x + c_mainwid_spacing) * i);
        int base_y = (c_mainwid_border + (c_seqarea_y + c_mainwid_spacing) * j);
        int rectangle_x = base_x + c_text_x - 1;
        int rectangle_y = base_y + c_text_y + c_text_x - 1;
        int length = seq->get_length();
        a_tick += (length - seq->get_trigger_offset());
        a_tick %= length;

        long tick_x = a_tick * c_seqarea_seq_x / length;
        m_window->draw_drawable
        (
            m_gc, m_pixmap,
            rectangle_x + m_last_tick_x[a_seq], rectangle_y + 1,
            rectangle_x + m_last_tick_x[a_seq], rectangle_y + 1,
            1, c_seqarea_seq_y
        );
        m_last_tick_x[a_seq] = tick_x;
        if (seq->get_playing())
            m_gc->set_foreground(m_white);
        else
            m_gc->set_foreground(m_black);

        if (seq->get_queued())
            m_gc->set_foreground(m_black);

        m_window->draw_line
        (
            m_gc, rectangle_x + tick_x, rectangle_y + 1,
            rectangle_x + tick_x, rectangle_y + c_seqarea_seq_y
        );
    }
}

/**
 *  Updates the image of multiple sequencers.
 */

void
mainwid::update_sequences_on_window ()
{
    draw_sequences_on_pixmap();
    draw_pixmap_on_window();
}

/**
 *  Updates the image of one sequencer.
 */

void
mainwid::update_sequence_on_window (int a_seq)
{
    draw_sequence_on_pixmap(a_seq);
    draw_sequence_pixmap_on_window(a_seq);          // effective?
}

/**
 *  This function queues the blit of pixmap to window.
 */

void
mainwid::draw_pixmap_on_window ()
{
    queue_draw();
}

/**
 *  Translates XY corridinates in the Patterns Panel to a sequence number.
 */

int
mainwid::seq_from_xy (int a_x, int a_y)
{
    int x = a_x - c_mainwid_border;         // adjust for border
    int y = a_y - c_mainwid_border;
    if                                      // is it in the box?
    (
        x < 0 || x >= ((c_seqarea_x + c_mainwid_spacing) * c_mainwnd_cols) ||
        y < 0 || y >= ((c_seqarea_y + c_mainwid_spacing) * c_mainwnd_rows)
    )
    {
        return -1;                          // no
    }

    int box_test_x = x % (c_seqarea_x + c_mainwid_spacing); // box coordinate
    int box_test_y = y % (c_seqarea_y + c_mainwid_spacing); // box coordinate
    if (box_test_x > c_seqarea_x || box_test_y > c_seqarea_y)
    {
        return -1;                          // right inactive side of area
    }

    x /= (c_seqarea_x + c_mainwid_spacing);
    y /= (c_seqarea_y + c_mainwid_spacing);
    int sequence =
    (
        (x*c_mainwnd_rows + y) + (m_screenset*c_mainwnd_rows*c_mainwnd_cols)
    );
    return sequence;
}

/**
 *  This function redraws everything and queues up a redraw operation.
 */

void
mainwid::reset()
{
    draw_sequences_on_pixmap();
    draw_pixmap_on_window();
}

/**
 *  Set the current screen set.
 */

void
mainwid::set_screenset (int a_ss)
{
    m_screenset = a_ss;
    if (m_screenset < 0)
        m_screenset = c_max_sets - 1;

    if (m_screenset >= c_max_sets)
        m_screenset = 0;

    m_mainperf->set_offset(m_screenset);
    reset();
}

/*
 * Event-handler section:
 */

/**
 *  For this GTK callback, on realization of window, initialize the shiz.
 *  It allocates any additional resources that weren't initialized in the
 *  constructor.
 */

void
mainwid::on_realize ()
{
    Gtk::DrawingArea::on_realize();         // base-class on_realize()
    set_flags(Gtk::CAN_FOCUS);
    m_window = get_window();                // more resources
    m_gc = Gdk::GC::create(m_window);       // graphics context?
    m_window->clear();
    p_font_renderer->init(m_window);
    m_pixmap = Gdk::Pixmap::create(m_window, c_mainwid_x, c_mainwid_y, -1);
    fill_background_window();
    draw_sequences_on_pixmap();
}

/**
 *  Implements the GTK expose event callback.
 */

bool
mainwid::on_expose_event (GdkEventExpose * a_e)
{
    m_window->draw_drawable
    (
        m_gc, m_pixmap,
        a_e->area.x, a_e->area.y,
        a_e->area.x, a_e->area.y,
        a_e->area.width,
        a_e->area.height
    );
    return true;
}

/**
 *  Handles a press of a mouse button.  It grabs the focus, calculates
 *  the pattern/sequence over which the button press occureed, and sets
 *  the m_button_down flag if it is over a pattern.
 */

bool
mainwid::on_button_press_event (GdkEventButton * a_p0)
{
    grab_focus();
    m_current_seq = seq_from_xy((int) a_p0->x, (int) a_p0->y);
    if (m_current_seq != -1 && a_p0->button == 1)
        m_button_down = true;

    return true;
}

/**
 *  Handles a release of a mouse button.  This event is a lot more complex
 *  than a press.
 */

bool
mainwid::on_button_release_event (GdkEventButton * a_p0)
{
    m_current_seq = seq_from_xy((int) a_p0->x, (int) a_p0->y);
    m_button_down = false;

    /*
     * Have we hit a sequence with the L button?  Toggle its play mode.
     */

    if (m_current_seq != -1 && a_p0->button == 1 && ! m_moving)
    {
        if (m_mainperf->is_active(m_current_seq))
        {
            m_mainperf->sequence_playing_toggle(m_current_seq);
            draw_sequence_on_pixmap(m_current_seq);
            draw_sequence_pixmap_on_window(m_current_seq);      // effective?
        }
    }
    if (a_p0->button == 1 && m_moving)
    {
        m_moving = false;
        if          // if we're in a pattern, it is active, and in edit mode...
        (
            ! m_mainperf->is_active(m_current_seq) && m_current_seq != -1 &&
            ! m_mainperf->is_sequence_in_edit(m_current_seq)
        )
        {
            m_mainperf->new_sequence(m_current_seq);
            *(m_mainperf->get_sequence(m_current_seq)) = m_moving_seq;
            draw_sequence_on_pixmap(m_current_seq);
            draw_sequence_pixmap_on_window(m_current_seq);      // effective?
        }
        else
        {
            m_mainperf->new_sequence(m_old_seq);
            *(m_mainperf->get_sequence(m_old_seq)) = m_moving_seq;
            draw_sequence_on_pixmap(m_old_seq);
            draw_sequence_pixmap_on_window(m_old_seq);          // effective?
        }
    }
    if (m_current_seq != -1 && a_p0->button == 3)       // launch menu (R button)
        popup_menu();

    return true;
}

/**
 *  Handle the motion of the mouse if a mouse button is down and in
 *  another sequence and if the current sequence is not in edit mode.
 */

bool
mainwid::on_motion_notify_event (GdkEventMotion * a_p0)
{
    int seq = seq_from_xy((int) a_p0->x, (int) a_p0->y);
    if (m_button_down)
    {
        if
        (
            seq != m_current_seq && ! m_moving &&
            ! m_mainperf->is_sequence_in_edit(m_current_seq)
        )
        {
            if (m_mainperf->is_active(m_current_seq))
            {
                m_old_seq = m_current_seq;
                m_moving = true;
                m_moving_seq = *(m_mainperf->get_sequence(m_current_seq));
                m_mainperf->delete_sequence(m_current_seq);
                draw_sequence_on_pixmap(m_current_seq);
                draw_sequence_pixmap_on_window(m_current_seq);  // effective?
            }
        }
    }
    return true;
}

/**
 *  Handles an on-focus event.  Just sets the Gtk::HAS_FOCUS flag.
 */

bool
mainwid::on_focus_in_event (GdkEventFocus *)
{
    set_flags(Gtk::HAS_FOCUS);
    return false;
}

/**
 *  Handles an out-of-focus event.  Just unsets the Gtk::HAS_FOCUS flag.
 */

bool
mainwid::on_focus_out_event (GdkEventFocus *)
{
    unset_flags(Gtk::HAS_FOCUS);
    return false;
}

/*
 * mainwid.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
