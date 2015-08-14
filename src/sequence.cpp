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
 * \file          sequence.cpp
 *
 *  This module declares/defines the base class for handling
 *  patterns/sequences.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-12
 * \license       GNU GPLv2 or above
 *
 */

#include <stdlib.h>
#include <gtkmm/combo.h>               // Gtk::Entry, ToggleButton, RadioButton

#include "mastermidibus.h"
#include "sequence.h"
#include "seqedit.h"

/**
 *  A static clipboard for holding pattern/sequence events.
 */

sequence::EventList sequence::m_list_clipboard;

/**
 *  Principal constructor.
 */

sequence::sequence ()
 :
    m_list_event                (),
    m_list_trigger              (),
    m_trigger_clipboard         (),
    m_list_undo                 (),
    m_list_redo                 (),
    m_list_trigger_undo         (),
    m_list_trigger_redo         (),
    m_iterator_play             (),
    m_iterator_draw             (),
    m_iterator_play_trigger     (),
    m_iterator_draw_trigger     (),
    m_midi_channel              (0),
    m_bus                       (0),
    m_song_mute                 (false),
    m_notes_on                  (0),
    m_masterbus                 (nullptr),
    m_playing_notes             (),             // an array
    m_was_playing               (false),
    m_playing                   (false),
    m_recording                 (false),
    m_quantized_rec             (false),
    m_thru                      (false),
    m_queued                    (false),
    m_trigger_copied            (false),
    m_dirty_main                (true),
    m_dirty_edit                (true),
    m_dirty_perf                (true),
    m_dirty_names               (true),
    m_editing                   (false),
    m_raise                     (false),
    m_name                      (c_dummy),
    m_last_tick                 (0),
    m_queued_tick               (0),
    m_trigger_offset            (0),
    m_length                    (4*c_ppqn),
    m_snap_tick                 (c_ppqn/4),
    m_time_beats_per_measure    (4),
    m_time_beat_width           (4),
    m_rec_vol                   (0),
    m_mutex                     ()
{
    for (int i = 0; i < c_midi_notes; i++)      /* no notes are playing */
        m_playing_notes[i] = 0;
}

/**
 *  A rote destructor.
 */

sequence::~sequence ()
{
    // Empty body
}

/**
 *  Principal assignment operator.  Follows the stock rules for such an
 *  operator, but does a little more then just assign member values.
 *
 * \threadsafe
 */

sequence &
sequence::operator = (const sequence & a_rhs)
{
    lock();
    if (this != &a_rhs)
    {
        m_list_event   = a_rhs.m_list_event;
        m_list_trigger = a_rhs.m_list_trigger;
        m_midi_channel = a_rhs.m_midi_channel;
        m_masterbus    = a_rhs.m_masterbus;
        m_bus          = a_rhs.m_bus;
        m_name         = a_rhs.m_name;
        m_length       = a_rhs.m_length;
        m_playing      = false;
        m_time_beats_per_measure = a_rhs.m_time_beats_per_measure;
        m_time_beat_width = a_rhs.m_time_beat_width;
        for (int i = 0; i < c_midi_notes; i++)      /* no notes are playing */
            m_playing_notes[i] = 0;

        zero_markers();                             /* reset */
    }
    verify_and_link();
    unlock();
    return *this;
}

/**
 *  Pushes the list-event into the undo-list.
 *
 * \threadsafe
 */

void
sequence::push_undo ()
{
    lock();
    m_list_undo.push(m_list_event);
    unlock();
}

/**
 *  If there are items on the undo list, this function pushes the
 *  list-event into the redo-list, puts the top of the undo-list into the
 *  list-event, pops from the undo-list, calls verify_and_link(), and then
 *  calls unselect.
 *
 * \threadsafe
 */

void
sequence::pop_undo ()
{
    lock();
    if (m_list_undo.size() > 0)
    {
        m_list_redo.push(m_list_event);
        m_list_event = m_list_undo.top();
        m_list_undo.pop();
        verify_and_link();
        unselect();
    }
    unlock();
}

/**
 *  If there are items on the redo list, this function pushes the
 *  list-event into the undo-list, puts the top of the redo-list into the
 *  list-event, pops from the redo-list, calls verify_and_link(), and then
 *  calls unselect.
 *
 * \threadsafe
 */

void
sequence::pop_redo ()
{
    lock();
    if (m_list_redo.size() > 0)
    {
        m_list_undo.push(m_list_event);
        m_list_event = m_list_redo.top();
        m_list_redo.pop();
        verify_and_link();
        unselect();
    }
    unlock();
}

/**
 *  Pushes the list-trigger into the trigger undo-list, then flags each
 *  item in the undo-list as unselected.
 *
 * \threadsafe
 */

void
sequence::push_trigger_undo ()
{
    lock();
    m_list_trigger_undo.push(m_list_trigger);
    TriggerList::iterator i;
    for
    (
        i  = m_list_trigger_undo.top().begin();
        i != m_list_trigger_undo.top().end(); i++
    )
    {
        i->m_selected = false;
    }
    unlock();
}

/**
 *  If the trigger undo-list has any items, the list-trigger is pushed
 *  9nto the redo list, the top of the undo-list is coped into the
 *  list-trigger, and then pops from the undo-list.
 */

void
sequence::pop_trigger_undo ()
{
    lock();
    if (m_list_trigger_undo.size() > 0)
    {
        m_list_trigger_redo.push(m_list_trigger);
        m_list_trigger = m_list_trigger_undo.top();
        m_list_trigger_undo.pop();
    }
    unlock();
}

/**
 * \setter m_masterbux
 *
 * \threadsafe
 */

void
sequence::set_master_midi_bus (mastermidibus * a_mmb)
{
    lock();
    m_masterbus = a_mmb;
    unlock();
}

/**
 * \setter m_time_beats_per_measure
 *
 * \threadsafe
 */

void
sequence::set_bpm (long a_beats_per_measure)
{
    lock();
    m_time_beats_per_measure = a_beats_per_measure;
    set_dirty_mp();
    unlock();
}

/**
 * \setter m_time_beat_width
 *
 * \threadsafe
 */

void
sequence::set_bw (long a_beat_width)
{
    lock();
    m_time_beat_width = a_beat_width;
    set_dirty_mp();
    unlock();
}

/**
 * \setter m_rec_vol
 *
 * \threadsafe
 */

void
sequence::set_rec_vol (long a_rec_vol)
{
    lock();
    m_rec_vol = a_rec_vol;
    unlock();
}

/**
 *  Adds an event to the internal event list in a sorted manner.  Then it
 *  reset the draw-marker and sets the dirty flag.
 *
 * \threadsafe
 */

void
sequence::add_event (const event * a_e)
{
    lock();
    m_list_event.push_front(*a_e);
    m_list_event.sort();
    reset_draw_marker();
    set_dirty();
    unlock();
}

/**
 * \setter m_last_tick
 *
 * \threadsafe
 */

void
sequence::set_orig_tick (long a_tick)
{
    lock();
    m_last_tick = a_tick;
    unlock();
}

/**
 * \setter m_queued and m_queued_tick
 *
 *  Toggles the queued flag and sets the dirty-mp flag.  Also calculated
 *  the queued tick based on m_last_tick.
 *
 * \threadsafe
 */

void
sequence::toggle_queued ()
{
    lock();
    set_dirty_mp();
    m_queued = ! m_queued;
    m_queued_tick = m_last_tick - (m_last_tick % m_length) + m_length;
    unlock();
}

/**
 * \setter m_queued
 *
 *  Toggles the queued flag and sets the dirty-mp flag.
 *
 * \threadsafe
 */

void
sequence::off_queued ()
{
    lock();
    set_dirty_mp();
    m_queued = false;
    unlock();
}

/**
 *  The play() function dumps notes starting from thee given tick, and it
 *  prebuffers ahead.  This function is called by the sequencer thread,
 *  performance.  The tick comes in as global tick.
 *
 *  It turns the sequence off after we play in this frame.
 *
 * \threadsafe
 */

void
sequence::play (long a_tick, bool a_playback_mode)
{
    lock();
    bool trigger_turning_off = false;       /* turns off after frame play */
    long times_played  = m_last_tick / m_length;
    long offset_base   = times_played * m_length;
    long start_tick = m_last_tick;
    long end_tick = a_tick;
    long trigger_offset = 0;
    if (m_song_mute)
    {
        set_playing(false);
    }
    else
    {
        if (a_playback_mode)        /* if using in-sequence on/off triggers */
        {
            bool trigger_state = false;
            long trigger_tick = 0;
            TriggerList::iterator i = m_list_trigger.begin();
            while (i != m_list_trigger.end())
            {
                if (i->m_tick_start <= end_tick)
                {
                    trigger_state = true;
                    trigger_tick = i->m_tick_start;
                    trigger_offset = i->m_offset;
                }
                if (i->m_tick_end <= end_tick)
                {
                    trigger_state = false;
                    trigger_tick = i->m_tick_end;
                    trigger_offset = i->m_offset;
                }
                if (i->m_tick_start > end_tick || i->m_tick_end > end_tick)
                    break;

                i++;
            }

            /* had triggers in the slice, and not equal to current state */

            if (trigger_state != m_playing)
            {
                if (trigger_state)                  /* we are turning on */
                {
                    if (trigger_tick < m_last_tick)
                        start_tick = m_last_tick;
                    else
                        start_tick = trigger_tick;

                    set_playing(true);
                }
                else
                {
                    end_tick = trigger_tick;        /* on, and turning off */
                    trigger_turning_off = true;
                }
            }
            if (m_list_trigger.size() == 0 && m_playing)
                set_playing(false);
        }
    }
    set_trigger_offset(trigger_offset);

    long start_tick_offset = (start_tick + m_length - m_trigger_offset);
    long end_tick_offset = (end_tick + m_length - m_trigger_offset);
    if (m_playing)                              /* play the notes in frame */
    {
        EventList::iterator e = m_list_event.begin();
        while (e != m_list_event.end())
        {
            if ((e->get_timestamp() + offset_base) >= (start_tick_offset) &&
                    (e->get_timestamp() + offset_base) <= (end_tick_offset))
            {
                put_event_on_bus(&(*e));
            }
            else if ((e->get_timestamp() + offset_base) >  end_tick_offset)
            {
                break;
            }
            e++;                                    /* advance              */
            if (e == m_list_event.end())            /* did we hit the end ? */
            {
                e = m_list_event.begin();
                offset_base += m_length;
            }
        }
    }
    if (trigger_turning_off)         /* if triggers said we should turn off */
        set_playing(false);

    m_last_tick = end_tick + 1;                     /* update for next frame */
    m_was_playing = m_playing;
    unlock();
}

