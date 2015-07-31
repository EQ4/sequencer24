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
 * \file          mainwnd.cpp
 *
 *  This module declares/defines the base class for ...
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-07-26
 * \license       GNU GPLv2 or above
 *
 */

#include <cctype>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <gtk/gtkversion.h>

#include "gtk_helpers.h"
#include "mainwnd.h"
#include "perform.h"
#include "midifile.h"
#include "perfedit.h"

#include "pixmaps/play2.xpm"
#include "pixmaps/stop.xpm"
#include "pixmaps/learn.xpm"
#include "pixmaps/learn2.xpm"
#include "pixmaps/perfedit.xpm"
#include "pixmaps/seq24_32.xpm"
#include "pixmaps/sequencer24_square.xpm" // replaces "pixmaps/seq24.xpm"

/**
 *  Another global.
 */

bool is_pattern_playing = false;

/**
 *  The constructor the main window of the application.
 *  This constructor is way too large; it would be nicer to provide a
 *  number of well-named initialization functions.
 */

mainwnd::mainwnd (perform * a_p)
 :
    m_mainperf  (a_p),
    m_modified  (false),
    m_options   (NULL)

    /*
     * A lot more members should be listed here.
     */
{
    set_icon(Gdk::Pixbuf::create_from_xpm_data(seq24_32_xpm));
    m_mainperf->m_notify.push_back(this);   // register for notification
    update_window_title();                  // main window

#if GTK_MINOR_VERSION < 12
    m_tooltips = manage(new Tooltips());
#endif

    m_main_wid = manage(new mainwid(m_mainperf));
    m_main_time = manage(new maintime());
    m_menubar = manage(new MenuBar());
    m_menu_file = manage(new Menu());
    m_menubar->items().push_front(MenuElem("_File", *m_menu_file));
    m_menu_view = manage(new Menu());
    m_menubar->items().push_back(MenuElem("_View", *m_menu_view));
    m_menu_help = manage(new Menu());
    m_menubar->items().push_back(MenuElem("_Help", *m_menu_help));

    /**
     * File menu items, their accelerator keys, and their hot keys.
     */

    m_menu_file->items().push_back
    (
        MenuElem
        (
            "_New", Gtk::AccelKey("<control>N"),
            mem_fun(*this, &mainwnd::file_new)
        )
    );
    m_menu_file->items().push_back
    (
        MenuElem
        (
            "_Open...", Gtk::AccelKey("<control>O"),
            mem_fun(*this, &mainwnd::file_open)
        )
    );
    m_menu_file->items().push_back
    (
        MenuElem
        (
            "_Save", Gtk::AccelKey("<control>S"),
            mem_fun(*this, &mainwnd::file_save)
        )
    );
    m_menu_file->items().push_back
    (
        MenuElem
        (
            "Save _as...", mem_fun(*this, &mainwnd::file_save_as)
        )
    );
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back
    (
        MenuElem("_Import...", mem_fun(*this, &mainwnd::file_import_dialog))
    );
    m_menu_file->items().push_back
    (
        MenuElem("O_ptions...", mem_fun(*this, &mainwnd::options_dialog))
    );
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back
    (
        MenuElem
        (
            "E_xit", Gtk::AccelKey("<control>Q"),
            mem_fun(*this, &mainwnd::file_exit)
        )
    );

    /**
     * View menu items and their hot keys.
     */

    m_menu_view->items().push_back
    (
        MenuElem
        (
            "_Song Editor...", Gtk::AccelKey("<control>E"),
            mem_fun(*this, &mainwnd::open_performance_edit)
        )
    );

    /**
     * Help menu items
     */

    m_menu_help->items().push_back
    (
        MenuElem("_About...", mem_fun(*this, &mainwnd::about_dialog))
    );

    /**
     * Top panel items, including the logo (updated for the new version of
     * this application) and the "timeline" progress bar.
     */

    HBox * tophbox = manage(new HBox(false, 0));
    tophbox->pack_start
    (
        *manage
        (
            new Image(Gdk::Pixbuf::create_from_xpm_data(sequencer24_square_xpm))
        ), false, false
    );

    /* Adjust placement of the logo. */

    VBox * vbox_b = manage(new VBox());
    HBox * hbox3 = manage(new HBox(false, 0));
    vbox_b->pack_start(*hbox3, false, false);
    tophbox->pack_end(*vbox_b, false, false);
    hbox3->set_spacing(10);

    hbox3->pack_start(*m_main_time, false, false);  // timeline

    m_button_learn = manage(new Button());          // group learn ("L") button
    m_button_learn->set_focus_on_click(false);
    m_button_learn->set_flags(m_button_learn->get_flags() & ~Gtk::CAN_FOCUS);
    m_button_learn->set_image
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(learn_xpm)))
    );
    m_button_learn->signal_clicked().connect
    (
        mem_fun(*this, &mainwnd::learn_toggle)
    );
    add_tooltip
    (
        m_button_learn,
        "Mute Group Learn: "
        "Click 'L', then press a mute-group key to store the mute state of "
        "the sequences in that key. "
        "See File/Options/Keyboard for available mute-group keys "
        "and the corresponding hotkey for the 'L' button."
    );
    hbox3->pack_end(*m_button_learn, false, false);

    /*
     *  This seems to be a dirty hack to clear the focus, not to trigger L
     *  via keys.
     */

    Button w;
    hbox3->set_focus_child(w);

    /*
     *  Bottom panel items.  The first ones are the start/stop buttons and
     *  the container that groups them.
     */

    HBox * bottomhbox = manage(new HBox(false, 10));            // bottom box
    HBox * startstophbox = manage(new HBox(false, 4));          // button box
    bottomhbox->pack_start(*startstophbox, Gtk::PACK_SHRINK);
    m_button_stop = manage(new Button());                       // stop button
    m_button_stop->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(stop_xpm)))
    );
    m_button_stop->signal_clicked().connect
    (
        mem_fun(*this, &mainwnd::stop_playing)
    );
    add_tooltip(m_button_stop, "Stop playing MIDI sequence");
    startstophbox->pack_start(*m_button_stop, Gtk::PACK_SHRINK);

    m_button_play = manage(new Button());                       // play button
    m_button_play->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(play2_xpm)))
    );
    m_button_play->signal_clicked().connect
    (
        mem_fun(*this, &mainwnd::start_playing)
    );
    add_tooltip(m_button_play, "Play MIDI sequence");
    startstophbox->pack_start(*m_button_play, Gtk::PACK_SHRINK);

    /*
     * BPM spin button with label.
     */

    HBox * bpmhbox = manage(new HBox(false, 4));
    bottomhbox->pack_start(*bpmhbox, Gtk::PACK_SHRINK);
    m_adjust_bpm = manage(new Adjustment(m_mainperf->get_bpm(), 20, 500, 1));
    m_spinbutton_bpm = manage(new SpinButton(*m_adjust_bpm));
    m_spinbutton_bpm->set_editable(false);
    m_adjust_bpm->signal_value_changed().connect
    (
        mem_fun(*this, &mainwnd::adj_callback_bpm)
    );
    add_tooltip(m_spinbutton_bpm, "Adjust beats per minute (BPM) value");
    Label * bpmlabel = manage(new Label("_bpm", true));
    bpmlabel->set_mnemonic_widget(*m_spinbutton_bpm);
    bpmhbox->pack_start(*bpmlabel, Gtk::PACK_SHRINK);
    bpmhbox->pack_start(*m_spinbutton_bpm, Gtk::PACK_SHRINK);

    /*
     * Screen set name edit line.
     */

    HBox * notebox = manage(new HBox(false, 4));
    bottomhbox->pack_start(*notebox, Gtk::PACK_EXPAND_WIDGET);
    m_entry_notes = manage(new Entry());
    m_entry_notes->signal_changed().connect
    (
        mem_fun(*this, &mainwnd::edit_callback_notepad)
    );
    m_entry_notes->set_text
    (
        *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
    );
    add_tooltip(m_entry_notes, "Enter screen set name");
    Label * notelabel = manage(new Label("_Name", true));
    notelabel->set_mnemonic_widget(*m_entry_notes);
    notebox->pack_start(*notelabel, Gtk::PACK_SHRINK);
    notebox->pack_start(*m_entry_notes, Gtk::PACK_EXPAND_WIDGET);

    /*
     * Sequence screen set spin button.
     */

    HBox * sethbox = manage(new HBox(false, 4));
    bottomhbox->pack_start(*sethbox, Gtk::PACK_SHRINK);
    m_adjust_ss = manage(new Adjustment(0, 0, c_max_sets - 1, 1));
    m_spinbutton_ss = manage(new SpinButton(*m_adjust_ss));
    m_spinbutton_ss->set_editable(false);
    m_spinbutton_ss->set_wrap(true);
    m_adjust_ss->signal_value_changed().connect
    (
        mem_fun(*this, &mainwnd::adj_callback_ss)
    );
    add_tooltip(m_spinbutton_ss, "Select screen set");
    Label * setlabel = manage(new Label("_Set", true));
    setlabel->set_mnemonic_widget(*m_spinbutton_ss);
    sethbox->pack_start(*setlabel, Gtk::PACK_SHRINK);
    sethbox->pack_start(*m_spinbutton_ss, Gtk::PACK_SHRINK);

    /*
     * Song editor button.
     */

    m_button_perfedit = manage(new Button());
    m_button_perfedit->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(perfedit_xpm)))
    );
    m_button_perfedit->signal_clicked().connect
    (
        mem_fun(*this, &mainwnd::open_performance_edit)
    );
    add_tooltip(m_button_perfedit, "Show or hide song editor window");
    bottomhbox->pack_end(*m_button_perfedit, Gtk::PACK_SHRINK);

    /*
     * Vertical layout container for window content.
     */

    VBox * contentvbox = new VBox();
    contentvbox->set_spacing(10);
    contentvbox->set_border_width(10);
    contentvbox->pack_start(*tophbox, Gtk::PACK_SHRINK);
    contentvbox->pack_start(*m_main_wid, Gtk::PACK_SHRINK);
    contentvbox->pack_start(*bottomhbox, Gtk::PACK_SHRINK);

    /*
     * Main container for menu and window content.
     */

    VBox * mainvbox = new VBox();
    mainvbox->pack_start(*m_menubar, false, false);
    mainvbox->pack_start(*contentvbox);
    this->add(*mainvbox);                   // add main layout box
    show_all();                             // show everything

    add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
    m_timeout_connect = Glib::signal_timeout().connect
    (
        mem_fun(*this, &mainwnd::timer_callback), 25
    );
    m_perf_edit = new perfedit(m_mainperf);
    m_sigpipe[0] = -1;
    m_sigpipe[1] = -1;
    install_signal_handlers();
}

