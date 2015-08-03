#ifndef SEQ24_PERFEDIT_H
#define SEQ24_PERFEDIT_H

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
 * \file          perfedit.h
 *
 *  This module declares/defines the base class for the Performance Editor,
 *  also known as the Song Editor.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-07-31
 * \license       GNU GPLv2 or above
 *
 */

#include "sequence.h"
#include "perform.h"

#include <list>
#include <string>
#include <gtkmm/widget.h>       // somehow, can't forward-declare GdkEventAny

#include "globals.h"
#include "mainwid.h"
#include "perfnames.h"
#include "perfroll.h"
#include "perftime.h"

using namespace Gtk;

/*
 *  Since these items are pointers, we were able to move (most) of the
 *  included header files to the cpp file.   Except for the items that
 *  come from widget.h, perhaps because GdkEventAny was a typedef.
 */

namespace Gtk
{
    class Adjustment;
    class Button;
    class Entry;
    class HBox;
    class HScrollbar;
    class Menu;
    class Table;
    class ToggleButton;
    class Tooltips;
    class VScrollbar;
}

class perfnames;
class perfroll;
class perftime;

/*
 * ca 2015-07-24
 * Just a note:  The patches in the pld-linux/seq24 GitHub project had a
 * namespace sigc declaration here, which does not seem to be needed.
 * And a lot of the patches from that project were already applied to
 * seq24 v 0.9.2.
 */

/**
 *  This class supports a Performance Editor that is used to arrange the
 *  patterns/sequences defined in the patterns panel, I think.
 *
 *  It has a seqroll and piano roll?  No, it has a perform, a perfnames, a
 *  perfroll, and a perftime.
 */

class perfedit: public Gtk::Window
{

private:

    perform * m_mainperf;

    Table * m_table;
    Adjustment * m_vadjust;
    Adjustment * m_hadjust;
    VScrollbar * m_vscroll;
    HScrollbar * m_hscroll;

    perfnames * m_perfnames;
    perfroll * m_perfroll;
    perftime * m_perftime;

    Menu * m_menu_snap;
    Button * m_button_snap;
    Entry * m_entry_snap;

    Button * m_button_stop;
    Button * m_button_play;
    ToggleButton * m_button_loop;

    Button * m_button_expand;
    Button * m_button_collapse;
    Button * m_button_copy;

    Button * m_button_grow;
    Button * m_button_undo;

    Button * m_button_bpm;
    Entry * m_entry_bpm;

    Button * m_button_bw;
    Entry * m_entry_bw;

    HBox * m_hbox;
    HBox * m_hlbox;

    Tooltips * m_tooltips;

    /**
     * Menus for time signature, beats per measure, beat width.
     */

    Menu * m_menu_bpm;
    Menu * m_menu_bw;

    /**
     * Set snap-to in "pulses".
     */

    int m_snap;
    int m_bpm;
    int m_bw;

public:

    perfedit (perform * a_perf);
    ~perfedit ();

    void init_before_show ();

private:

    void set_bpm (int a_beats_per_measure);
    void set_bw (int a_beat_width);
    void set_snap (int a_snap);
    void set_guides ();
    void grow ();
    void start_playing ();
    void stop_playing ();
    void set_looped ();
    void expand ();
    void collapse ();
    void copy ();
    void undo ();
    void popup_menu (Menu * a_menu);
    bool timeout ();

    void on_realize ();
    bool on_delete_event (GdkEventAny * a_event);
    bool on_key_press_event (GdkEventKey * a_ev);
};

#endif   // SEQ24_PERFEDIT_H

/*
 * perfedit.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