/**
 *  Resets everything to zero.  This function is used when the sequencer stops.
 *
 * \threadsafe
 */

void
sequence::zero_markers ()
{
    lock();
    m_last_tick = 0;            // m_masterbus->flush( );
    unlock();
}


/**
 *  This function verifies state: all note-ons have an off, and it links
 *  note-offs with their note-ons.
 *
 * \threadsafe
 */

void
sequence::verify_and_link ()
{
    EventList::iterator i;
    EventList::iterator on;
    EventList::iterator off;
    bool end_found = false;

    lock();
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        i->clear_link();
        i->unmark();
    }
    on = m_list_event.begin();
    while (on != m_list_event.end())                /* pair ons and offs */
    {
        if (on->is_note_on())          /* note on, look for its note off */
        {
            off = on;                       /* get next possible off node */
            off++;
            end_found = false;
            while (off != m_list_event.end())
            {
                if          /* is a off event, == notes, and isn't marked  */
                (
                    off->is_note_off() &&
                    off->get_note() == on->get_note() &&
                    ! off->is_marked()
                )
                {
                    on->link(&(*off));                    /* link + mark */
                    off->link(&(*on));
                    on->mark();
                    off->mark();
                    end_found = true;
                    break;
                }
                off++;
            }
            if (!end_found)
            {
                off = m_list_event.begin();
                while (off != on)
                {
                    if
                    (
                        off->is_note_off() &&
                        off->get_note() == on->get_note() &&
                        ! off->is_marked()
                    )
                    {
                        on->link(&(*off));                /* link + mark */
                        off->link(&(*on));
                        on->mark();
                        off->mark();
                        end_found = true;
                        break;
                    }
                    off++;
                }
            }
        }
        on++;
    }
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
        i->unmark();                                      /* unmark all */

    /* kill those events not in range */

    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        /* if our current time stamp is greater then the length */

        if (i->get_timestamp() >= m_length || i->get_timestamp() < 0)
        {
            i->mark();                            /* we have to prune it */
            if (i->is_linked())
                i->get_linked()->mark();
        }
    }
    remove_marked();
    unlock();
}

/**
 *  Links a new event.
 *
 * \threadsafe
 */

void
sequence::link_new ()
{
    EventList::iterator off;
    bool end_found = false;

    lock();
    EventList::iterator on = m_list_event.begin();
    while (on != m_list_event.end())                /* pair ons and offs */
    {
        /* check for a note on, then look for its note off */

        if (on->is_note_on() && !on->is_linked())
        {
            off = on;
            off++;                                   /* get next element */
            end_found = false;
            while (off != m_list_event.end())
            {
                /* is a off event, == notes, and isn't selected  */

                if
                (
                    off->is_note_off() &&
                    off->get_note() == on->get_note() &&
                    ! off->is_linked()
                )
                {
                    on->link(&(*off));                        /* link */
                    off->link(&(*on));
                    end_found = true;
                    break;
                }
                off++;
            }
            if (! end_found)
            {
                off = m_list_event.begin();
                while (off != on)
                {
                    /* is a off event, == notes, and isn't selected  */

                    if
                    (
                        off->is_note_off() &&
                        off->get_note() == on->get_note() &&
                        ! off->is_linked()
                    )
                    {
                        on->link(&(*off));                    /* link */
                        off->link(&(*on));
                        end_found = true;
                        break;
                    }
                    off++;
                }
            }
        }
        on++;
    }
    unlock();
}

/**
 *  A helper function, which does not lock/unlock, so it is unsafe to call
 *  without supplying an iterator from the list-event.
 *
 *  If it's a note off, and that note is currently playing, the send a
 *  note off.
 */

void
sequence::remove (EventList::iterator i)
{
    if (i->is_note_off()  && m_playing_notes[i->get_note()] > 0)
    {
        m_masterbus->play(m_bus, &(*i), m_midi_channel);
        m_playing_notes[i->get_note()]--;
    }
    m_list_event.erase(i);
}

/**
 *  A helper function, which does not lock/unlock, so it is unsafe to call
 *  without supplying an iterator from the list-event.
 *
 *  Finds the given event in m_list_event, and removes the first iterator
 *  matching that.
 */

void
sequence::remove (event * e)
{
    EventList::iterator i = m_list_event.begin();
    while (i != m_list_event.end())
    {
        if (e == &(*i))                 /* comparing pointers */
        {
            remove(i);
            return;
        }
        ++i;
    }
}

/**
 *  Removes marked events.
 *
 * \threadsafe
 */

void
sequence::remove_marked ()
{
    lock();
    EventList::iterator t;
    EventList::iterator i = m_list_event.begin();
    while (i != m_list_event.end())
    {
        if (i->is_marked())
        {
            t = i;
            t++;
            remove(i);
            i = t;
        }
        else
            i++;
    }
    reset_draw_marker();
    unlock();
}

/**
 *  Marks the selected events.
 *
 * \threadsafe
 */

void
sequence::mark_selected ()
{
    lock();
    EventList::iterator i = m_list_event.begin();
    while (i != m_list_event.end())         // what's wrong with a for-loop?
    {
        if (i->is_selected())
            i->mark();

        ++i;
    }
    reset_draw_marker();
    unlock();
}

/**
 *  Unpaints all list-events.
 *
 * \threadsafe
 */

void
sequence::unpaint_all ()
{
    lock();
    EventList::iterator i = m_list_event.begin();
    while (i != m_list_event.end())         // what's wrong with a for-loop?
    {
        i->unpaint();
        i++;
    }
    unlock();
}

/**
 *  Returns the 'box' of the selected items.
 */

void
sequence::get_selected_box
(
    long * a_tick_s, int * a_note_h, long * a_tick_f, int * a_note_l)
{
    lock();
    *a_tick_s = c_maxbeats * c_ppqn;
    *a_tick_f = 0;
    *a_note_h = 0;
    *a_note_l = 128;
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_selected())
        {
            long time = i->get_timestamp();

            /*
             * can't check on/off here, it screws up the seqevent
             * selection, which has no "off".
             */

            if (time < *a_tick_s)
                *a_tick_s = time;

            if (time > *a_tick_f)
                *a_tick_f = time;

            int note = i->get_note();
            if (note < *a_note_l)
                *a_note_l = note;

            if (note > *a_note_h)
                *a_note_h = note;
        }
    }
    unlock();
}

/**
 *  Returns the 'box' of selected items.
 */

void
sequence::get_clipboard_box
(
    long * a_tick_s, int * a_note_h, long * a_tick_f, int * a_note_l
)
{
    lock();
    *a_tick_s = c_maxbeats * c_ppqn;
    *a_tick_f = 0;
    *a_note_h = 0;
    *a_note_l = 128;
    if (m_list_clipboard.size() == 0)
    {
        *a_tick_s = *a_tick_f = *a_note_h = *a_note_l = 0;
    }

    EventList::iterator i;
    for (i = m_list_clipboard.begin(); i != m_list_clipboard.end(); i++)
    {
        long time = i->get_timestamp();
        if (time < *a_tick_s)
            *a_tick_s = time;

        if (time > *a_tick_f)
            *a_tick_f = time;

        int note = i->get_note();
        if (note < *a_note_l)
            *a_note_l = note;

        if (note > *a_note_h)
            *a_note_h = note;
    }
    unlock();
}

/**
 *  Counts the selected notes in the event list.
 *
 * \threadsafe
 */

int
sequence::get_num_selected_notes ()
{
    int result = 0;
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_note_on() && i->is_selected())
            result++;
    }
    unlock();
    return result;
}

/**
 *  Counts the selected events, with the given status, in the event list.
 *  If the event is a control change (CC), then it must also match the
 *  given CC value.
 *
 * \threadsafe
 */

int
sequence::get_num_selected_events (unsigned char a_status, unsigned char a_cc)
{
    int result = 0;
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->get_status() == a_status)
        {
            unsigned char d0, d1;
            i->get_data(&d0, &d1);            /* get the two data bytes */
            if
            (
                (a_status == EVENT_CONTROL_CHANGE && d0 == a_cc) ||
                (a_status != EVENT_CONTROL_CHANGE))
            {
                if (i->is_selected())
                    result++;
            }
        }
    }
    unlock();
    return result;
}

/**
 *  This function selects events in range of tick start, note high, tick end,
 *  and note low.  Returns the number selected.
 *
 * \threadsafe
 */

