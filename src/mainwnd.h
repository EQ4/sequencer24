#ifndef SEQ24_MAINWINDOW_H
#define SEQ24_MAINWINDOW_H

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
 * \file          mainwnd.h
 *
 *  This module declares/defines the base class for the main window.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-02
 * \license       GNU GPLv2 or above
 *
 */

#include <map>
#include <string>
#include <gtkmm.h>

#include "globals.h"
#include "mainwid.h"
#include "perform.h"
#include "sequence.h"
#include "event.h"
#include "maintime.h"
#include "perfedit.h"
#include "options.h"

using namespace Gtk;
using namespace Menu_Helpers;

/**
 *  This class implements the functionality of the main window of the
 *  application, except for the Patterns Panel functionality, which is
 *  implemented in the mainwid class.
 */

class mainwnd : public Gtk::Window, public performcallback
{

private:

    /**
     *  Interesting; what is this used for.
     */

    static int m_sigpipe[2];

    /**
     *  Management object (?) for the performance mode.
     */

    perform * m_mainperf;

    /**
     *  Flag for modifications to the loaded file.
     */

    bool m_modified;

#if GTK_MINOR_VERSION < 12
    Tooltips * m_tooltips;
#endif

    /**
     *  Theses objects support the menu and its sub-menus.
     */

    MenuBar * m_menubar;
    Menu * m_menu_file;
    Menu * m_menu_view;
    Menu * m_menu_help;

    /**
     *  The biggest sub-components of mainwnd.  The first is the Patterns
     *  Panel.
     */

    mainwid * m_main_wid;

    /**
     *  Is this the bar at the top that shows moving squares?
     */

    maintime * m_main_time;

    /**
     *  A pointer to the song/performance editor.
     */

    perfedit * m_perf_edit;

    /**
     *  A pointer to the program options.
     */

    options * m_options;

    /**
     *  Mouse cursor?
     */

    Gdk::Cursor m_main_cursor;

    /**
     *  This button is the learn button, otherwise known as the "L"
     *  button.
     */

    Button * m_button_learn;

    /**
     *  Implements the red square stop button.
     */

    Button * m_button_stop;

    /**
     *  Implements the green triangle play button.
     */

    Button * m_button_play;

    /**
     *  The button for bringing up the Song Editor (Performance Editor).
     */

    Button * m_button_perfedit;

    /**
     *  The spin/adjustment controls for the BPM (beats-per-minute) value.
     */

    SpinButton * m_spinbutton_bpm;
    Adjustment * m_adjust_bpm;

    /**
     *  The spin/adjustment controls for the screen set value.
     */

    SpinButton * m_spinbutton_ss;
    Adjustment * m_adjust_ss;

    /**
     *  The spin/adjustment controls for the load offset value.
     *  However, where is this button located?  It is handled in the code,
     *  but I've never seen the button!
     */

    SpinButton * m_spinbutton_load_offset;
    Adjustment * m_adjust_load_offset;

    /**
     *  What is this?
     */

    Entry * m_entry_notes;

    /**
     *  Provides a timeout handler?
     */

    sigc::connection m_timeout_connect;

public:

    mainwnd (perform * a_p);
    ~mainwnd ();

    void open_file (const Glib::ustring &);

private:

    void file_import_dialog ();
    void options_dialog ();
    void about_dialog ();

    void adj_callback_ss ();                // make 'em void at some point
    void adj_callback_bpm ();
    void edit_callback_notepad ();
    bool timer_callback ();

    void start_playing ();
    void stop_playing ();
    void learn_toggle ();
    void open_performance_edit ();
    void sequence_key (int a_seq);
    void update_window_title ();
    void toLower (std::string &);    // isn't this std::string?
    bool is_modified ();
    void file_new ();
    void file_open ();
    void file_save ();
    void file_save_as ();
    void file_exit ();
    void new_file ();
    bool save_file ();
    void choose_file ();
    int query_save_changes ();
    bool is_save ();
    static void handle_signal (int sig);
    bool install_signal_handlers ();
    bool signal_action (Glib::IOCondition condition);

private:

    bool on_delete_event (GdkEventAny * a_e);
    bool on_key_press_event (GdkEventKey * a_ev);
    bool on_key_release_event (GdkEventKey * a_ev);

private:

    /**
     *  Notification handler for learn mode toggle.
     */

    virtual void on_grouplearnchange (bool state);

};

#endif   // SEQ24_MAINWINDOW_H

/*
 * mainwnd.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
