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
 * \file          fruityseq.cpp
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
 *  This code was extracted from seqevent to make that module more
 *  manageable.
 */

#include <gdkmm/cursor.h>
#include <gtkmm/button.h>

#include "fruityseq.h"
#include "seqevent.h"
#include "sequence.h"                  // needed for full usage of seqevent

/**
 *  Provides support for a context-sensitive mouse.
 */

void
FruitySeqEventInput::updateMousePtr (seqevent & seqev)
{
    long tick_s, tick_w, tick_f, pos;
    seqev.convert_x(seqev.m_current_x, &tick_s);
    seqev.convert_x(c_eventevent_x, &tick_w);
    tick_f = tick_s + tick_w;
    if (tick_s < 0)
        tick_s = 0;                    // clamp to 0

    if (m_is_drag_pasting || seqev.m_selecting || seqev.m_moving || seqev.m_paste)
        seqev.get_window()->set_cursor(Gdk::Cursor(Gdk::LEFT_PTR));
    else if (seqev.m_seq->intersectEvents(tick_s, tick_f, seqev.m_status, pos))
        seqev.get_window()->set_cursor(Gdk::Cursor(Gdk::CENTER_PTR));
    else
        seqev.get_window()->set_cursor(Gdk::Cursor(Gdk::PENCIL));
}

/**
 *  Implements the on-button-press event callback.
 */