int
sequence::select_note_events
(
    long a_tick_s, int a_note_h,
    long a_tick_f, int a_note_l, select_action_e a_action
)
{
    int result = 0;
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->get_note() <= a_note_h && i->get_note() >= a_note_l)
        {
            long tick_s = 0;            // must be initialized
            long tick_f = 0;            // must be initialized
            if (i->is_linked())
            {
                event * ev = i->get_linked();
                if (i->is_note_off())
                {
                    tick_s = ev->get_timestamp();
                    tick_f = i->get_timestamp();
                }
                if (i->is_note_on())
                {
                    tick_f = ev->get_timestamp();
                    tick_s = i->get_timestamp();
                }

                bool tick_and = (tick_s <= a_tick_f) && (tick_f >= a_tick_s);
                bool tick_or = (tick_s <= a_tick_f) || (tick_f >= a_tick_s);
                if
                (
                    ((tick_s <= tick_f) && tick_and) ||
                    ((tick_s > tick_f) && tick_or)
                )
                {
                    if (a_action == e_select || a_action == e_select_one)
                    {
                        i->select();
                        ev->select();
                        result++;
                        if (a_action == e_select_one)
                            break;
                    }
                    if (a_action == e_is_selected)
                    {
                        if (i->is_selected())
                        {
                            result = 1;
                            break;
                        }
                    }
                    if (a_action == e_would_select)
                    {
                        result = 1;
                        break;
                    }
                    if (a_action == e_deselect)
                    {
                        result = 0;
                        i->unselect();
                        ev->unselect();
                    }
                    if (a_action == e_toggle_selection &&
                            i->is_note_on()) // don't toggle twice
                    {
                        if (i->is_selected())
                        {
                            i->unselect();
                            ev->unselect();
                            result ++;
                        }
                        else
                        {
                            i->select();
                            ev->select();
                            result ++;
                        }
                    }
                    if (a_action == e_remove_one)
                    {
                        remove(&(*i));
                        remove(ev);
                        reset_draw_marker();
                        result++;
                        break;
                    }
                }
            }
            else
            {
                tick_s = tick_f = i->get_timestamp();
                if (tick_s  >= a_tick_s - 16 && tick_f <= a_tick_f)
                {
                    if (a_action == e_select || a_action == e_select_one)
                    {
                        i->select();
                        result++;
                        if (a_action == e_select_one)
                            break;
                    }
                    if (a_action == e_is_selected)
                    {
                        if (i->is_selected())
                        {
                            result = 1;
                            break;
                        }
                    }
                    if (a_action == e_would_select)
                    {
                        result = 1;
                        break;
                    }
                    if (a_action == e_deselect)
                    {
                        result = 0;
                        i->unselect();
                    }
                    if (a_action == e_toggle_selection)
                    {
                        if (i->is_selected())
                        {
                            i->unselect();
                            result ++;
                        }
                        else
                        {
                            i->select();
                            result ++;
                        }
                    }
                    if (a_action == e_remove_one)
                    {
                        remove(&(*i));
                        reset_draw_marker();
                        result++;
                        break;
                    }
                }
            }
        }
    }
    unlock();
    return result;
}

/**
 *  Select all events in the given range, and returns the number
 *  selected.  Note that there is also an overloaded version of this
 *  function.
 *
 * \threadsafe
 */

int
sequence::select_events
(
    long a_tick_s, long a_tick_f,
    unsigned char a_status, unsigned char a_cc, select_action_e a_action
)
{
    int result = 0;
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if
        (
            i->get_status() == a_status &&
            i->get_timestamp() >= a_tick_s &&
            i->get_timestamp() <= a_tick_f
        )
        {
            unsigned char d0, d1;
            i->get_data(&d0, &d1);
            if
            (
                (a_status == EVENT_CONTROL_CHANGE && d0 == a_cc) ||
                (a_status != EVENT_CONTROL_CHANGE))
            {
                if (a_action == e_select || a_action == e_select_one)
                {
                    i->select();
                    result++;
                    if (a_action == e_select_one)
                        break;
                }
                if (a_action == e_is_selected)
                {
                    if (i->is_selected())
                    {
                        result = 1;
                        break;
                    }
                }
                if (a_action == e_would_select)
                {
                    result = 1;
                    break;
                }
                if (a_action == e_toggle_selection)
                {
                    if (i->is_selected())
                        i->unselect();
                    else
                        i->select();
                }
                if (a_action == e_deselect)
                    i->unselect();

                if (a_action == e_remove_one)
                {
                    remove(&(*i));
                    reset_draw_marker();
                    result++;
                    break;
                }
            }
        }
    }
    unlock();
    return result;
}

/**
 *  Selects all events, unconditionally.
 *
 * \threadsafe
 */

void
sequence::select_all ()
{
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
        i->select();

    unlock();
}

/**
 *  Deselects all events, unconditionally.
 *
 * \threadsafe
 */

void
sequence::unselect ()
{
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
        i->unselect();

    unlock();
}

/**
 *  Removes and adds reads selected in position.
 */

void
sequence::move_selected_notes (long a_delta_tick, int a_delta_note)
{
    lock();
    mark_selected();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_marked())                       /* is it being moved ?i   */
        {
            event e = *i;                         /* copy event             */
            e.unmark();
            if
            (
                (e.get_note() + a_delta_note) >= 0 &&
                (e.get_note() + a_delta_note) < c_num_keys
            )
            {
                bool noteon = e.is_note_on();
                long timestamp = e.get_timestamp() + a_delta_tick;
                if (timestamp > m_length)
                    timestamp = timestamp - m_length;

                if (timestamp < 0)
                    timestamp = m_length + timestamp;

                if ((timestamp == 0) && !noteon)
                    timestamp = m_length - 2;

                if ((timestamp == m_length) && noteon)
                    timestamp = 0;

                e.set_timestamp(timestamp);
                e.set_note(e.get_note() + a_delta_note);
                e.select();
                add_event(&e);
            }
        }
    }
    remove_marked();
    verify_and_link();
    unlock();
}

/**
 *  Performs a stretch operation on the selected events.  This should move
 *  a note off event, according to old comments, but it doesn't seem to do
 *  that.  See the grow_selected() function.
 *
 * \threadsafe
 */

void
sequence::stretch_selected (long a_delta_tick)
{
    lock();
    int first_ev = 0x7fffffff;                  /* timestamp lower limit */
    int last_ev = 0x00000000;                   /* timestamp upper limit */
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_selected())
        {
            event * e = &(*i);
            if (e->get_timestamp() < first_ev)
                first_ev = e->get_timestamp();
            if (e->get_timestamp() > last_ev)
                last_ev = e->get_timestamp();
        }
    }
    int old_len = last_ev - first_ev;
    int new_len = old_len + a_delta_tick;
    if (new_len > 1)
    {
        float ratio = float(new_len) / float(old_len);
        mark_selected();
        for (i = m_list_event.begin(); i != m_list_event.end(); i++)
        {
            if (i->is_marked())
            {
                event * e = &(*i);                  /* copy & scale event */
                event new_e = *e;
                new_e.set_timestamp
                (
                    long((e->get_timestamp() - first_ev) * ratio) + first_ev
                );
                new_e.unmark();
                add_event(&new_e);
            }
        }
        remove_marked();
        verify_and_link();
    }
    unlock();
}

/**
 *  Moves note off event.
 *
 * \threadsafe
 */

void
sequence::grow_selected (long a_delta_tick)
{
    lock();
    mark_selected();

    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_marked() && i->is_note_on() && i->is_linked())
        {
            event * on = &(*i);
            event * off = i->get_linked();
            long length = off->get_timestamp() + a_delta_tick;

            /*
             * If timestamp + delta is greater that m_length, we do round
             * robin magic.
             */

            if (length > m_length)
                length = length - m_length;

            if (length < 0)
                length = m_length + length;

            if (length == 0)
                length = m_length - 2;

            on->unmark();
            event e  = *off;                        /* copy event */
            e.unmark();
            e.set_timestamp(length);
            add_event(&e);
        }
    }
    remove_marked();
    verify_and_link();
    unlock();
}

/**
 *  Increments events the match the given status and control values.
 *  The supported statuses are:
 *
 *      -   EVENT_NOTE_ON
 *      -   EVENT_NOTE_OFF
 *      -   EVENT_AFTERTOUCH
 *      -   EVENT_CONTROL_CHANGE
 *      -   EVENT_PITCH_WHEEL
 *      -   EVENT_PROGRAM_CHANGE
 *      -   EVENT_CHANNEL_PRESSURE
 *
 * \threadsafe
 */

void
sequence::increment_selected (unsigned char a_stat, unsigned char /*a_control*/)
{
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_selected() && i->get_status() == a_stat)
        {
            if
            (
                a_stat == EVENT_NOTE_ON || a_stat == EVENT_NOTE_OFF ||
                a_stat == EVENT_CONTROL_CHANGE || a_stat == EVENT_PITCH_WHEEL ||
                a_stat == EVENT_AFTERTOUCH
            )
            {
                i->increment_data2();
            }
            else if
            (
                a_stat == EVENT_PROGRAM_CHANGE ||
                a_stat == EVENT_CHANNEL_PRESSURE
            )
            {
                i->increment_data1();
            }
        }
    }
    unlock();
}

/**
 *  Decrements events the match the given status and control values.
 *  The supported statuses are:
 *
 *      -   EVENT_NOTE_ON
 *      -   EVENT_NOTE_OFF
 *      -   EVENT_AFTERTOUCH
 *      -   EVENT_CONTROL_CHANGE
 *      -   EVENT_PITCH_WHEEL
 *      -   EVENT_PROGRAM_CHANGE
 *      -   EVENT_CHANNEL_PRESSURE
 *
 * \threadsafe
 */

void
sequence::decrement_selected (unsigned char a_stat, unsigned char /*a_control*/)
{
    lock();
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_selected() && i->get_status() == a_stat)
        {
            if
            (
                a_stat == EVENT_NOTE_ON || a_stat == EVENT_NOTE_OFF ||
                a_stat == EVENT_AFTERTOUCH || a_stat == EVENT_CONTROL_CHANGE ||
                a_stat == EVENT_PITCH_WHEEL
            )
            {
                i->decrement_data2();
            }
            else if
            (
                a_stat == EVENT_PROGRAM_CHANGE ||
                a_stat == EVENT_CHANNEL_PRESSURE
            )
            {
                i->decrement_data1();
            }
        }
    }
    unlock();
}

/**
 *  Copies the selected events.
 *
 * \threadsafe
 */

