#ifndef SEQ24_KEYBINDENTRY_H
#define SEQ24_KEYBINDENTRY_H

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
 * \file          keybindentry.h
 *
 *  This module declares/defines the base class for keybinding entries.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-07-26
 * \license       GNU GPLv2 or above
 *
 *  This module define a GTK text-edit widget for getting keyboard button
 *  values (for binding keys).  Put the cursor in the text-box, hit a key,
 *  and something like  'a' (42)  appears...  each keypress replaces the
 *  previous text.  It also supports key-event and key-group maps in the
 *  perform class.
 */

#include <gtkmm/entry.h>
#include "easy_macros.h"

class perform;      /* forward declaration */

/**
 *  Class
 */

class KeyBindEntry : public Gtk::Entry
{

    friend class options;

private:

    enum type
    {
        location,
        events,
        groups
    };

private:

    unsigned int * m_key;
    type m_type;
    perform * m_perf;
    long m_slot;

public:

    KeyBindEntry
    (
        type t,
        unsigned int * location_to_write = nullptr,
        perform * p = nullptr,
        long s = 0
    );

    void set (unsigned int val);
    virtual bool on_key_press_event (GdkEventKey * event);
};

#endif   // SEQ24_KEYBINDENTRY_H

/*
 * keybindentry.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