bool
FruitySeqEventInput::on_button_press_event
(
    GdkEventButton * a_ev, seqevent & seqev
)
{
    int x, w, numsel;
    long tick_s;
    long tick_f;
    long tick_w;
    seqev.convert_x(c_eventevent_x, &tick_w);

    /*
     * Set the values for dragging, then reset the box that holds the
     * dirty redraw spot.
     */

    seqev.m_drop_x = seqev.m_current_x = (int) a_ev->x + seqev.m_scroll_offset_x;
    seqev.m_old.x = 0;
    seqev.m_old.y = 0;
    seqev.m_old.width = 0;
    seqev.m_old.height = 0;
    if (seqev.m_paste)
    {
        seqev.snap_x(&seqev.m_current_x);
        seqev.convert_x(seqev.m_current_x, &tick_s);
        seqev.m_paste = false;
        seqev.m_seq->push_undo();
        seqev.m_seq->paste_selected(tick_s, 0);
    }
    else
    {

        if (a_ev->button == 1)                      /* left mouse button     */
        {
            seqev.convert_x(seqev.m_drop_x, &tick_s);  /* x,y into tick/note */

            tick_f = tick_s + (seqev.m_zoom);       /* shift back some ticks */
            tick_s -= (tick_w);
            if (tick_s < 0)
                tick_s = 0;

            if
            (
                ! seqev.m_seq->select_events(tick_s, tick_f,
                       seqev.m_status, seqev.m_cc, sequence::e_would_select) &&
                ! (a_ev->state & GDK_CONTROL_MASK)
            )
            {
                seqev.m_painting = true;
                seqev.snap_x(&seqev.m_drop_x);
                seqev.convert_x(seqev.m_drop_x, &tick_s); /* x,y into tick/note */

                /* add note, length = a little less than snap */

                if
                (
                    ! seqev.m_seq->select_events(tick_s, tick_f,
                           seqev.m_status, seqev.m_cc, sequence::e_would_select)
                )
                {
                    seqev.m_seq->push_undo();
                    seqev.drop_event(tick_s);
                }
            }
            else                                    /* selecting */
            {
                if
                (
                    ! seqev.m_seq->select_events(tick_s, tick_f,
                           seqev.m_status, seqev.m_cc, sequence::e_is_selected)
                )
                {
                    if                              /* if clicking event */
                    (
                        seqev.m_seq->select_events(tick_s, tick_f,
                             seqev.m_status, seqev.m_cc, sequence::e_would_select)
                    )
                    {
                        if (!(a_ev->state & GDK_CONTROL_MASK))
                            seqev.m_seq->unselect();
                    }
                    else                            /* if clicking empty space */
                    {
                        /* ... unselect all if ctrl-shift not held */

                        if
                        (
                            ! ((a_ev->state & GDK_CONTROL_MASK) &&
                                (a_ev->state & GDK_SHIFT_MASK))
                        )
                        {
                            seqev.m_seq->unselect();
                        }
                    }

                    /* on direct click select only one event */

                    numsel = seqev.m_seq->select_events
                    (
                        tick_s, tick_f,
                        seqev.m_status,
                        seqev.m_cc, sequence::e_select_one
                    );

                    /* prevent deselect in button_release() */

                    if (numsel)
                        m_justselected_one = true;

                    /* if nothing selected, start the selection box */

                    if (numsel == 0 && (a_ev->state & GDK_CONTROL_MASK))
                        seqev.m_selecting = true;
                }

                if                      /* if event under cursor is selected */
                (
                    seqev.m_seq->select_events(tick_s, tick_f,
                         seqev.m_status, seqev.m_cc, sequence::e_is_selected)
                )
                {
                    if (!(a_ev->state & GDK_CONTROL_MASK)) /* grab/move note */
                    {
                        seqev.m_moving_init = true;
                        int note;

                        /* get the box that selected elements are in */

                        seqev.m_seq->get_selected_box
                        (
                            &tick_s, &note, &tick_f, &note
                        );
                        tick_f += tick_w;

                        seqev.convert_t(tick_s, &x); /* convert to X,Y values */
                        seqev.convert_t(tick_f, &w);
                        w = w - x;                   /* w is coordinates now */

                        /* set the m_selected rectangle for x,y,w,h */

                        seqev.m_selected.x = x;
                        seqev.m_selected.width = w;
                        seqev.m_selected.y = (c_eventarea_y-c_eventevent_y) / 2;
                        seqev.m_selected.height = c_eventevent_y;

                        /* save offset that we get from the snap above */

                        int adjusted_selected_x = seqev.m_selected.x;
                        seqev.snap_x(&adjusted_selected_x);
                        seqev.m_move_snap_offset_x =
                            seqev.m_selected.x - adjusted_selected_x;

                        /* align selection for drawing */

                        seqev.snap_x(&seqev.m_selected.x);
                        seqev.snap_x(&seqev.m_current_x);
                        seqev.snap_x(&seqev.m_drop_x);
                    }
                    else if /* ctrl left click when stuff already selected */
                    (
                        (a_ev->state & GDK_CONTROL_MASK) &&
                        seqev.m_seq->select_events(tick_s, tick_f,
                           seqev. m_status, seqev.m_cc, sequence::e_is_selected)
                    )
                    {
                        m_is_drag_pasting_start = true;
                    }
                }
            }

        } /* end if button == 1 */

        if (a_ev->button == 3)                        /* right button         */
        {
            seqev.convert_x(seqev.m_drop_x, &tick_s); /* x,y in to tick/note  */

            tick_f = tick_s + (seqev.m_zoom);         /* shift back some ticks */
            tick_s -= (tick_w);
            if (tick_s < 0)
                tick_s = 0;

            if                  /* erase event under cursor if there is one */
            (
                seqev.m_seq->select_events(tick_s, tick_f,
                     seqev.m_status, seqev.m_cc, sequence::e_would_select)
            )
            {
                /* remove only note under cursor, leave selection intact */

                seqev.m_seq->push_undo();
                seqev.m_seq->select_events
                (
                    tick_s, tick_f,
                    seqev.m_status, seqev.m_cc, sequence::e_remove_one
                );
                seqev.redraw();
                seqev.m_seq->set_dirty();
            }
            else                                        /* selecting          */
            {
                if (!(a_ev->state & GDK_CONTROL_MASK))
                    seqev.m_seq->unselect();

                /* nothing selected, start the selection box */

                seqev.m_selecting = true;
            }
        }
    }
    seqev.update_pixmap();              /* if they clicked, something changed */
    seqev.draw_pixmap_on_window();
    updateMousePtr(seqev);
    return true;
}

/**
 *  Implements the on-button-release callback.
 */