void
sequence::copy_selected ()
{
    lock();
    m_list_clipboard.clear();

    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_selected())
            m_list_clipboard.push_back((*i));
    }

    long first_tick = (*m_list_clipboard.begin()).get_timestamp();
    for (i = m_list_clipboard.begin(); i != m_list_clipboard.end(); i++)
    {
        i->set_timestamp(i->get_timestamp() - first_tick);
    }
    unlock();
}

/**
 *  Pastes the selected notes (and only note events) at the given tick and
 *  the given note value.
 *
 *  I wonder if we can get away with just getting a reference to
 *  m_list_clipboard, rather than copying the whole thing, for speed.
 *
 * \threadsafe
 */

void
sequence::paste_selected (long a_tick, int a_note)
{
    lock();
    EventList clipboard = m_list_clipboard;      /* copy clipboard */
    EventList::iterator i;
    for (i = clipboard.begin(); i != clipboard.end(); i++)
        i->set_timestamp(i->get_timestamp() + a_tick);

    if (clipboard.begin()->is_note_on() || clipboard.begin()->is_note_off())
    {
        int highest_note = 0;
        for (i = clipboard.begin(); i != clipboard.end(); i++)
            if (i->get_note() > highest_note)
                highest_note = i->get_note();

        for (i = clipboard.begin(); i != clipboard.end(); i++)
            i->set_note(i->get_note() - (highest_note - a_note));
    }
    m_list_event.merge(clipboard);
    m_list_event.sort();
    verify_and_link();
    reset_draw_marker();
    unlock();
}

/**
 *  Changes the event data range.  Changes only selected events, if any.
 *
 * \threadsafe
 *
 *  Let t == the current tick value; ts == tick start value; tf == tick
 *  finish value; ds = data start value; df == data finish value; d = the
 *  new data value.
 *
 *  Then
 *
\verbatim
             df (t - ts) + ds (tf - t)
        d = --------------------------
                    tf  -   ts
\endverbatim
 *
 *  If this were an interpolation formula it would be:
 *
\verbatim
                            t -  ts
        d = ds + (df - ds) ---------
                            tf - ts
\endverbatim
 *
 *  Something is not quite right; to be investigated.
 *
 *  \param a_tick_s
 *      Provides the starting tick value.
 *
 *  \param a_tick_f
 *      Provides the ending tick value.
 *
 *  \param a_status
 *      Provides the event status that is to be changed.
 *
 *  \param a_cc
 *      Provides the event control value.
 *
 *  \param a_data_s
 *      Provides the starting data value.
 *
 *  \param a_data_f
 *      Provides the finishing data value.
 */

void
sequence::change_event_data_range
(
    long a_tick_s, long a_tick_f,
    unsigned char a_status, unsigned char a_cc,
    int a_data_s, int a_data_f
)
{
    lock();
    bool have_selection = false;
    if (get_num_selected_events(a_status, a_cc))
        have_selection = true;

    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        unsigned char d0, d1;
        bool set = false;
        i->get_data(&d0, &d1);

        /* correct status and not CC */

        if (a_status != EVENT_CONTROL_CHANGE && i->get_status() == a_status)
            set = true;

        /* correct status and correct cc */

        if
        (
            a_status == EVENT_CONTROL_CHANGE &&
            i->get_status() == a_status && d0 == a_cc
        )
        {
            set = true;
        }

        long tick = i->get_timestamp();
        if (! (tick >= a_tick_s && tick <= a_tick_f))       /* in range? */
            set = false;

        if (have_selection && (!i->is_selected()))    /* in selection? */
            set = false;

        if (set)
        {
            if (a_tick_f == a_tick_s)
                a_tick_f = a_tick_s + 1;                /* avoid divide by 0 */

            int newdata =
            (
                (tick - a_tick_s) * a_data_f + (a_tick_f - tick) * a_data_s
            ) / (a_tick_f - a_tick_s);

            if (newdata < 0)
                newdata = 0;

            if (newdata > 127)
                newdata = 127;

            if (a_status == EVENT_NOTE_ON)
                d1 = newdata;
            else if (a_status == EVENT_NOTE_OFF)
                d1 = newdata;
            else if (a_status == EVENT_AFTERTOUCH)
                d1 = newdata;
            else if (a_status == EVENT_CONTROL_CHANGE)
                d1 = newdata;
            else if (a_status == EVENT_PROGRAM_CHANGE)
                d0 = newdata;                           /* d0 == new patch */
            else if (a_status == EVENT_CHANNEL_PRESSURE)
                d0 = newdata;                           /* d0 == pressure  */
            else if (a_status == EVENT_PITCH_WHEEL)
                d1 = newdata;

            i->set_data(d0, d1);
        }
    }
    unlock();
}

/**
 *  Adds a note of a given length and  note value, at a given tick
 *  location.  It adds a single note-on / note-off pair.
 *
 *  The a_paint parameter indicates if we care about the painted event,
 *  so then the function runs though the events and deletes the painted
 *  ones that overlap the ones we want to add.
 *
 * \threadsafe
 */

void
sequence::add_note (long a_tick, long a_length, int a_note, bool a_paint)
{
    lock();
    if (a_tick >= 0 && a_note >= 0 && a_note < c_num_keys)
    {
        bool ignore = false;
        if (a_paint)                        /* see the banner above */
        {
            EventList::iterator i;
            for (i = m_list_event.begin(); i != m_list_event.end(); i++)
            {
                if
                (
                    i->is_painted() &&
                    i->is_note_on() &&
                    i->get_timestamp() == a_tick
                )
                {
                    if (i->get_note() == a_note)
                    {
                        ignore = true;
                        break;
                    }
                    i->mark();
                    if (i->is_linked())
                        i->get_linked()->mark();

                    set_dirty();
                }
            }
            remove_marked();
        }
        if (! ignore)
        {
            event e;
            if (a_paint)
                e.paint();                      // ????

            e.set_status(EVENT_NOTE_ON);
            e.set_data(a_note, 100);
            e.set_timestamp(a_tick);
            add_event(&e);

            e.set_status(EVENT_NOTE_OFF);
            e.set_data(a_note, 100);
            e.set_timestamp(a_tick + a_length);
            add_event(&e);
        }
    }
    verify_and_link();
    unlock();
}

/**
 *  Adds a event of a given status value and data values, at a given tick
 *  location.
 *
 *  The a_paint parameter indicates if we care about the painted event,
 *  so then the function runs though the events and deletes the painted
 *  ones that overlap the ones we want to add.
 *
 * \threadsafe
 */

void
sequence::add_event
(
    long a_tick, unsigned char a_status,
    unsigned char a_d0, unsigned char a_d1, bool a_paint
)
{
    lock();
    if (a_tick >= 0)
    {
        if (a_paint)
        {
            EventList::iterator i;
            for (i = m_list_event.begin(); i != m_list_event.end(); i++)
            {
                if (i->is_painted() && i->get_timestamp() == a_tick)
                {
                    i->mark();
                    if (i->is_linked())
                        i->get_linked()->mark();

                    set_dirty();
                }
            }
            remove_marked();
        }
        event e;
        if (a_paint)
            e.paint();                              // ???

        e.set_status(a_status);
        e.set_data(a_d0, a_d1);
        e.set_timestamp(a_tick);
        add_event(&e);
    }
    verify_and_link();
    unlock();
}

/**
 *  Streams the given event.
 *
 * \threadsafe
 */

void
sequence::stream_event (event *a_ev)
{
    lock();
    a_ev->mod_timestamp(m_length);              /* adjust the tick */
    if (m_recording)
    {
        if (global_is_pattern_playing)
        {
            add_event(a_ev);
            set_dirty();
        }
        else
        {
            if (a_ev->is_note_on())
            {
                push_undo();
                add_note
                (
                    m_last_tick % m_length, m_snap_tick - 2,
                    a_ev->get_note(), false
                );
                set_dirty();
                m_notes_on++;
            }
            if (a_ev->is_note_off()) m_notes_on--;
            if (m_notes_on <= 0) m_last_tick += m_snap_tick;
        }
    }
    if (m_thru)
    {
        put_event_on_bus(a_ev);
    }
    link_new();
    if (m_quantized_rec && global_is_pattern_playing)
    {
        if (a_ev->is_note_off())
        {
            select_note_events
            (
                a_ev->get_timestamp(), a_ev->get_note(),
                    a_ev->get_timestamp(), a_ev->get_note(), e_select
            );
            quantize_events(EVENT_NOTE_ON, 0, m_snap_tick, 1 , true);
        }
    }
    unlock();
}

/**
 *  Sets the dirty flags for names, main, and performance.
 */

void
sequence::set_dirty_mp ()
{
    m_dirty_names =  m_dirty_main =  m_dirty_perf = true;
}

/**
 *  Call set_dirty_mp() and then sets the dirty flag for editing.
 *
 * \threadsafe
 */

void
sequence::set_dirty ()
{
    set_dirty_mp();
    m_dirty_edit = true;
}

/**
 *  Returns the value of the dirty names (heh heh) flag, and sets that
 *  flag to false.
 *
 * \threadsafe
 */

bool
sequence::is_dirty_names ()
{
    lock();
    bool result = m_dirty_names;
    m_dirty_names = false;
    unlock();
    return result;
}

/**
 *  Returns the value of the dirty main flag, and sets that flag to false
 *  (i.e. resets it).  This flag signals that a redraw is needed from
 *  recording.
 *
 * \threadsafe
 */

bool
sequence::is_dirty_main ()
{
    lock();
    bool result = m_dirty_main;
    m_dirty_main = false;
    unlock();
    return result;
}

/**
 *  Returns the value of the dirty performance flag, and sets that
 *  flag to false.
 *
 * \threadsafe
 */

bool
sequence::is_dirty_perf()
{
    lock();
    bool result = m_dirty_perf;
    m_dirty_perf = false;
    unlock();
    return result;
}

/**
 *  Returns the value of the dirty edit flag, and sets that
 *  flag to false.
 *
 * \threadsafe
 */