/**
 *  This destructor must explicitly delete some allocated resources.
 */

mainwnd::~mainwnd ()
{
    delete m_perf_edit;
    delete m_options;
    if (m_sigpipe[0] != -1)
        close(m_sigpipe[0]);

    if (m_sigpipe[1] != -1)
        close(m_sigpipe[1]);
}

/**
 *  This function is the GTK timer callback, used to draw our current time
 *  and BPM on_events (the main window).
 */

bool
mainwnd::timer_callback ()
{
    long ticks = m_mainperf->get_tick();
    m_main_time->idle_progress(ticks);
    m_main_wid->update_markers(ticks);
    if (m_adjust_bpm->get_value() != m_mainperf->get_bpm())
        m_adjust_bpm->set_value(m_mainperf->get_bpm());

    if (m_adjust_ss->get_value() != m_mainperf->get_screenset())
    {
        m_main_wid->set_screenset(m_mainperf->get_screenset());
        m_adjust_ss->set_value(m_mainperf->get_screenset());
        m_entry_notes->set_text
        (
            *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
        );
    }
    return true;
}

/**
 *  Opens the Performance Editor (Song Editor).
 */

void
mainwnd::open_performance_edit ()
{
    if (m_perf_edit->is_visible())
        m_perf_edit->hide();
    else
    {
        m_perf_edit->init_before_show();
        m_perf_edit->show_all();
        m_modified = true;
    }
}