bool
FruitySeqEventInput::on_button_release_event
(
    GdkEventButton * a_ev, seqevent & seqev
)
{
    long tick_s;
    long tick_f;
    int x, w;
    seqev.grab_focus();
    seqev.m_current_x = (int) a_ev->x  + seqev.m_scroll_offset_x;;
    if (seqev.m_moving || m_is_drag_pasting)
        seqev.snap_x(&seqev.m_current_x);

    int delta_x = seqev.m_current_x - seqev.m_drop_x;
    long delta_tick;
    if (a_ev->button == 1)
    {
        int current_x = seqev.m_current_x;
        long t_s, t_f;
        seqev.snap_x(&current_x);
        seqev.convert_x(current_x, &t_s);
        t_f = t_s + (seqev.m_zoom);                 /* shift back a few ticks */
        if (t_s < 0)
            t_s = 0;

        /*
         * Use the ctrl-left click button up for select/drag copy/paste;
         * use the left click button up for ending a move of selected notes.
         */

        if (m_is_drag_pasting)
        {
            m_is_drag_pasting = false;
            m_is_drag_pasting_start = false;
            seqev.m_paste = false; /* convert deltas into screen coordinates */
            seqev.m_seq->push_undo();
            seqev.m_seq->paste_selected(t_s, 0);
        }

        /* ctrl-left click but without movement - select a note */

        if (m_is_drag_pasting_start)
        {
            m_is_drag_pasting_start = false;

            /*
             * If a ctrl-left click without movement and if note under
             * cursor is selected, and ctrl is held and button-down didn't
             * just select one.
             */

            if                                  /* deselect the event? */
            (
                ! m_justselected_one &&
                seqev.m_seq->select_events
                (
                    t_s, t_f,
                    seqev.m_status, seqev.m_cc, sequence::e_is_selected
                ) &&
                (a_ev->state & GDK_CONTROL_MASK)
            )
            {
                (void) seqev.m_seq->select_events
                (
                    t_s, t_f,
                    seqev.m_status, seqev.m_cc, sequence::e_deselect
                );
            }
        }
        m_justselected_one = false;         /* clear flag on left button up */
        if (seqev.m_moving)
        {
            delta_x -= seqev.m_move_snap_offset_x; /* adjust for snap */
            seqev.convert_x(delta_x, &delta_tick); /* deltas to screen coords */
            seqev.m_seq->push_undo();              /* still moves events */
            seqev.m_seq->move_selected_notes(delta_tick, 0);
        }
    }
    if (a_ev->button == 3 || a_ev->button == 1)
    {
        if (seqev.m_selecting)
        {
            seqev.x_to_w(seqev.m_drop_x, seqev.m_current_x, &x, &w);
            seqev.convert_x(x,   &tick_s);
            seqev.convert_x(x + w, &tick_f);
            (void) seqev.m_seq->select_events
            (
                tick_s, tick_f,
                seqev.m_status, seqev.m_cc, sequence::e_toggle_selection
            );
        }
    }

    /* turn off */

    seqev.m_selecting = false;
    seqev.m_moving = false;
    seqev.m_growing = false;
    seqev.m_moving_init = false;
    seqev.m_painting = false;
    seqev.m_seq->unpaint_all();

    /* if they clicked, something changed */

    seqev.update_pixmap();
    seqev.draw_pixmap_on_window();
    updateMousePtr(seqev);
    return true;
}

/**
 *  Implements the on-motion-notify callback.
 */

bool
FruitySeqEventInput::on_motion_notify_event
(
    GdkEventMotion * a_ev, seqevent & seqev
)
{
    long tick = 0;
    seqev.m_current_x = (int) a_ev->x  + seqev.m_scroll_offset_x;
    if (seqev.m_moving_init)
    {
        seqev.m_moving_init = false;
        seqev.m_moving = true;
    }
    updateMousePtr(seqev);      /* context sensitive mouse pointer... */

    /* ctrl-left click drag on selected note(s) starts a copy/unselect/paste */

    if (m_is_drag_pasting_start)
    {
        seqev.m_seq->copy_selected();
        seqev.m_seq->unselect();
        seqev.start_paste();
        m_is_drag_pasting_start = false;
        m_is_drag_pasting = true;
    }
    if (seqev.m_selecting || seqev.m_moving || seqev.m_paste)
    {
        if (seqev.m_moving || seqev.m_paste)
            seqev.snap_x(&seqev.m_current_x);

        seqev.draw_selection_on_window();
    }
    if (seqev.m_painting)
    {
        seqev.m_current_x = (int) a_ev->x  + seqev.m_scroll_offset_x;;
        seqev.snap_x(&seqev.m_current_x);
        seqev.convert_x(seqev.m_current_x, &tick);
        seqev.drop_event(tick);
    }
    return true;
}

/*
 * fruityseq.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