bool
sequence::is_dirty_edit ()
{
    lock();
    bool result = m_dirty_edit;
    m_dirty_edit = false;
    unlock();
    return result;
}

/**
 *  Plays a note from the piano roll on the main bus on the master MIDI
 *  buss.  It flushes a note to the midibus to preview its sound, used by
 *  the virtual piano.
 *
 * \threadsafe
 */

void
sequence::play_note_on (int a_note)
{
    lock();
    event e;
    e.set_status(EVENT_NOTE_ON);
    e.set_data(a_note, 127);
    m_masterbus->play(m_bus, &e, m_midi_channel);
    m_masterbus->flush();
    unlock();
}

/**
 *  Turns off a note from the piano roll on the main bus on the master MIDI
 *  buss.
 *
 * \threadsafe
 */

void
sequence::play_note_off (int a_note)
{
    lock();
    event e;
    e.set_status(EVENT_NOTE_OFF);
    e.set_data(a_note, 127);
    m_masterbus->play(m_bus, &e, m_midi_channel);
    m_masterbus->flush();
    unlock();
}

/**
 *  Clears the whole list of triggers.
 *
 * \threadsafe
 */

void
sequence::clear_triggers ()
{
    lock();
    m_list_trigger.clear();
    unlock();
}



/**
 *  Adds a trigger.  If a_state = true, the range is on.
 *  If a_state = false, the range is off.
 *
 *  What is this?
 *
\verbatim
   is      ie
   <      ><        ><        >
   es             ee
   <               >
   XX

   es ee
   <   >
   <>

   es    ee
   <      >
   <    >

   es     ee
   <       >
   <    >
\endverbatim
*/

void
sequence::add_trigger
(
    long a_tick, long a_length, long a_offset, bool a_adjust_offset
)
{
    lock();
    trigger e;
    if (a_adjust_offset)
        e.m_offset = adjust_offset(a_offset);
    else
        e.m_offset = a_offset;

    e.m_selected = false;
    e.m_tick_start = a_tick;
    e.m_tick_end = a_tick + a_length - 1;

    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if                              /* Is it inside the new one ? erase */
        (
            i->m_tick_start >= e.m_tick_start &&
            i->m_tick_end <= e.m_tick_end
        )
        {
            m_list_trigger.erase(i);
            i = m_list_trigger.begin();
            continue;
        }
        else if                         /* Is the event's end inside? */
        (
            i->m_tick_end >= e.m_tick_end &&
            i->m_tick_start <= e.m_tick_end
        )
        {
            i->m_tick_start = e.m_tick_end + 1;
        }
        else if                     /* is the last start inside the new end? */
        (
            i->m_tick_end   >= e.m_tick_start &&
            i->m_tick_start <= e.m_tick_start
        )
        {
            i->m_tick_end = e.m_tick_start - 1;
        }
        ++i;
    }
    m_list_trigger.push_front(e);
    m_list_trigger.sort();
    unlock();
}

/**
 *  This function examines each trigger in the trigger list.  If the given
 *  position is between the current trigger's tick-start and tick-end
 *  values, the these values are copied to the start and end parameters,
 *  respectively, and then we exit.
 *
 * \threadsafe
 *
 * \param position
 *      The position to examine.
 *
 * \param start
 *      The destination for the starting tick (m_tick_start) of the
 *      matching trigger.
 *
 * \param end
 *      The destination for the ending tick (m_tick_end) of the
 *      matching trigger.
 *
 * \return
 *      Returns true if a trigger was found whose start/end ticks
 *      contained the position.  Otherwise, false is returned, and the
 *      start and end return parameters should not be used.
 */

bool
sequence::intersectTriggers (long position, long & start, long & end)
{
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if (i->m_tick_start <= position && position <= i->m_tick_end)
        {
            start = i->m_tick_start;
            end = i->m_tick_end;
            unlock();
            return true;
        }
        ++i;
    }
    unlock();
    return false;
}

/**
 *  This function examines each note in the event list.
 *
 *  If the given
 *  position is between the current notes on and off time values,
 *  values, the these values are copied to the start and end parameters,
 *  respectively, the note value is copied to the note parameter, and then
 *  we exit.
 *
 * \threadsafe
 *
 * \param position
 *      The position to examine.
 *
 * \param position_note
 *      I think this is the note value we might be looking for ???
 *
 * \param start
 *      The destination for the starting tick (m_tick_start) of the
 *      matching trigger.
 *
 * \param end
 *      The destination for the ending tick (m_tick_end) of the
 *      matching trigger.
 *
 * \param note
 *      The destination for the note of the matching event.
 *
 * \return
 *      Returns true if a event was found whose start/end ticks
 *      contained the position.  Otherwise, false is returned, and the
 *      start and end return parameters should not be used.
 */

bool
sequence::intersectNotes
(
    long position, long position_note, long & start, long & end, long & note
)
{
    lock();
    EventList::iterator on = m_list_event.begin();
    EventList::iterator off = m_list_event.begin();
    while (on != m_list_event.end())
    {
        if (position_note == on->get_note() && on->is_note_on())
        {
            off = on;               /* find next "off" event for the note   */
            ++off;
            while
            (
                off != m_list_event.end() &&
                (on->get_note() != off->get_note() || off->is_note_on())
            )
            {
                ++off;
            }
            if
            (
                on->get_note() == off->get_note() &&
                off->is_note_off() &&
                on->get_timestamp() <= position &&
                position <= off->get_timestamp()
            )
            {
                start = on->get_timestamp();
                end = off->get_timestamp();
                note = on->get_note();
                unlock();
                return true;
            }
        }
        ++on;
    }
    unlock();
    return false;
}

/**
 *  This function examines each non-note event in the event list.
 *
 *  If the given
 *  position is between the current trigger's tick-start and tick-end
 *  values, the these values are copied to the start and end parameters,
 *  respectively, and then we exit.
 *
 * \threadsafe
 *
 * \param posstart
 *      The starting position to examine.
 *
 * \param posend
 *      The ending position to examine.
 *
 * \param status
 *      The desired status value.
 *
 * \param start
 *      The destination for the starting tick (m_tick_start) of the
 *      matching trigger.
 *
 * \return
 *      Returns true if a event was found whose start/end ticks
 *      contained the position.  Otherwise, false is returned, and the
 *      start and end return parameters should not be used.
 */

bool
sequence::intersectEvents
(
    long posstart, long posend, long status, long & start
)
{
    lock();
    EventList::iterator on = m_list_event.begin();
    while (on != m_list_event.end())
    {
        if (status == on->get_status())
        {
            if
            (
                on->get_timestamp() <= posstart &&
                posstart <= (on->get_timestamp() + (posend - posstart))
            )
            {
                start = on->get_timestamp();
                unlock();
                return true;
            }
        }
        ++on;
    }
    unlock();
    return false;
}

/**
 *  Grows a trigger.
 *
 * \threadsafe
 */

void
sequence::grow_trigger (long a_tick_from, long a_tick_to, long a_length)
{
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        /* Find our pair */

        if (i->m_tick_start <= a_tick_from && i->m_tick_end >= a_tick_from)
        {
            long start = i->m_tick_start;
            long end   = i->m_tick_end;
            if (a_tick_to < start)
                start = a_tick_to;

            if ((a_tick_to + a_length - 1) > end)
                end = (a_tick_to + a_length - 1);

            add_trigger(start, end - start + 1, i->m_offset);
            break;
        }
        ++i;
    }
    unlock();
}

/**
 *  Deletes a trigger, that brackets the given tick, from the trigger-list.
 *
 * \threadsafe
 */

void
sequence::del_trigger (long a_tick)
{
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if (i->m_tick_start <= a_tick && i->m_tick_end >= a_tick)
        {
            m_list_trigger.erase(i);
            break;
        }
        ++i;
    }
    unlock();
}

/**
 *  Sets m_trigger_offset and wraps it to m_length.
 *
 * \threadsafe
 */

void
sequence::set_trigger_offset (long a_trigger_offset)
{
    lock();
    m_trigger_offset = (a_trigger_offset % m_length);
    m_trigger_offset += m_length;
    m_trigger_offset %= m_length;
    unlock();
}

/**
 *  Splits the trigger given by the parameter into two triggers.  The
 *  original trigger ends 1 tick before the a_split_tick parameter,
 *  and the new trigger starts at a_split_tick and ends where the original
 *  trigger ended.
 *
 *  This is the private overload of split_trigger.
 *
 * \threadsafe
 *
 * \param trig
 *      Provides the original trigger, and also holds the changes made to
 *      that trigger as it is shortened.
 *
 * \param a_split_tick
 *      The position just after where the original trigger will be
 *      truncated, and the new trigger begins.
 */

void
sequence::split_trigger (trigger & trig, long a_split_tick)
{
    lock();
    long new_tick_end   = trig.m_tick_end;
    long new_tick_start = a_split_tick;
    trig.m_tick_end = a_split_tick - 1;

    long length = new_tick_end - new_tick_start;
    if (length > 1)
        add_trigger(new_tick_start, length + 1, trig.m_offset);

    unlock();
}

/**
 *  Splits a trigger.
 *
 *  This is the public overload of split_trigger.
 *
 * \threadsafe
 */

void
sequence::split_trigger (long a_tick)
{
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        /* trigger greater than L and R */

        if (i->m_tick_start <= a_tick && i->m_tick_end >= a_tick)
        {
            long tick = i->m_tick_end - i->m_tick_start;
            tick += 1;
            tick /= 2;

            split_trigger(*i, i->m_tick_start + tick);
            break;
        }
        ++i;
    }
    unlock();
}

