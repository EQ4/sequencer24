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

#ifndef SEQ24_MENU_H
#define SEQ24_MENU_H

/**
 * \file          seqmenu.h
 *
 *  This module declares/defines the class that handles the right-click
 *  menu of the sequence slots in the pattern window.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-02
 * \license       GNU GPLv2 or above
 *
 */

#include "sequence.h"

class perform;
class seqedit;

using namespace Gtk;

namespace Gtk
{
    class Menu;
}

/**
 *  This class handles the right-click menu of the sequence slots in the
 *  pattern window.
 *
 *  It is an abstract base class.
 */

class seqmenu : public virtual Glib::ObjectBase
{

private:

    Menu * m_menu;
    perform * m_mainperf;
    sequence m_clipboard;

    /**
     * \change
     *      Added by Chris on 2015-08-02 based on compiler warnings and a
     *      comment warning in the seq_edit() function.  We'll save the
     *      result of that function here, and will let valgrind tell us
     *      later if Gtkmm takes care of it.
     */

    seqedit * m_seqedit;

protected:

    int m_current_seq;

public:

    seqmenu (perform * a_p);

    /**
     *  Provides a rote base-class destructor.  This is necessary in an
     *  abstraction base class.
     */

    virtual ~seqmenu ();

protected:

    void popup_menu ();

private:

    void seq_edit ();
    void seq_new ();
    void seq_copy ();
    void seq_cut ();
    void seq_paste ();
    void seq_clear_perf ();
    void set_bus_and_midi_channel (int a_bus, int a_ch);
    void mute_all_tracks ();

    virtual void redraw (int a_sequence) = 0;   // pure virtual function

private:            // callback

    void on_realize();

};

#endif   // SEQ24_MENU_H

/*
 * seqmenu.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