/**
 *  Opens the File / Options dialog.
 */

void
mainwnd::options_dialog ()
{
    if (not_nullptr(m_options))
        delete m_options;

    m_options = new options(*this, m_mainperf);
    m_options->show_all();
}

/**
 *  Play!
 */

void
mainwnd::start_playing ()
{
    m_mainperf->position_jack(false);
    m_mainperf->start(false);
    m_mainperf->start_jack();
    is_pattern_playing = true;
}

/**
 *  Stop!
 */

void
mainwnd::stop_playing ()
{
    m_mainperf->stop_jack();
    m_mainperf->stop();
    m_main_wid->update_sequences_on_window();
    is_pattern_playing = false;
}

/**
 *  This handler responds to a learn-mode change from m_mainperf.
 */

void
mainwnd::on_grouplearnchange (bool state)
{
    m_button_learn->set_image
    (
        *manage
        (
            new Image
            (
                Gdk::Pixbuf::create_from_xpm_data(state ? learn2_xpm : learn_xpm)
            )
        )
    );
}

/**
 *  Toggle the group-learn status.
 */

void
mainwnd::learn_toggle ()
{
    if (m_mainperf->is_group_learning())
        m_mainperf->unset_mode_group_learn();
    else
        m_mainperf->set_mode_group_learn();
}