/**
 *  Not sure what these diagrams are for yet.
 *
\verbatim
      |...|...|...|...|...|...|...

      0123456789abcdef0123456789abcdef
      [      ][      ][      ][      ][      ][

      [  ][      ][  ][][][][][      ]  [  ][  ]
      0   4       4   0 7 4 2 0         6   2
      0   4       4   0 1 4 6 0         2   6 inverse offset

      [              ][              ][              ]
      [  ][      ][  ][][][][][      ]  [  ][  ]
      0   c       4   0 f c a 8         e   a
      0   4       c   0 1 4 6 8         2   6  inverse offset

      [                              ][
      [  ][      ][  ][][][][][      ]  [  ][  ]
      k   g f c a 8
      0   4       c   g h k m n       inverse offset

      0123456789abcdefghijklmonpq
      ponmlkjihgfedcba9876543210
      0fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210
\endverbatim
 *
 */

/**
 *  Adjusts trigger offsets to the length of ???,
 *  for all triggers, and undo triggers.
 *
 * \threadsafe
 */

void
sequence::adjust_trigger_offsets_to_length(long a_new_len)
{
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        i->m_offset = adjust_offset(i->m_offset);
        i->m_offset = m_length - i->m_offset; // flip

        long inverse_offset = m_length - (i->m_tick_start % m_length);
        long local_offset = (inverse_offset - i->m_offset);
        local_offset %= m_length;

        long inverse_offset_new = a_new_len - (i->m_tick_start % a_new_len);
        long new_offset = inverse_offset_new - local_offset;
        i->m_offset = (new_offset % a_new_len);
        i->m_offset = a_new_len - i->m_offset;
        ++i;
    }
    unlock();
}

/**
 *  Not sure what these diagrams are for yet.
 *
\verbatim

... a
[      ][      ]
...
... a
...

5   7    play
3        offset
8   10   play

X...X...X...X...X...X...X...X...X...X...
L       R
[        ] [     ]  []  orig
[                    ]

        <<
        [     ]    [  ][ ]  [] split on the R marker, shift first
        [     ]        [     ]
        delete middle
        [     ][ ]  []         move ticks
        [     ][     ]

        L       R
        [     ][ ] [     ]  [] split on L
        [     ][             ]

        [     ]        [ ] [     ]  [] increase all after L
        [     ]        [             ]
\endverbatim
 *
 */

/**
 *  Copies triggers to...
 *
 * \threadsafe
 */

void
sequence::copy_triggers (long a_start_tick, long a_distance)
{
    long from_start_tick = a_start_tick + a_distance;
    long from_end_tick = from_start_tick + a_distance - 1;
    lock();
    move_triggers(a_start_tick, a_distance, true);

    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if
        (
            i->m_tick_start >= from_start_tick &&
            i->m_tick_start <= from_end_tick
        )
        {
            trigger e;
            e.m_offset = i->m_offset;
            e.m_selected = false;
            e.m_tick_start  = i->m_tick_start - a_distance;
            if (i->m_tick_end <= from_end_tick)
                e.m_tick_end = i->m_tick_end - a_distance;

            if (i->m_tick_end > from_end_tick)
                e.m_tick_end = from_start_tick - 1;

            e.m_offset += (m_length - (a_distance % m_length));
            e.m_offset %= m_length;
            if (e.m_offset < 0)
                e.m_offset += m_length;

            m_list_trigger.push_front(e);
        }
        ++i;
    }
    m_list_trigger.sort();
    unlock();
}

/**
 *  Moves triggers in the trigger-list.
 *
 * \threadsafe
 */

void
sequence::move_triggers (long a_start_tick, long a_distance, bool a_direction)
{
    long a_end_tick = a_start_tick + a_distance;
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        /* trigger greater than L and R */

        if (i->m_tick_start < a_start_tick && i->m_tick_end > a_start_tick)
        {
            if (a_direction)                            /* forward */
                split_trigger(*i, a_start_tick);
            else                                        /* back    */
                split_trigger(*i, a_end_tick);
        }

        /* triggers on L */

        if (i->m_tick_start < a_start_tick && i->m_tick_end > a_start_tick)
        {
            if (a_direction)                            /* forward */
                split_trigger(*i, a_start_tick);
            else                                        /* back    */
                i->m_tick_end = a_start_tick - 1;
        }

        /* In betweens */

        if
        (
            i->m_tick_start >= a_start_tick &&
            i->m_tick_end <= a_end_tick && ! a_direction
        )
        {
            m_list_trigger.erase(i);
            i = m_list_trigger.begin();
        }

        /* triggers on R */

        if (i->m_tick_start < a_end_tick && i->m_tick_end > a_end_tick)
        {
            if (!a_direction)                           /* forward */
                i->m_tick_start = a_end_tick;
        }
        ++i;
    }
    i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if (a_direction)                                /* forward */
        {
            if (i->m_tick_start >= a_start_tick)
            {
                i->m_tick_start += a_distance;
                i->m_tick_end   += a_distance;
                i->m_offset += a_distance;
                i->m_offset %= m_length;
            }
        }
        else                                            /* back    */
        {
            if (i->m_tick_start >= a_end_tick)
            {
                i->m_tick_start -= a_distance;
                i->m_tick_end   -= a_distance;
                i->m_offset += (m_length - (a_distance % m_length));
                i->m_offset %= m_length;
            }
        }
        i->m_offset = adjust_offset(i->m_offset);
        ++i;
    }
    unlock();
}

/**
 *  Gets the selected trigger's start tick.
 *
 * \threadsafe
 */

long
sequence::get_selected_trigger_start_tick ()
{
    long result = -1;
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if (i->m_selected)
            result = i->m_tick_start;

        ++i;
    }
    unlock();
    return result;
}

/**
 *  Gets the selected trigger's end tick.
 *
 * \threadsafe
 */

long
sequence::get_selected_trigger_end_tick ()
{
    long result = -1;
    lock();
    TriggerList::iterator i = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if (i->m_selected)
            result = i->m_tick_end;

        ++i;
    }
    unlock();
    return result;
}

/**
 *  Moves selected triggers as per the given parameters.
 *
\verbatim
          min_tick][0                1][max_tick
                            2
\endverbatim
 *
 *  -   If we are moving the 0, use first as offset.
 *  -   If we are moving the 1, use the last as the offset.
 *  -   If we are moving both (2), use first as offset.
 *
 * \threadsafe
 */

void
sequence::move_selected_triggers_to
(
    long a_tick, bool a_adjust_offset, int a_which
)
{
    lock();
    long min_tick = 0;
    long max_tick = 0x7ffffff;
    TriggerList::iterator i = m_list_trigger.begin();
    TriggerList::iterator s = m_list_trigger.begin();
    while (i != m_list_trigger.end())
    {
        if (i->m_selected)
        {
            s = i;
            if (i != m_list_trigger.end() && ++i != m_list_trigger.end())
                max_tick = i->m_tick_start - 1;


            /* See the list of options in the function banner. */

            long a_delta_tick = 0;
            if (a_which == 1)
            {
                a_delta_tick = a_tick - s->m_tick_end;
                if (a_delta_tick > 0 && (a_delta_tick+s->m_tick_end) > max_tick)
                    a_delta_tick = ((max_tick) - s->m_tick_end);

                /* not past the first */

                if
                (
                    a_delta_tick < 0 &&
                    (
                        a_delta_tick+s->m_tick_end <= (s->m_tick_start+c_ppqn/8)
                    )
                )
                {
                    a_delta_tick = (s->m_tick_start+c_ppqn/8) - s->m_tick_end;
                }
            }
            if (a_which == 0)
            {
                a_delta_tick = a_tick - s->m_tick_start;
                if
                (
                    a_delta_tick < 0 &&
                    (a_delta_tick + s->m_tick_start) < min_tick)
                {
                    a_delta_tick = ((min_tick) - s->m_tick_start);
                }

                /* not past last */

                if
                (
                    a_delta_tick > 0 &&
                    (a_delta_tick + s->m_tick_start >= (s->m_tick_end-c_ppqn/8))
                )
                {
                    a_delta_tick = (s->m_tick_end-c_ppqn/8) - s->m_tick_start;
                }
            }
            if (a_which == 2)
            {
                a_delta_tick = a_tick - s->m_tick_start;
                if
                (
                    a_delta_tick < 0 &&
                    (a_delta_tick + s->m_tick_start) < min_tick
                )
                {
                    a_delta_tick = ((min_tick) - s->m_tick_start);
                }
                if
                (
                    a_delta_tick > 0 &&
                    (a_delta_tick + s->m_tick_end) > max_tick
                )
                {
                    a_delta_tick = ((max_tick) - s->m_tick_end);
                }
            }

            if (a_which == 0 || a_which == 2)
                s->m_tick_start += a_delta_tick;

            if (a_which == 1 || a_which == 2)
                s->m_tick_end   += a_delta_tick;

            if (a_adjust_offset)
            {
                s->m_offset += a_delta_tick;
                s->m_offset = adjust_offset(s->m_offset);
            }
            break;
        }
        else
        {
            min_tick = i->m_tick_end + 1;
        }
        ++i;
    }
    unlock();
}

/**
 *  Get the ending value of the last trigger in the trigger-list.
 *
 * \threadsafe
 */

long
sequence::get_max_trigger ()
{
    long result = 0;
    lock();
    if (m_list_trigger.size() > 0)
        result = m_list_trigger.back().m_tick_end;

    unlock();
    return result;
}

/**
 *  Adjusts the given offset by mod'ing it with m_length and adding
 *  m_length if needed, and returning the result.
 */

long
sequence::adjust_offset (long a_offset)
{
    a_offset %= m_length;
    if (a_offset < 0)
        a_offset += m_length;

    return a_offset;
}

bool
sequence::get_trigger_state (long a_tick)
{
    lock();
    bool result = false;
    TriggerList::iterator i;
    for (i = m_list_trigger.begin(); i != m_list_trigger.end(); i++)
    {
        if (i->m_tick_start <= a_tick && i->m_tick_end >= a_tick)
        {
            result = true;
            break;
        }
    }
    unlock();
    return result;
}

