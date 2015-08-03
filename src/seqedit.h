#ifndef SEQ24_SEQEDIT_H
#define SEQ24_SEQEDIT_H

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
 * \file          seqedit.h
 *
 *  This module declares/defines the base class for editing a
 *  pattern/sequence.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-01
 * \license       GNU GPLv2 or above
 *
 */

#include <list>
#include <string>
#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/window.h>
#include <gtkmm/table.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/widget.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/viewport.h>
#include <gtkmm/combo.h>
#include <gtkmm/label.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/invisible.h>
#include <gtkmm/separator.h>
#include <gtkmm/image.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/invisible.h>
#include <gtkmm/image.h>
#include <sigc++/bind.h>

#include "globals.h"
#include "mainwid.h"
#include "sequence.h"
#include "seqkeys.h"
#include "seqroll.h"
#include "seqdata.h"
#include "seqtime.h"
#include "seqevent.h"
#include "perform.h"

using namespace Gtk;

/**
 *  Implements the Pattern Editor, which has references to:
 *
 *      -   perform
 *      -   seqroll
 *      -   seqkeys
 *      -   seqdata
 *      -   seqtime
 *      -   seqevent
 *
 *  This class has a metric ton of user-interface objects and other
 *  members.
 */

class seqedit : public Gtk::Window
{

private:

    static const int c_min_zoom = 1;
    static const int c_max_zoom = 32;
    static int m_initial_zoom;
    static int m_initial_snap;
    static int m_initial_note_length;
    static int m_initial_scale;
    static int m_initial_key;
    static int m_initial_sequence;

    /**
     *  Provides the zoom values: 0  1  2  3  4, and 1, 2, 4, 8, 16.
     */

    int m_zoom;

    /**
     *  Use in setting the snap-to in pulses, off = 1.
     */

    int m_snap;
    int m_note_length;

    /**
     *  Settings for the music scale and key.
     */

    int m_scale;
    int m_key;
    int m_sequence;
    long m_measures;

    sequence * const m_seq;
    perform * const m_mainperf;
    MenuBar * m_menubar;
    Menu * m_menu_tools;
    Menu * m_menu_zoom;
    Menu * m_menu_snap;
    Menu * m_menu_note_length;

    /**
     *  Provides the length in measures.
     */

    Menu * m_menu_length;
    Menu * m_menu_midich;
    Menu * m_menu_midibus;
    Menu * m_menu_data;
    Menu * m_menu_key;
    Menu * m_menu_scale;
    Menu * m_menu_sequences;

    /**
     *  These member provife the time signature, beats per measure, and
     *  beat width menus.
     */

    Menu * m_menu_bpm;
    Menu * m_menu_bw;
    Menu * m_menu_rec_vol;
    int m_pos;
    Adjustment * m_vadjust;
    Adjustment * m_hadjust;
    VScrollbar * m_vscroll_new;
    HScrollbar * m_hscroll_new;
    seqkeys * m_seqkeys_wid;
    seqtime * m_seqtime_wid;
    seqdata * m_seqdata_wid;
    seqevent * m_seqevent_wid;
    seqroll * m_seqroll_wid;
    Table * m_table;
    VBox * m_vbox;
    HBox * m_hbox;
    HBox * m_hbox2;
    HBox * m_hbox3;
    Button * m_button_undo;
    Button * m_button_redo;
    Button * m_button_quantize;
    Button * m_button_tools;
    Button * m_button_sequence;
    Entry * m_entry_sequence;
    Button * m_button_bus;
    Entry * m_entry_bus;
    Button * m_button_channel;
    Entry * m_entry_channel;
    Button * m_button_snap;
    Entry * m_entry_snap;
    Button * m_button_note_length;
    Entry * m_entry_note_length;
    Button * m_button_zoom;
    Entry * m_entry_zoom;
    Button * m_button_length;
    Entry * m_entry_length;
    Button * m_button_key;
    Entry * m_entry_key;
    Button * m_button_scale;
    Entry * m_entry_scale;
    Tooltips* m_tooltips;
    Button * m_button_data;
    Entry * m_entry_data;
    Button * m_button_bpm;
    Entry * m_entry_bpm;
    Button * m_button_bw;
    Entry * m_entry_bw;
    Button * m_button_rec_vol;
    ToggleButton * m_toggle_play;
    ToggleButton * m_toggle_record;
    ToggleButton * m_toggle_q_rec;
    ToggleButton * m_toggle_thru;
    RadioButton * m_radio_select;
    RadioButton * m_radio_grow;
    RadioButton * m_radio_draw;
    Entry * m_entry_name;

    /**
     *  Indicates what is the data window currently editing?
     */

    unsigned char m_editing_status;
    unsigned char m_editing_cc;

public:

    seqedit (sequence * a_seq, perform * a_perf, int a_pos);
    ~seqedit ();

private:

    void set_zoom (int a_zoom);
    void set_snap (int a_snap);
    void set_note_length (int a_note_length);
    void set_bpm (int a_beats_per_measure);
    void set_bw (int a_beat_width);
    void set_rec_vol (int a_rec_vol);
    void set_measures (int a_length_measures);
    void apply_length (int a_bpm, int a_bw, int a_measures);
    long get_measures ();
    void set_midi_channel (int a_midichannel);
    void set_midi_bus (int a_midibus);
    void set_scale (int a_scale);
    void set_key (int a_note);
    void set_background_sequence (int a_seq);
    void name_change_callback ();
    void play_change_callback ();
    void record_change_callback ();
    void q_rec_change_callback ();
    void thru_change_callback ();
    void undo_callback ();
    void redo_callback ();
    void set_data_type (unsigned char a_status, unsigned char a_control = 0);
    void update_all_windows ();
    void fill_top_bar ();
    void create_menus ();

    /*
     * An unsed, empty function:    void menu_action_quantise ();
     */

    void popup_menu (Menu * a_menu);
    void popup_event_menu ();
    void popup_midibus_menu ();
    void popup_sequence_menu ();
    void popup_tool_menu ();
    void popup_midich_menu ();
    Gtk::Image * create_menu_image (bool a_state = false);
    bool timeout ();
    void do_action (int a_action, int a_var);
    void mouse_action (mouse_action_e a_action);

private:        // callbacks

    void on_realize ();
    bool on_delete_event (GdkEventAny * a_event);
    bool on_scroll_event (GdkEventScroll * a_ev);
    bool on_key_press_event (GdkEventKey * a_ev);
};

#endif   // SEQ24_SEQEDIT_H

/*
 * seqedit.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