/**
 *  A callback function for the File / New menu entry.
 */

void
mainwnd::file_new ()
{
    if (is_save())
        new_file();
}

/**
 *  Actually does the work of setting up for a new file.
 */

void
mainwnd::new_file ()
{
    m_mainperf->clear_all();
    m_main_wid->reset();
    m_entry_notes->set_text
    (
        *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
    );
    global_filename = "";           // clear()
    update_window_title();
    m_modified = false;
}

/**
 *  A callback function for the File / Save menu entry.
 */

void
mainwnd::file_save ()
{
    save_file();
}

/**
 *  A callback function for the File / Save As menu entry.
 */

void
mainwnd::file_save_as ()
{
    Gtk::FileChooserDialog dialog("Save file as", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

    Gtk::FileFilter filter_midi;
    filter_midi.set_name("MIDI files");
    filter_midi.add_pattern("*.midi");
    filter_midi.add_pattern("*.mid");
    dialog.add_filter(filter_midi);

    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);

    dialog.set_current_folder(last_used_dir);
    int result = dialog.run();

    switch (result)
    {
        case Gtk::RESPONSE_OK:
        {
            bool result = false;
            std::string fname = dialog.get_filename();
            Gtk::FileFilter* current_filter = dialog.get_filter();
            if
            (
                (current_filter != NULL) &&
                (current_filter->get_name() == "MIDI files")
            )
            {
                /*
                 * Check for MIDI file extension; if missing, add ".midi".
                 */

                std::string suffix = fname.substr
                (
                    fname.find_last_of(".") + 1, std::string::npos
                );
                toLower(suffix);
                if ((suffix != "midi") && (suffix != "mid"))
                    fname += ".midi";
            }

            if (Glib::file_test(fname, Glib::FILE_TEST_EXISTS))
            {
                Gtk::MessageDialog warning
                (
                    *this,
                   "File already exists!\nDo you want to overwrite it?",
                   false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true
                );
                result = warning.run();
                if (result == Gtk::RESPONSE_NO)
                    return;
            }
            global_filename = fname;
            update_window_title();
            save_file();
            break;
        }
        default:
            break;
    }
}

/**
 *  Opens a MIDI file.
 */

void
mainwnd::open_file (const Glib::ustring & fn)
{
    bool result;
    m_mainperf->clear_all();

    midifile f(fn);                    // create object to represent file
    result = f.parse(m_mainperf, 0);
    m_modified = ! result;
    if (! result)
    {
        Gtk::MessageDialog errdialog
        (
            *this,
            "Error reading file: " + fn, false,
            Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true
        );
        errdialog.run();
        return;
    }

    last_used_dir = fn.substr(0, fn.rfind("/") + 1);
    global_filename = fn;
    update_window_title();
    m_main_wid->reset();
    m_entry_notes->set_text
    (
        *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
    );
    m_adjust_bpm->set_value(m_mainperf->get_bpm());
}

/**
 *  A callback function for the File / Open menu entry.
 */

void
mainwnd::file_open ()
{
    if (is_save())
        choose_file();
}

/**
 *  Creates a file-chooser dialog.
 */