bool
sequence::select_trigger (long a_tick)
{
    lock();
    bool result = false;
    TriggerList::iterator i;
    for (i = m_list_trigger.begin(); i != m_list_trigger.end(); i++)
    {
        if (i->m_tick_start <= a_tick && i->m_tick_end >= a_tick)
        {
            i->m_selected = true;
            result = true;
        }
    }
    unlock();
    return result;
}

/**
 *  Always returns false!
 */

bool
sequence::unselect_triggers ()
{
    lock();
    bool result = false;
    TriggerList::iterator i;
    for (i = m_list_trigger.begin(); i != m_list_trigger.end(); i++)
        i->m_selected = false;

    unlock();
    return result;
}

void
sequence::del_selected_trigger ()
{
    lock();
    TriggerList::iterator i;
    for (i = m_list_trigger.begin(); i != m_list_trigger.end(); i++)
    {
        if (i->m_selected)
        {
            m_list_trigger.erase(i);
            break;
        }
    }
    unlock();
}

void
sequence::cut_selected_trigger ()
{
    copy_selected_trigger();
    del_selected_trigger();
}

void
sequence::copy_selected_trigger ()
{
    lock();
    TriggerList::iterator i;
    for (i = m_list_trigger.begin(); i != m_list_trigger.end(); i++)
    {
        if (i->m_selected)
        {
            m_trigger_clipboard = *i;
            m_trigger_copied = true;
            break;
        }
    }
    unlock();
}

void
sequence::paste_trigger ()
{
    if (m_trigger_copied)
    {
        long length =  m_trigger_clipboard.m_tick_end -
                       m_trigger_clipboard.m_tick_start + 1;
        // paste at copy end
        add_trigger(m_trigger_clipboard.m_tick_end + 1,
                    length,
                    m_trigger_clipboard.m_offset + length);

        m_trigger_clipboard.m_tick_start = m_trigger_clipboard.m_tick_end + 1;
        m_trigger_clipboard.m_tick_end = m_trigger_clipboard.m_tick_start + length - 1;

        m_trigger_clipboard.m_offset += length;
        m_trigger_clipboard.m_offset = adjust_offset(m_trigger_clipboard.m_offset);
    }
}


/* this refreshes the play marker to the LastTick */
/* resets draw marker so calls to getNextnoteEvent
       will start from the first */

void
sequence::reset_draw_marker ()
{
    lock();
    m_iterator_draw = m_list_event.begin();
    unlock();
}

void
sequence::reset_draw_trigger_marker ()
{
    lock();
    m_iterator_draw_trigger = m_list_trigger.begin();
    unlock();
}

int
sequence::get_lowest_note_event ()
{
    lock();
    int result = 127;
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_note_on() || i->is_note_off())
            if (i->get_note() < result)
                result = i->get_note();
    }
    unlock();
    return result;
}

int
sequence::get_highest_note_event ()
{
    lock();
    int ret = 0;
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if (i->is_note_on() || i->is_note_off())
            if (i->get_note() > ret)
                ret = i->get_note();
    }
    unlock();
    return ret;
}

/* each call seqdata( sequence *a_seq, int a_scale );fills the passed
 * references with a
       events elements, and returns true.  When it has no more events,
       returns a false */

draw_type
sequence::get_next_note_event
(
    long * a_tick_s, long * a_tick_f,
    int * a_note, bool * a_selected, int * a_velocity
)
{
    draw_type result = DRAW_FIN;
    *a_tick_f = 0;
    while (m_iterator_draw  != m_list_event.end())
    {
        *a_tick_s   = (*m_iterator_draw).get_timestamp();
        *a_note     = (*m_iterator_draw).get_note();
        *a_selected = (*m_iterator_draw).is_selected();
        *a_velocity = (*m_iterator_draw).get_note_velocity();

        /* note on, so its linked */

        if ((*m_iterator_draw).is_note_on() && (*m_iterator_draw).is_linked())
        {
            *a_tick_f   = (*m_iterator_draw).get_linked()->get_timestamp();
            result = DRAW_NORMAL_LINKED;
            m_iterator_draw++;
            return result;
        }
        else if ((*m_iterator_draw).is_note_on() && (!(*m_iterator_draw).is_linked()))
        {
            result = DRAW_NOTE_ON;
            m_iterator_draw++;
            return result;
        }
        else if ((*m_iterator_draw).is_note_off() && (!(*m_iterator_draw).is_linked()))
        {
            result = DRAW_NOTE_OFF;
            m_iterator_draw++;
            return result;
        }

        /* keep going until we hit null or find a NoteOn */

        m_iterator_draw++;
    }
    return DRAW_FIN;
}

bool
sequence::get_next_event (unsigned char * a_status, unsigned char * a_cc)
{
    unsigned char j;
    while (m_iterator_draw  != m_list_event.end())
    {
        *a_status = (*m_iterator_draw).get_status();
        (*m_iterator_draw).get_data(a_cc, &j);

        m_iterator_draw++; /* we have a good one  update and return */
        return true;
    }
    return false;
}

bool
sequence::get_next_event
(
    unsigned char a_status, unsigned char a_cc,
    long * a_tick, unsigned char * a_D0, unsigned char * a_D1,
    bool * a_selected
)
{
    while (m_iterator_draw  != m_list_event.end())
    {
        /* note on, so its linked */

        if ((*m_iterator_draw).get_status() == a_status)
        {
            (*m_iterator_draw).get_data(a_D0, a_D1);
            *a_tick = (*m_iterator_draw).get_timestamp();
            *a_selected = (*m_iterator_draw).is_selected();

            /* either we have a control chage with the right CC
               or its a different type of event */

            if
            (
                (a_status == EVENT_CONTROL_CHANGE && *a_D0 == a_cc) ||
                (a_status != EVENT_CONTROL_CHANGE))
            {
                m_iterator_draw++; /* we have a good one update and return */
                return true;
            }
        }

        /* keep going until we hit null or find a NoteOn */

        m_iterator_draw++;
    }
    return false;
}

bool
sequence::get_next_trigger
(
    long * a_tick_on, long * a_tick_off, bool * a_selected, long * a_offset)
{
    while (m_iterator_draw_trigger  != m_list_trigger.end())
    {
        *a_tick_on  = (*m_iterator_draw_trigger).m_tick_start;
        *a_selected = (*m_iterator_draw_trigger).m_selected;
        *a_offset = (*m_iterator_draw_trigger).m_offset;
        *a_tick_off = (*m_iterator_draw_trigger).m_tick_end;
        m_iterator_draw_trigger++;
        return true;
    }
    return false;
}

void
sequence::remove_all ()
{
    lock();
    m_list_event.clear();
    unlock();
}

void
sequence::lock ()
{
    m_mutex.lock();
}

void
sequence::unlock ()
{
    m_mutex.unlock();
}

const char *
sequence::get_name ()
{
    return m_name.c_str();
}

/* returns last tick played..  used by editors idle function */

long
sequence::get_last_tick ()
{
    return (m_last_tick + (m_length - m_trigger_offset)) % m_length;
}

/* sets the midibus to dump to */

void
sequence::set_midi_bus (char a_mb)
{
    lock();
    off_playing_notes(); /* off notes except initial */
    this->m_bus = a_mb;
    set_dirty();
    unlock();
}

char
sequence::get_midi_bus ()
{
    return this->m_bus;
}

void
sequence::set_length (long a_len, bool a_adjust_triggers)
{
    lock();
    bool was_playing = get_playing();
    set_playing(false); /* turn everything off */
    if (a_len < (c_ppqn / 4))
        a_len = (c_ppqn / 4);

    if (a_adjust_triggers)
        adjust_trigger_offsets_to_length(a_len);

    m_length = a_len;
    verify_and_link();
    reset_draw_marker();
    if (was_playing)            /* start up and refresh */
        set_playing(true);

    unlock();
}

long
sequence::get_length ()
{
    return m_length;
}


/* sets state.  when playing, and sequencer is running, notes get dumped
 * to the alsa buffers */

void
sequence::set_playing (bool a_p)
{
    lock();
    if (a_p != get_playing())
    {
        if (a_p)
        {
            m_playing = true; /* turn on */
        }
        else
        {
            m_playing = false; /* turn off */
            off_playing_notes();
        }
        set_dirty();
    }
    m_queued = false;
    unlock();
}

void
sequence::toggle_playing ()
{
    set_playing(! get_playing());
}

bool
sequence::get_playing ()
{
    return m_playing;
}

void
sequence::set_recording (bool a_r)
{
    lock();
    m_recording = a_r;
    m_notes_on = 0;
    unlock();
}

bool
sequence::get_recording ()
{
    return m_recording;
}

void
sequence::set_snap_tick (int a_st)
{
    lock();
    m_snap_tick = a_st;
    unlock();
}

void
sequence::set_quantized_rec (bool a_qr)
{
    lock();
    m_quantized_rec = a_qr;
    unlock();
}

bool
sequence::get_quantized_rec ()
{
    return m_quantized_rec;
}

void
sequence::set_thru (bool a_r)
{
    lock();
    m_thru = a_r;
    unlock();
}

bool
sequence::get_thru ()
{
    return m_thru;
}

/* sets sequence name */

void
sequence::set_name (char * a_name)
{
    m_name = a_name;
    set_dirty_mp();
}

void
sequence::set_name (const std::string & a_name)
{
    m_name = a_name;
    set_dirty_mp();
}

void
sequence::set_midi_channel (unsigned char a_ch)
{
    lock();
    off_playing_notes();
    m_midi_channel = a_ch;
    set_dirty();
    unlock();
}

unsigned char
sequence::get_midi_channel ()
{
    return m_midi_channel;
}

void
sequence::print ()
{
    printf("[%s]\n", m_name.c_str());
    for
    (
        EventList::iterator i = m_list_event.begin();
        i != m_list_event.end(); i++
    )
    {
        i->print();
    }
    printf("events[%ld]\n\n", m_list_event.size());
}

void
sequence::print_triggers()
{
    printf("[%s]\n", m_name.c_str());
    for
    (
        TriggerList::iterator i = m_list_trigger.begin();
        i != m_list_trigger.end(); i++
    )
    {
        printf
        (
            "  tick_start[%ld] tick_end[%ld] off[%ld]\n",
            i->m_tick_start, i->m_tick_end, i->m_offset
        );
    }
}

/* takes an event this sequence is holding and places it on our midibus */

void
sequence::put_event_on_bus (event * a_e)
{
    lock();
    unsigned char note = a_e->get_note();
    bool skip = false;
    if (a_e->is_note_on())
        m_playing_notes[note]++;

    if (a_e->is_note_off())
    {
        if (m_playing_notes[note] <= 0)
            skip = true;
        else
            m_playing_notes[note]--;
    }
    if (! skip)
        m_masterbus->play(m_bus, a_e,  m_midi_channel);

    m_masterbus->flush();
    unlock();
}

/* send a note off for all active notes */

void
sequence::off_playing_notes ()
{
    lock();
    event e;
    for (int x = 0; x < c_midi_notes; x++)
    {
        while (m_playing_notes[x] > 0)
        {
            e.set_status(EVENT_NOTE_OFF);
            e.set_data(x, 0);
            m_masterbus->play(m_bus, &e, m_midi_channel);
            m_playing_notes[x]--;
        }
    }
    m_masterbus->flush();
    unlock();
}

/**
 *  Select all events with the given status, and returns the number
 *  selected.  Note that there is also an overloaded version of this
 *  function.
 *
 * \threadsafe
 *
 * \warning
 *      This used to be a void function, so it just returns 0 for now.
 */

int
sequence::select_events
(
    unsigned char a_status, unsigned char a_cc, bool a_inverse
)
{
    lock();
    unsigned char d0, d1;
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        bool set = false;
        i->get_data(&d0, &d1);

        /* correct status and not CC */

        if (a_status != EVENT_CONTROL_CHANGE && i->get_status() == a_status)
            set = true;

        /* correct status and correct cc */

        if
        (
            a_status == EVENT_CONTROL_CHANGE &&
            i->get_status() == a_status &&
            d0 == a_cc
        )
        {
            set = true;
        }
        if (set)
        {
            if (a_inverse)
            {
                if (!i->is_selected())
                    i->select();
                else
                    i->unselect();
            }
            else
                i->select();
        }
    }
    unlock();
    return 0;
}

void
sequence::transpose_notes (int a_steps, int a_scale)
{
    event e;
    EventList transposed_events;
    lock();
    mark_selected();
    EventList::iterator i;
    const int * transpose_table = nullptr;
    if (a_steps < 0)
    {
        transpose_table = &c_scales_transpose_dn[a_scale][0];
        a_steps *= -1;
    }
    else
        transpose_table = &c_scales_transpose_up[a_scale][0];

    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        if                                          /* is it being moved ? */
        (
            (i->get_status() ==  EVENT_NOTE_ON ||
                i->get_status() ==  EVENT_NOTE_OFF) &&
            i->is_marked()
        )
        {
            e = (*i);
            e.unmark();
            int  note = e.get_note();
            bool off_scale = false;
            if (transpose_table[note % 12] == 0)
            {
                off_scale = true;
                note -= 1;
            }
            for (int x = 0; x < a_steps; ++x)
                note += transpose_table[note % 12];

            if (off_scale)
                note += 1;

            e.set_note(note);
            transposed_events.push_front(e);
        }
    }
    remove_marked();
    transposed_events.sort();
    m_list_event.merge(transposed_events);
    verify_and_link();
    unlock();
}

/* Not deleting the ends, not selected.
 */

void
sequence::quantize_events
(
    unsigned char a_status, unsigned char a_cc,
    long a_snap_tick,  int a_divide, bool a_linked
)
{
    event e, f;
    lock();
    unsigned char d0, d1;
    EventList::iterator i;
    EventList quantized_events;
    mark_selected();
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        bool set = false;
        i->get_data(&d0, &d1);

        /* correct status and not CC */

        if (a_status != EVENT_CONTROL_CHANGE && i->get_status() == a_status)
            set = true;

        /* correct status and correct cc */

        if
        (   a_status == EVENT_CONTROL_CHANGE &&
            i->get_status() == a_status && d0 == a_cc
        )
        {
            set = true;
        }
        if (!i->is_marked())
            set = false;

        if (set)
        {
            e = (*i); /* copy event */
            i->select();
            e.unmark();

            long timestamp = e.get_timestamp();
            long timestamp_remander = (timestamp % a_snap_tick);
            long timestamp_delta = 0;
            if (timestamp_remander < a_snap_tick / 2)
                timestamp_delta = - (timestamp_remander / a_divide);
            else
                timestamp_delta = (a_snap_tick - timestamp_remander) / a_divide;
            if ((timestamp_delta + timestamp) >= m_length)
                timestamp_delta = - e.get_timestamp() ;

            e.set_timestamp(e.get_timestamp() + timestamp_delta);
            quantized_events.push_front(e);
            if (i->is_linked() && a_linked)
            {
                f = *i->get_linked();
                f.unmark();
                i->get_linked()->select();
                f.set_timestamp(f.get_timestamp() + timestamp_delta);
                quantized_events.push_front(f);
            }
        }
    }
    remove_marked();
    quantized_events.sort();
    m_list_event.merge(quantized_events);
    verify_and_link();
    unlock();
}

/**
 *  This was a <i> global </i> internal function called addListVar().
 *  Let's at least make it a private static member now, and hew to the
 *  naming conventions of this class.
 */

void
sequence::add_list_var (CharList * a_list, long a_var)
{
    long buffer;
    buffer = a_var & 0x7F;

    /*
     * We shift it right 7, and, if there are still set bits, we encode
     * into buffer in reverse order.
     */

    while ((a_var >>= 7))
    {
        buffer <<= 8;
        buffer |= ((a_var & 0x7F) | 0x80);
    }
    for (;;)
    {
        a_list->push_front((char) buffer & 0xFF);
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}

/**
 *  This was a <i> global </i> internal function called addLongList().
 *  Let's at least make it a private static member now, and hew to the
 *  naming conventions of this class.
 */

void
sequence::add_long_list (CharList * a_list, long a_x)
{
    a_list->push_front((a_x & 0xFF000000) >> 24);
    a_list->push_front((a_x & 0x00FF0000) >> 16);
    a_list->push_front((a_x & 0x0000FF00) >> 8);
    a_list->push_front((a_x & 0x000000FF));
}

void
sequence::fill_list (CharList * a_list, int a_pos)
{
    lock();
    *a_list = CharList();                               /* clear list */
    add_list_var(a_list, 0);                              /* sequence number */
    a_list->push_front(char(0xFF));
    a_list->push_front(0x00);
    a_list->push_front(0x02);
    a_list->push_front(char((a_pos & 0xFF00) >> 8));
    a_list->push_front(char(a_pos & 0x00FF));

    add_list_var(a_list, 0);                              /* name */
    a_list->push_front(char(0xFF));
    a_list->push_front(0x03);

    int length =  m_name.length();
    if (length > 0x7F) length = 0x7f;
    a_list->push_front(length);
    for (int i = 0; i < length; i++)
        a_list->push_front(m_name.c_str()[i]);

    long timestamp = 0, delta_time = 0, prev_timestamp = 0;
    EventList::iterator i;
    for (i = m_list_event.begin(); i != m_list_event.end(); i++)
    {
        event e = *i;
        timestamp = e.get_timestamp();
        delta_time = timestamp - prev_timestamp;
        prev_timestamp = timestamp;
        add_list_var(a_list, delta_time);             /* encode delta_time */

        /* timestamp is encoded, now do the status and data */

        a_list->push_front(e.m_status | m_midi_channel);
        switch (e.m_status & 0xF0)
        {
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xE0:

            a_list->push_front(e.m_data[0]);
            a_list->push_front(e.m_data[1]);
            break;

        case 0xC0:
        case 0xD0:

            a_list->push_front(e.m_data[0]);
            break;

        default:
            break;
        }
    }

    int num_triggers = m_list_trigger.size();
    TriggerList::iterator t = m_list_trigger.begin();
    TriggerList::iterator p;

    add_list_var(a_list, 0);
    a_list->push_front(char(0xFF));
    a_list->push_front(char(0x7F));
    add_list_var(a_list, (num_triggers * 3 * 4) + 4);
    add_long_list(a_list, c_triggers_new);
    for (int i = 0; i < num_triggers; i++)
    {
        p = t;
        add_long_list(a_list, t->m_tick_start);
        add_long_list(a_list, t->m_tick_end);
        add_long_list(a_list, t->m_offset);
        t++;
    }

    add_list_var(a_list, 0);                              /* bus */
    a_list->push_front(char(0xF));
    a_list->push_front(char(0x7F));
    a_list->push_front(char(0x05));
    add_long_list(a_list, c_midibus);
    a_list->push_front(m_bus);

    add_list_var(a_list, 0);                              /* timesig */
    a_list->push_front(char(0xFF));
    a_list->push_front(char(0x7F));
    a_list->push_front(char(0x06));
    add_long_list(a_list, c_timesig);
    a_list->push_front(m_time_beats_per_measure);
    a_list->push_front(m_time_beat_width);

    add_list_var(a_list, 0);                              /* channel */
    a_list->push_front(char(0xFF));
    a_list->push_front(char(0x7F));
    a_list->push_front(char(0x05));
    add_long_list(a_list, c_midich);
    a_list->push_front(m_midi_channel);

    delta_time = m_length - prev_timestamp;             /* meta track end */
    add_list_var(a_list, delta_time);
    a_list->push_front(char(0xFF));
    a_list->push_front(char(0x2F));
    a_list->push_front(char(0x00));
    unlock();
}

/*
 * sequence.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