void
mainwnd::choose_file ()
{
    Gtk::FileChooserDialog dlg("Open MIDI file", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dlg.set_transient_for(*this);
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    Gtk::FileFilter filter_midi;
    filter_midi.set_name("MIDI files");
    filter_midi.add_pattern("*.midi");
    filter_midi.add_pattern("*.mid");
    dlg.add_filter(filter_midi);

    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dlg.add_filter(filter_any);
    dlg.set_current_folder(last_used_dir);

    int result = dlg.run();
    switch (result)
    {
    case (Gtk::RESPONSE_OK):
        open_file(dlg.get_filename());
        break;

    default:
        break;
    }
}

/**
 *  Saves the current state in a MIDI file.
 */

bool mainwnd::save_file ()
{
    bool result = false;
    if (global_filename == "")
    {
        file_save_as();
        return true;
    }

    midifile f(global_filename);
    result = f.write(m_mainperf);
    if (! result)
    {
        Gtk::MessageDialog errdialog
        (
            *this, "Error writing file.", false,
            Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true
        );
        errdialog.run();
    }
    m_modified = ! result;
    return result;
}

/**
 *  Queries the user to save the changes made while the application was
 *  running.
 */

int mainwnd::query_save_changes ()
{
    Glib::ustring query_str;
    if (global_filename == "")
        query_str = "Unnamed file was changed.\nSave changes?";
    else
        query_str = "File '" + global_filename + "' was changed.\nSave changes?";

    Gtk::MessageDialog dialog
    (
        *this, query_str, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true
    );
    dialog.add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
    dialog.add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    return dialog.run();
}

/**
 *  If the data is modified, then the user is queried, and the file is
 *  save if okayed.
 */

bool mainwnd::is_save ()
{
    bool result = false;
    if (is_modified())
    {
        int choice = query_save_changes();
        switch (choice)
        {
        case Gtk::RESPONSE_YES:
            if (save_file())
                result = true;
            break;

        case Gtk::RESPONSE_NO:
            result = true;
            break;

        case Gtk::RESPONSE_CANCEL:
        default:
            break;
        }
    }
    else
        result = true;

    return result;
}

/**
 *  Converts a string to lower-case letters.
 */

void
mainwnd::toLower (std::string & s)
{
    for (std::string::iterator p = s.begin(); p != s.end(); p++)
        *p = tolower(*p);
}

/**
 *  Presents a file dialog to import a MIDI file.  Note that every track of
 *  the MIDI file will be imported, even if the track is only a label
 *  track (without any MIDI events) or a very long track.
 */

void
mainwnd::file_import_dialog ()
{
    Gtk::FileChooserDialog dlg("Import MIDI file", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dlg.set_transient_for(*this);

    Gtk::FileFilter filter_midi;
    filter_midi.set_name("MIDI files");
    filter_midi.add_pattern("*.midi");
    filter_midi.add_pattern("*.mid");
    dlg.add_filter(filter_midi);

    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dlg.add_filter(filter_any);
    dlg.set_current_folder(last_used_dir);

    ButtonBox * btnbox = dlg.get_action_area();
    HBox hbox(false, 2);
    m_adjust_load_offset = manage
    (
        new Adjustment(0, -(c_max_sets - 1), c_max_sets - 1, 1)
    );
    m_spinbutton_load_offset = manage(new SpinButton(*m_adjust_load_offset));
    m_spinbutton_load_offset->set_editable(false);
    m_spinbutton_load_offset->set_wrap(true);
    hbox.pack_end(*m_spinbutton_load_offset, false, false);
    hbox.pack_end(*(manage(new Label("Screen Set Offset"))), false, false, 4);
    btnbox->pack_start(hbox, false, false);
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
    dlg.show_all_children();

    int result = dlg.run();

    switch (result)                    // handle the response
    {
    case (Gtk::RESPONSE_OK):
    {
        try
        {
            midifile f(dlg.get_filename());
            f.parse(m_mainperf, (int) m_adjust_load_offset->get_value());
        }
        catch (...)
        {
            Gtk::MessageDialog errdialog
            (
                *this, "Error reading file.", false,
                Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true
            );
            errdialog.run();
        }

        global_filename = std::string(dlg.get_filename());
        update_window_title();
        m_modified = true;
        m_main_wid->reset();
        m_entry_notes->set_text
        (
            *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
        );
        m_adjust_bpm->set_value(m_mainperf->get_bpm());
        break;
    }

    case (Gtk::RESPONSE_CANCEL):
        break;

    default:
        break;

    }
}

/**
 *  A callback function for the File / Exit menu entry.
 */

void
mainwnd::file_exit ()
{
    if (is_save())
    {
        if (is_pattern_playing)
            stop_playing();

        hide();
    }
}

/**
 *  This callback function handles a delete event from ...?
 *
 *  Any changed data is saved.  If the pattern is playing, then it is
 *  stopped.
 */

bool
mainwnd::on_delete_event (GdkEventAny * a_e)
{
    bool result = is_save();
    if (result && is_pattern_playing)
        stop_playing();

    return ! result;
}

/**
 *  Presents a Help / About dialog.  I (Chris) took the liberty of tacking
 *  my name at the end, and hope to eventually have done enough work to
 *  warrant having it there.
 */

void
mainwnd::about_dialog ()
{
    Gtk::AboutDialog dialog;
    dialog.set_transient_for(*this);
    dialog.set_name(PACKAGE_NAME);
    dialog.set_version(VERSION);
    dialog.set_comments("Interactive MIDI Sequencer\n");
    dialog.set_copyright
    (
        "(C) 2002 - 2006 Rob C. Buse\n"
        "(C) 2008 - 2010 Seq24team"
        "(C) 2015 Chris Ahlstrom"
    );
    dialog.set_website
    (
        "http://www.filter24.org/seq24\n"
        "http://edge.launchpad.net/seq24"
        "https://github.com/ahlstromcj/sequencer24.git"
    );

    std::list<Glib::ustring> list_authors;
    list_authors.push_back("Rob C. Buse <rcb@filter24.org>");
    list_authors.push_back("Ivan Hernandez <ihernandez@kiusys.com>");
    list_authors.push_back("Guido Scholz <guido.scholz@bayernline.de>");
    list_authors.push_back("Jaakko Sipari <jaakko.sipari@gmail.com>");
    list_authors.push_back("Peter Leigh <pete.leigh@gmail.com>");
    list_authors.push_back("Anthony Green <green@redhat.com>");
    list_authors.push_back("Daniel Ellis <mail@danellis.co.uk>");
    list_authors.push_back("Sebastien Alaiwan <sebastien.alaiwan@gmail.com>");
    list_authors.push_back("Kevin Meinert <kevin@subatomicglue.com>");
    list_authors.push_back("Andrea delle Canne <andreadellecanne@gmail.com>");
    list_authors.push_back("Chris Ahlstrom<ahlstromcj@gmail.com>");
    dialog.set_authors(list_authors);

    std::list<Glib::ustring> list_documenters;
    list_documenters.push_back("Dana Olson <seq24@ubuntustudio.com>");
    list_documenters.push_back("Chris Ahlstrom<ahlstromcj@gmail.com>");
    list_documenters.push_back("https://github.com/ahlstromcj/seq24-doc.git");
    dialog.set_documenters(list_documenters);

    dialog.show_all_children();
    dialog.run();
}

/**
 *  This function is the callback for adjusting the screen-set value.
 *
 *  Sets the screen set value in the Performance/Song window, the
 *  Patterns, and something about setting the text based on a screen-set
 *  notepad from the Performance/Song window.
 *
 *  Screen-set notepad?
 */

void
mainwnd::adj_callback_ss ()
{
    m_mainperf->set_screenset((int) m_adjust_ss->get_value());
    m_main_wid->set_screenset(m_mainperf->get_screenset());
    m_entry_notes->set_text
    (
        *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
    );
    m_modified = true;
}

/**
 *  This function is the callback for adjusting the BPM value.
 */

void
mainwnd::adj_callback_bpm ()
{
    m_mainperf->set_bpm((int) m_adjust_bpm->get_value());
    m_modified = true;
}

/**
 *  A callback function for handling an edit to the screen-set notepad.
 */

void
mainwnd::edit_callback_notepad ()
{
    string text = m_entry_notes->get_text();
    m_mainperf->set_screen_set_notepad(m_mainperf->get_screenset(), &text);
    m_modified = true;
}

/**
 *  Handles a key release event.
 */

bool
mainwnd::on_key_release_event (GdkEventKey * a_ev)
{
    if (a_ev->keyval == m_mainperf->m_key_replace)
        m_mainperf->unset_sequence_control_status(c_status_replace);

    if (a_ev->keyval == m_mainperf->m_key_queue)
        m_mainperf->unset_sequence_control_status(c_status_queue);

    if (a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
            a_ev->keyval == m_mainperf->m_key_snapshot_2)
        m_mainperf->unset_sequence_control_status(c_status_snapshot);

    if (a_ev->keyval == m_mainperf->m_key_group_learn)
        m_mainperf->unset_mode_group_learn();

    return false;
}

/**
 *  Handles a key press event.
 */

bool
mainwnd::on_key_press_event (GdkEventKey * a_ev)
{
    Gtk::Window::on_key_press_event(a_ev);
    if (a_ev->type == GDK_KEY_PRESS)
    {
        /*
         * Handle the control-key and modifier-key combinations matching
         * the entries in this list of if statements.
         */

        if (global_print_keys)
        {
            printf("key_press[%d]\n", a_ev->keyval);
            fflush(stdout);
        }

        if (a_ev->keyval == m_mainperf->m_key_bpm_dn)
        {
            m_mainperf->set_bpm(m_mainperf->get_bpm() - 1);
            m_adjust_bpm->set_value(m_mainperf->get_bpm());
        }

        if (a_ev->keyval ==  m_mainperf->m_key_bpm_up)
        {
            m_mainperf->set_bpm(m_mainperf->get_bpm() + 1);
            m_adjust_bpm->set_value(m_mainperf->get_bpm());
        }

        if (a_ev->keyval == m_mainperf->m_key_replace)
        {
            m_mainperf->set_sequence_control_status(c_status_replace);
        }

        if
        (
            (a_ev->keyval == m_mainperf->m_key_queue) ||
            (a_ev->keyval == m_mainperf->m_key_keep_queue)
        )
        {
            m_mainperf->set_sequence_control_status(c_status_queue);
        }

        if
        (
            a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
            a_ev->keyval == m_mainperf->m_key_snapshot_2
        )
        {
            m_mainperf->set_sequence_control_status(c_status_snapshot);
        }

        if (a_ev->keyval == m_mainperf->m_key_screenset_dn)
        {

            m_mainperf->set_screenset(m_mainperf->get_screenset() - 1);
            m_main_wid->set_screenset(m_mainperf->get_screenset());
            m_adjust_ss->set_value(m_mainperf->get_screenset());
            m_entry_notes->set_text
            (
                *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
            );
        }

        if (a_ev->keyval == m_mainperf->m_key_screenset_up)
        {

            m_mainperf->set_screenset(m_mainperf->get_screenset() + 1);
            m_main_wid->set_screenset(m_mainperf->get_screenset());
            m_adjust_ss->set_value(m_mainperf->get_screenset());
            m_entry_notes->set_text
            (
                *m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset())
            );
        }

        if (a_ev->keyval == m_mainperf->m_key_set_playing_screenset)
        {
            m_mainperf->set_playing_screenset();
        }

        if (a_ev->keyval == m_mainperf->m_key_group_on)
        {
            m_mainperf->set_mode_group_mute();
        }

        if (a_ev->keyval == m_mainperf->m_key_group_off)
        {
            m_mainperf->unset_mode_group_mute();
        }

        if (a_ev->keyval == m_mainperf->m_key_group_learn)
        {
            m_mainperf->set_mode_group_learn();
        }

        if (m_mainperf->get_key_groups()->count(a_ev->keyval) != 0)
        {
            // activate mute group key

            m_mainperf->select_and_mute_group
            (
                m_mainperf->lookup_keygroup_group(a_ev->keyval)
            );
        }

        if
        (
            m_mainperf->is_learn_mode() &&
            a_ev->keyval != m_mainperf->m_key_group_learn
        )
        {
            // mute group learn

            if (m_mainperf->get_key_groups()->count(a_ev->keyval) != 0)
            {
                std::ostringstream os;
                os << "Key \""
                   << gdk_keyval_name(a_ev->keyval)
                   << "\" (code = "
                   << a_ev->keyval
                   << ") successfully mapped."
                   ;

                Gtk::MessageDialog dialog
                (
                    *this, "MIDI mute group learn success", false,
                    Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true
                );
                dialog.set_secondary_text(os.str(), false);
                dialog.run();

                /*
                 * Missed the key-up message for group-learn, so force it
                 * to off
                 */

                m_mainperf->unset_mode_group_learn();
            }
            else
            {
                std::ostringstream os;
                os << "Key \""
                   << gdk_keyval_name(a_ev->keyval)
                   << "\" (code = "
                   << a_ev->keyval
                   << ") is not one of the configured mute-group keys.\n"
                   << "To change this see File/Options menu or .seq24rc"
                   ;

                Gtk::MessageDialog dialog
                (
                    *this, "MIDI mute group learn failed", false,
                    Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true
                );

                dialog.set_secondary_text(os.str(), false);
                dialog.run();

                /*
                 * Missed the key-up message for group-learn, so force it
                 * to off
                 */

                m_mainperf->unset_mode_group_learn();
            }
        }

        /*
         * The start/end key may be the same key (i.e. SPACE) to allow
         * toggling when the same key is mapped to both triggers (i.e.
         * SPACEBAR)
         */

        bool dont_toggle = m_mainperf->m_key_start != m_mainperf->m_key_stop;
        if
        (
            a_ev->keyval == m_mainperf->m_key_start &&
            (dont_toggle || !is_pattern_playing)
        )
        {
            start_playing();
        }
        else if
        (
            a_ev->keyval == m_mainperf->m_key_stop &&
            (dont_toggle || is_pattern_playing))
        {
            stop_playing();
        }

        /*
         * Toggle the sequence mute/unmute setting using keyboard keys.
         */

        if (m_mainperf->get_key_events()->count(a_ev->keyval) != 0)
        {
            sequence_key(m_mainperf->lookup_keyevent_seq(a_ev->keyval));
        }
    }
    return false;
}

/**
 *  Handle a sequence key to toggle the palying of an active pattern in
 *  the selected screen-set.
 */

void
mainwnd::sequence_key (int a_seq)
{
    int offset = m_mainperf->get_screenset() * c_mainwnd_rows * c_mainwnd_cols;
    if (m_mainperf->is_active(a_seq + offset))
    {
        m_mainperf->sequence_playing_toggle(a_seq + offset);
    }
}

/**
 *  Updates the title shown in the title bar of the window.  Note that the
 *  name of the application is obtained by the "(PACKAGE)} construction.
 *  Never saw that thing before.  Perhaps it is a Glib thing.
 */

void
mainwnd::update_window_title ()
{
    std::string title;
    if (global_filename == "")
        title = (PACKAGE) + string(" - [unnamed]");
    else
        title = (PACKAGE) + string(" - [")
            + Glib::filename_to_utf8(global_filename)
            + string("]");

    set_title(title.c_str());
}

/**
 * \getter m_modified
 */

bool
mainwnd::is_modified ()
{
    return m_modified;
}

/**
 *  Provides a couple pipes for signalling/messaging.
 */

int mainwnd::m_sigpipe[2];


/**
 *  This function is the handler for system signals (SIGUSR1, SIGINT...)
 *  It Writes a message to the pipe and leaves as soon as possible.
 */

void
mainwnd::handle_signal (int sig)
{
    if (write(m_sigpipe[1], &sig, sizeof(sig)) == -1)
    {
        printf("signal write() failed: %s\n", std::strerror(errno));
    }
}

/**
 *  Installs the signal handlers and pipe code.
 */

bool
mainwnd::install_signal_handlers ()
{
    // install pipe to forward received system signals

    if (pipe(m_sigpipe) < 0)
    {
        printf("pipe() failed: %s\n", std::strerror(errno));
        return false;
    }

    // install a notifier to handle pipe messages

    Glib::signal_io().connect
    (
        sigc::mem_fun(*this, &mainwnd::signal_action), m_sigpipe[0], Glib::IO_IN
    );

    // install signal handlers

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    if (sigaction(SIGUSR1, &action, NULL) == -1)
    {
        printf("sigaction() failed: %s\n", std::strerror(errno));
        return false;
    }

    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        printf("sigaction() failed: %s\n", std::strerror(errno));
        return false;
    }
    return true;
}

/**
 *
 */

bool
mainwnd::signal_action (Glib::IOCondition condition)
{
    if ((condition & Glib::IO_IN) == 0)
    {
        printf("Error: unexpected IO condition\n");
        return false;
    }

    int message;
    if (read(m_sigpipe[0], &message, sizeof(message)) == -1)
    {
        printf("read() failed: %s\n", std::strerror(errno));
        return false;
    }

    switch (message)
    {
    case SIGUSR1:
        save_file();
        break;

    case SIGINT:
        file_exit();
        break;

    default:
        printf("Unexpected signal received: %d\n", message);
        break;
    }
    return true;
}

/*
 * mainwnd.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
