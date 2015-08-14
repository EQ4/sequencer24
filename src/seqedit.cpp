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
 * \file          seqedit.cpp
 *
 *  This module declares/defines the base class for editing a
 *  pattern/sequence.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-09
 * \license       GNU GPLv2 or above
 *
 */

#include <gtkmm/adjustment.h>
#include <gtkmm/image.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/combo.h>               // Gtk::Entry, ToggleButton, RadioButton
#include <gtkmm/label.h>
#include <gtkmm/separator.h>
#include <gtkmm/table.h>
#include <gtkmm/tooltips.h>

#include <sigc++/bind.h>

#include "globals.h"
#include "controllers.h"
#include "event.h"
#include "gtk_helpers.h"
#include "midibus.h"
#include "options.h"
#include "perform.h"
#include "seqdata.h"
#include "seqedit.h"
#include "seqevent.h"
#include "seqkeys.h"
#include "seqroll.h"
#include "seqtime.h"
#include "sequence.h"

#include "pixmaps/play.xpm"
#include "pixmaps/q_rec.xpm"
#include "pixmaps/rec.xpm"
#include "pixmaps/thru.xpm"
#include "pixmaps/bus.xpm"
#include "pixmaps/midi.xpm"
#include "pixmaps/snap.xpm"
#include "pixmaps/zoom.xpm"
#include "pixmaps/length.xpm"
#include "pixmaps/scale.xpm"
#include "pixmaps/key.xpm"
#include "pixmaps/down.xpm"
#include "pixmaps/note_length.xpm"
#include "pixmaps/undo.xpm"
#include "pixmaps/redo.xpm"
#include "pixmaps/quantize.xpm"
#include "pixmaps/menu_empty.xpm"
#include "pixmaps/menu_full.xpm"
#include "pixmaps/sequences.xpm"
#include "pixmaps/tools.xpm"
#include "pixmaps/seq-editor.xpm"

/**
 * Static data members.
 */

int seqedit::m_initial_zoom = 2;
int seqedit::m_initial_snap = c_ppqn / 4;
int seqedit::m_initial_note_length = c_ppqn / 4;
int seqedit::m_initial_scale = 0;
int seqedit::m_initial_key = 0;
int seqedit::m_initial_sequence = -1;

/**
 * Actions
 */

static const int c_select_all_notes      =  1;
static const int c_select_all_events     =  2;
static const int c_select_inverse_notes  =  3;
static const int c_select_inverse_events =  4;
static const int c_quantize_notes        =  5;
static const int c_quantize_events       =  6;
static const int c_tighten_events        =  8;
static const int c_tighten_notes         =  9;
static const int c_transpose             = 10;
static const int c_transpose_h           = 12;

/**
 *  Connects to a menu item, tells the performance to launch the timer
 *  thread.  But this is an unused, empty function.
 *
 *      void seqedit::menu_action_quantise () { }
 */

/**
 *  Principal constructor.
 */

seqedit::seqedit (sequence * a_seq, perform * a_perf, int a_pos)
 :
    m_zoom                  (m_initial_zoom),
    m_snap                  (m_initial_snap),
    m_note_length           (m_initial_note_length),
    m_scale                 (m_initial_scale),
    m_key                   (m_initial_key),
    m_sequence              (m_initial_sequence),
    m_measures              (0),
    m_seq                   (a_seq),
    m_mainperf              (a_perf),               /* set the performance */
    m_menubar               (manage(new Gtk::MenuBar())),
    m_menu_tools            (manage(new Gtk::Menu())),
    m_menu_zoom             (manage(new Gtk::Menu())),
    m_menu_snap             (manage(new Gtk::Menu())),
    m_menu_note_length      (manage(new Gtk::Menu())),
    m_menu_length           (manage(new Gtk::Menu())),
    m_menu_midich           (nullptr),
    m_menu_midibus          (nullptr),
    m_menu_data             (nullptr),
    m_menu_key              (manage(new Gtk::Menu())),
    m_menu_scale            (manage(new Gtk::Menu())),
    m_menu_sequences        (nullptr),
    m_menu_bpm              (manage(new Gtk::Menu())),
    m_menu_bw               (manage(new Gtk::Menu())),
    m_menu_rec_vol          (manage(new Gtk::Menu())),
    m_pos                   (a_pos),
    m_vadjust
    (
        manage(new Gtk::Adjustment(55, 0, c_num_keys, 1, 1, 1))
    ),
    m_hadjust               (manage(new Gtk::Adjustment(0, 0, 1, 1, 1, 1))),
    m_vscroll_new           (manage(new Gtk::VScrollbar(*m_vadjust))),
    m_hscroll_new           (manage(new Gtk::HScrollbar(*m_hadjust))),
    m_seqkeys_wid           (manage(new seqkeys(m_seq, m_vadjust))),
    m_seqtime_wid           (manage(new seqtime(m_seq, m_zoom, m_hadjust))),
    m_seqdata_wid           (manage(new seqdata(m_seq, m_zoom, m_hadjust))),
    m_seqevent_wid
    (
        manage(new seqevent(m_seq, m_zoom, m_snap, m_seqdata_wid, m_hadjust))
    ),
    m_seqroll_wid
    (
        manage(new seqroll(m_mainperf, m_seq, m_zoom, m_snap, m_seqdata_wid,
            m_seqevent_wid, m_seqkeys_wid, m_pos, m_hadjust, m_vadjust))
    ),
    m_table                 (manage(new Gtk::Table(7, 4, false))),
    m_vbox                  (manage(new Gtk::VBox(false, 2))),
    m_hbox                  (manage(new Gtk::HBox(false, 2))),
    m_hbox2                 (manage(new Gtk::HBox(false, 2))),
    m_hbox3                 (manage(new Gtk::HBox(false, 2))),
    m_button_undo           (nullptr),
    m_button_redo           (nullptr),
    m_button_quantize       (nullptr),
    m_button_tools          (nullptr),
    m_button_sequence       (nullptr),
    m_entry_sequence        (nullptr),
    m_button_bus            (nullptr),
    m_entry_bus             (nullptr),
    m_button_channel        (nullptr),
    m_entry_channel         (nullptr),
    m_button_snap           (nullptr),
    m_entry_snap            (nullptr),
    m_button_note_length    (nullptr),
    m_entry_note_length     (nullptr),
    m_button_zoom           (nullptr),
    m_entry_zoom            (nullptr),
    m_button_length         (nullptr),
    m_entry_length          (nullptr),
    m_button_key            (nullptr),
    m_entry_key             (nullptr),
    m_button_scale          (nullptr),
    m_entry_scale           (nullptr),
    m_tooltips              (manage(new Gtk::Tooltips())),
    m_button_data           (manage(new Gtk::Button(" Event "))),
    m_entry_data            (manage(new Gtk::Entry())),
    m_button_bpm            (nullptr),
    m_entry_bpm             (nullptr),
    m_button_bw             (nullptr),
    m_entry_bw              (nullptr),
    m_button_rec_vol        (manage(new Gtk::Button())),
    m_toggle_play           (manage(new Gtk::ToggleButton())),
    m_toggle_record         (manage(new Gtk::ToggleButton())),
    m_toggle_q_rec          (manage(new Gtk::ToggleButton())),
    m_toggle_thru           (manage(new Gtk::ToggleButton())),
    m_radio_select          (nullptr),
    m_radio_grow            (nullptr),
    m_radio_draw            (nullptr),
    m_entry_name            (nullptr),
    m_editing_status        (0),
    m_editing_cc            (0)
{
    std::string title = "seq24 - ";                         /* main window */
    set_icon(Gdk::Pixbuf::create_from_xpm_data(seq_editor_xpm));
    title.append(m_seq->get_name());
    set_title(title);
    set_size_request(700, 500);
    m_seq->set_editing(true);
    create_menus();

//  m_tooltips = manage(new Gtk::Tooltips()); /* tooltips */

    Gtk::HBox * dhbox = manage(new Gtk::HBox(false, 2));
    m_vbox->set_border_width(2);

    /* fill table */

    m_table->attach(*m_seqkeys_wid, 0, 1, 1, 2, Gtk::SHRINK, Gtk::FILL);
    m_table->attach(*m_seqtime_wid, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
    m_table->attach
    (
        *m_seqroll_wid, 1, 2, 1, 2, Gtk::FILL |  Gtk::SHRINK,
        Gtk::FILL |  Gtk::SHRINK
    );
    m_table->attach(*m_seqevent_wid, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
    m_table->attach(*m_seqdata_wid, 1, 2, 3, 4, Gtk::FILL, Gtk::SHRINK);
    m_table->attach
    (
        *dhbox, 1, 2, 4, 5, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK, 0, 2
    );
    m_table->attach
    (
        *m_vscroll_new, 2, 3, 1, 2, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND
    );
    m_table->attach
    (
        *m_hscroll_new, 1, 2, 5, 6, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK
    );

    /* no expand, just fit the widgets */

    m_vbox->pack_start(*m_hbox,  false, false, 0);
    m_vbox->pack_start(*m_hbox2, false, false, 0);
    m_vbox->pack_start(*m_hbox3, false, false, 0);

    /* expand, cause rollview expands */

    m_vbox->pack_start(*m_table, true, true, 0);
    m_button_data->signal_clicked().connect                 /* data button */
    (
        mem_fun(*this, &seqedit::popup_event_menu)
    );
    m_entry_data->set_size_request(40, -1);
    m_entry_data->set_editable(false);
    dhbox->pack_start(*m_button_data, false, false);
    dhbox->pack_start(*m_entry_data, true, true);

    /* play, rec, thru */

    m_toggle_play->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(play_xpm)))
    );
    m_toggle_play->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::play_change_callback)
    );
    add_tooltip(m_toggle_play, "Sequence dumps data to MIDI bus.");
    m_toggle_record->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(rec_xpm)))
    );
    m_toggle_record->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::record_change_callback)
    );
    add_tooltip(m_toggle_record, "Records incoming MIDI data.");
    m_toggle_q_rec->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(q_rec_xpm)))
    );
    m_toggle_q_rec->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::q_rec_change_callback)
    );
    add_tooltip(m_toggle_q_rec, "Quantized record.");
    m_button_rec_vol->add(*manage(new Gtk::Label("Vol")));
    m_button_rec_vol->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_rec_vol)
    );
    add_tooltip(m_button_rec_vol, "Select recording volume");
    m_toggle_thru->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(thru_xpm)))
    );
    m_toggle_thru->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::thru_change_callback)
    );
    add_tooltip
    (
        m_toggle_thru,
        "Incoming MIDI data passes thru to sequences MIDI bus and channel."
    );
    m_toggle_play->set_active(m_seq->get_playing());
    m_toggle_record->set_active(m_seq->get_recording());
    m_toggle_thru->set_active(m_seq->get_thru());
    dhbox->pack_end(*m_button_rec_vol, false, false, 4);
    dhbox->pack_end(*m_toggle_q_rec, false, false, 4);
    dhbox->pack_end(*m_toggle_record, false, false, 4);
    dhbox->pack_end(*m_toggle_thru, false, false, 4);
    dhbox->pack_end(*m_toggle_play, false, false, 4);
    dhbox->pack_end(*(manage(new Gtk::VSeparator())), false, false, 4);
    fill_top_bar();
    add(*m_vbox);       // this->add(*m_vbox); /* add table */
    show_all();

    /*
     * OPTION?  Sets scroll bar to the middle:
     *
     * gfloat middle = m_vscroll->get_adjustment()->get_upper() / 3;
     * m_vscroll->get_adjustment()->set_value(middle);
     */

    m_seqroll_wid->set_ignore_redraw(true);
    set_zoom(m_zoom);
    set_snap(m_snap);
    set_note_length(m_note_length);
    set_background_sequence(m_sequence);
    set_bpm(m_seq->get_bpm());
    set_bw(m_seq->get_bw());
    set_measures(get_measures());
    set_midi_channel(m_seq->get_midi_channel());
    set_midi_bus(m_seq->get_midi_bus());
    set_data_type(EVENT_NOTE_ON);
    set_scale(m_scale);
    set_key(m_key);
    m_seqroll_wid->set_ignore_redraw(false);
    add_events(Gdk::SCROLL_MASK);
}

/**
 *  A rote destructor.
 */

seqedit::~seqedit()
{
    // Empty body
}

/**
 *  Creates the various menus by pushing menu elements into the menus.
 *
 */

void
seqedit::create_menus ()
{
    using namespace Gtk::Menu_Helpers;

    /*
     *  This first menu is the Zoom menu, represented in the
     *  pattern/sequence editor by a button with a magnifying glass.
     *  The values are "pixels to ticks".
     */

    char b[20];
    for (int i = mc_min_zoom; i <= mc_max_zoom; i *= 2)           /* zoom */
    {
        snprintf(b, sizeof(b), "1:%d", i);
        m_menu_zoom->items().push_back
        (
            MenuElem(b, sigc::bind(mem_fun(*this, &seqedit::set_zoom), i))
        );
    }

    /*
     *  The Snap menu is actually the Grid Snap button, which shows two
     *  arrows pointing to a central bar.
     */

#define SET_SNAP    mem_fun(*this, &seqedit::set_snap)

    m_menu_snap->items().push_back                              /* note snap */
    (
        MenuElem("1", sigc::bind(SET_SNAP, c_ppqn * 4))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/2", sigc::bind(SET_SNAP, c_ppqn * 2))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/4", sigc::bind(SET_SNAP, c_ppqn))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/8", sigc::bind(SET_SNAP, c_ppqn / 2))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/16", sigc::bind(SET_SNAP, c_ppqn / 4))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/32", sigc::bind(SET_SNAP, c_ppqn / 8))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/64", sigc::bind(SET_SNAP, c_ppqn / 16))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/128", sigc::bind(SET_SNAP, c_ppqn / 32))
    );
    m_menu_snap->items().push_back(SeparatorElem());        /* separator */
    m_menu_snap->items().push_back
    (
        MenuElem("1/3", sigc::bind(SET_SNAP, c_ppqn * 4  / 3))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/6", sigc::bind(SET_SNAP, c_ppqn * 2  / 3))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/12", sigc::bind(SET_SNAP, c_ppqn * 1  / 3))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/24", sigc::bind(SET_SNAP, c_ppqn / 2  / 3))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/48", sigc::bind(SET_SNAP, c_ppqn / 4  / 3))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/96", sigc::bind(SET_SNAP, c_ppqn / 8  / 3))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/192", sigc::bind(SET_SNAP, c_ppqn / 16 / 3))
    );

    /*
     *  The note-length menu is on the button that shows four notes.
     */

#define SET_NOTE    mem_fun(*this, &seqedit::set_note_length)

    m_menu_note_length->items().push_back                   /* note length */
    (
        MenuElem("1", sigc::bind(SET_NOTE, c_ppqn * 4))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/2", sigc::bind(SET_NOTE, c_ppqn * 2))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/4", sigc::bind(SET_NOTE, c_ppqn * 1))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/8", sigc::bind(SET_NOTE, c_ppqn / 2))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/16", sigc::bind(SET_NOTE, c_ppqn / 4))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/32", sigc::bind(SET_NOTE, c_ppqn / 8))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/64", sigc::bind(SET_NOTE, c_ppqn / 16))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/128", sigc::bind(SET_NOTE, c_ppqn / 32))
    );
    m_menu_note_length->items().push_back(SeparatorElem());
    m_menu_note_length->items().push_back
    (
        MenuElem("1/3", sigc::bind(SET_NOTE, c_ppqn * 4  / 3))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/6", sigc::bind(SET_NOTE, c_ppqn * 2  / 3))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/12", sigc::bind(SET_NOTE, c_ppqn * 1  / 3))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/24", sigc::bind(SET_NOTE, c_ppqn / 2  / 3))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/48", sigc::bind(SET_NOTE, c_ppqn / 4  / 3))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/96", sigc::bind(SET_NOTE, c_ppqn / 8  / 3))
    );
    m_menu_note_length->items().push_back
    (
        MenuElem("1/192", sigc::bind(SET_NOTE, c_ppqn / 16 / 3))
    );

    /*
     *  This menu lets one set the key of the sequence, and is brought up
     *  by the button with the "golden key" image on it.
     */

#define SET_KEY     mem_fun(*this, &seqedit::set_key)

    m_menu_key->items().push_back                                   /* Key */
    (
        MenuElem(c_key_text[0], sigc::bind(SET_KEY, 0))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[1], sigc::bind(SET_KEY, 1))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[2], sigc::bind(SET_KEY, 2))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[3], sigc::bind(SET_KEY, 3))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[4], sigc::bind(SET_KEY, 4))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[5], sigc::bind(SET_KEY, 5))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[6], sigc::bind(SET_KEY, 6))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[7], sigc::bind(SET_KEY, 7))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[8], sigc::bind(SET_KEY, 8))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[9], sigc::bind(SET_KEY, 9))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[10], sigc::bind(SET_KEY, 10))
    );
    m_menu_key->items().push_back
    (
        MenuElem(c_key_text[11], sigc::bind(SET_KEY, 11))
    );

    /*
     *  This button shows a down around for the bottom half of the time
     *  signature.  It's tooltip is "Time signature.  Length of beat."
     *  But it is called bw, or beat width, in the code.
     */

#define SET_BW      mem_fun(*this, &seqedit::set_bw)

    m_menu_bw->items().push_back                        /* bw, beat width  */
    (
        MenuElem("1", sigc::bind(SET_BW, 1))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("2", sigc::bind(SET_BW, 2))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("4", sigc::bind(SET_BW, 4))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("8", sigc::bind(SET_BW, 8))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("16", sigc::bind(SET_BW, 16))
    );

    /*
     *  This menu is shown when pressing the button at the bottom of the
     *  window that has "Vol" as its label.  Let's show the numbers as
     *  well to help the user.  And we'll have to document this change.
     */

#define SET_REC_VOL     mem_fun(*this, &seqedit::set_rec_vol)

    m_menu_rec_vol->items().push_back                   /* record volume */
    (
        MenuElem("Free", sigc::bind(SET_REC_VOL, 0))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 8: 127", sigc::bind(SET_REC_VOL, 127))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 7: 112", sigc::bind(SET_REC_VOL, 112))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 6: 96", sigc::bind(SET_REC_VOL, 96))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 5: 80", sigc::bind(SET_REC_VOL, 80))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 4: 64", sigc::bind(SET_REC_VOL, 64))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 3: 48", sigc::bind(SET_REC_VOL, 48))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 2: 32", sigc::bind(SET_REC_VOL, 32))
    );
    m_menu_rec_vol->items().push_back
    (
        MenuElem("Fixed 1: 16", sigc::bind(SET_REC_VOL, 16))
    );

    /*
     *  This menu sets the scale to show on the panel, and the button
     *  shows a "staircase" image.  See the c_music_scales enumeration
     *  defined in the globals module.
     */

#define SET_SCALE   mem_fun(*this, &seqedit::set_scale)

#if 0
    m_menu_scale->items().push_back                     /* music scale */
    (
        MenuElem(c_scales_text[0], sigc::bind(SET_SCALE, c_scale_off))
    );
    m_menu_scale->items().push_back
    (
        MenuElem(c_scales_text[1], sigc::bind(SET_SCALE, c_scale_major))
    );
    m_menu_scale->items().push_back
    (
        MenuElem(c_scales_text[2], sigc::bind(SET_SCALE, c_scale_minor))
    );
#endif

    for (int i = int(c_scale_off); i < int(c_scale_size); i++)
    {
        m_menu_scale->items().push_back                     /* music scale */
        (
            MenuElem(c_scales_text[i], sigc::bind(SET_SCALE, i))
        );
    }

    /*
     *  This section sets up two different menus.  The first is
     *  m_menu_length.  This menu lets on set the sequence length in bars
     *  (not the MIDI channel).  The second menu is the m_menu_bpm, or
     *  BPM, which here means "beats per measure" (not "beats per
     *  minute").
     */

#define SET_BPM         mem_fun(*this, &seqedit::set_bpm)
#define SET_MEASURES    mem_fun(*this, &seqedit::set_measures)

    for (int i = 0; i < 16; i++)                        /* seq length menu   */
    {
        snprintf(b, sizeof(b), "%d", i + 1);
        m_menu_length->items().push_back                /* length            */
        (
            MenuElem(b, sigc::bind(SET_MEASURES, i + 1))
        );
        m_menu_bpm->items().push_back                   /* beats per measure */
        (
            MenuElem(b, sigc::bind(SET_BPM, i + 1))
        );
    }
    m_menu_length->items().push_back
    (
        MenuElem("32", sigc::bind(SET_MEASURES, 32))
    );
    m_menu_length->items().push_back
    (
        MenuElem("64", sigc::bind(SET_MEASURES, 64))
    );
}

/**
 *  Sets up the pop-up menus that are brought up by pressing the Tools
 *  button, which shows a hammer image.  This button shows three sub-menus
 *  that need to be filled in by this function.  All the functions
 *  accessed here seem to be implemented by the do_action() function.
 */

void
seqedit::popup_tool_menu ()
{
    using namespace Gtk::Menu_Helpers;

    m_menu_tools = manage(new Gtk::Menu());

    Gtk::Menu * holder;
    Gtk::Menu * holder2;
    holder = manage(new Gtk::Menu());

#define DO_ACTION       mem_fun(*this, &seqedit::do_action)

    holder->items().push_back
    (
        MenuElem("All Notes", sigc::bind(DO_ACTION, c_select_all_notes, 0))
    );
    holder->items().push_back
    (
        MenuElem("Inverse Notes", sigc::bind(DO_ACTION, c_select_inverse_notes, 0))
    );

    /*
     * This is an interesting wrinkle to document.
     */

    if (m_editing_status != EVENT_NOTE_ON && m_editing_status != EVENT_NOTE_OFF)
    {
        holder->items().push_back(SeparatorElem());
        holder->items().push_back
        (
            MenuElem("All Events", sigc::bind(DO_ACTION, c_select_all_events, 0))
        );
        holder->items().push_back
        (
            MenuElem("Inverse Events",
                sigc::bind(DO_ACTION, c_select_inverse_events, 0))
        );
    }

    m_menu_tools->items().push_back(MenuElem("Select", *holder));
    holder = manage(new Gtk::Menu());
    holder->items().push_back
    (
        MenuElem("Quantize Selected Notes",
            sigc::bind(DO_ACTION, c_quantize_notes, 0))
    );
    holder->items().push_back
    (
        MenuElem("Tighten Selected Notes",
            sigc::bind(DO_ACTION, c_tighten_notes, 0))
    );

    if (m_editing_status != EVENT_NOTE_ON && m_editing_status != EVENT_NOTE_OFF)
    {
        /*
         *  The action code here is c_quantize_events, not
         *  c_quantize_notes.
         */

        holder->items().push_back(SeparatorElem());
        holder->items().push_back
        (
            MenuElem("Quantize Selected Events",
                sigc::bind(DO_ACTION, c_quantize_events, 0))
        );
        holder->items().push_back
        (
            MenuElem("Tighten Selected Events",
                sigc::bind(DO_ACTION, c_tighten_events, 0))
        );
    }
    m_menu_tools->items().push_back(MenuElem("Modify Time", *holder));
    holder = manage(new Gtk::Menu());

    char num[16];
    holder2 = manage(new Gtk::Menu());
    for (int i = -12; i <= 12; ++i)
    {
        if (i != 0)
        {
            snprintf(num, sizeof(num), "%+d [%s]", i, c_interval_text[abs(i)]);
            holder2->items().push_front
            (
                MenuElem(num, sigc::bind(DO_ACTION, c_transpose, i))
            );
        }
    }
    holder->items().push_back(MenuElem("Transpose Selected", *holder2));
    holder2 = manage(new Gtk::Menu());
    for (int i = -7; i <= 7; ++i)
    {
        if (i != 0)
        {
            snprintf
            (
                num, sizeof(num), "%+d [%s]",
                (i < 0) ? i-1 : i+1, c_chord_text[abs(i)]
            );
            holder2->items().push_front
            (
                MenuElem(num, sigc::bind(DO_ACTION, c_transpose_h, i))
            );
        }
    }
    if (m_scale != 0)
    {
        holder->items().push_back
        (
            MenuElem("Harmonic Transpose Selected", *holder2)
        );
    }
    m_menu_tools->items().push_back(MenuElem("Modify Pitch", *holder));
    m_menu_tools->popup(0, 0);
}

/**
 *  Implements the actions brought forth from the Tools (hammer) button.
 */

void
seqedit::do_action (int a_action, int a_var)
{
    switch (a_action)
    {
    case c_select_all_notes:
        m_seq->select_events(EVENT_NOTE_ON, 0);
        m_seq->select_events(EVENT_NOTE_OFF, 0);
        break;

    case c_select_all_events:
        m_seq->select_events(m_editing_status, m_editing_cc);
        break;

    case c_select_inverse_notes:
        m_seq->select_events(EVENT_NOTE_ON, 0, true);
        m_seq->select_events(EVENT_NOTE_OFF, 0, true);
        break;

    case c_select_inverse_events:
        m_seq->select_events(m_editing_status, m_editing_cc, true);
        break;

    case c_quantize_notes:
        m_seq->push_undo();
        m_seq->quantize_events(EVENT_NOTE_ON, 0, m_snap, 1 , true);
        break;

    case c_quantize_events:
        m_seq->push_undo();
        m_seq->quantize_events(m_editing_status, m_editing_cc, m_snap, 1);
        break;

    case c_tighten_notes:
        m_seq->push_undo();
        m_seq->quantize_events(EVENT_NOTE_ON, 0, m_snap, 2 , true);
        break;

    case c_tighten_events:
        m_seq->push_undo();
        m_seq->quantize_events(m_editing_status, m_editing_cc, m_snap, 2);
        break;

    case c_transpose:
        m_seq->push_undo();
        m_seq->transpose_notes(a_var, 0);
        break;

    case c_transpose_h:
        m_seq->push_undo();
        m_seq->transpose_notes(a_var, m_scale);
        break;

    default:
        break;
    }
    m_seqroll_wid->redraw();
    m_seqtime_wid->redraw();
    m_seqdata_wid->redraw();
    m_seqevent_wid->redraw();
}

/**
 *  This function inserts the user-interface items into the top bar or
 *  panel of the pattern editor; this bar has two rows of user interface
 *  elements.
 */

void
seqedit::fill_top_bar ()
{
    /*
     *  First row of top bar
     */

    m_entry_name = manage(new Gtk::Entry());            /* name              */
    m_entry_name->set_max_length(26);
    m_entry_name->set_width_chars(26);
    m_entry_name->set_text(m_seq->get_name());
    m_entry_name->select_region(0, 0);
    m_entry_name->set_position(0);
    m_entry_name->signal_changed().connect
    (
        mem_fun(*this, &seqedit::name_change_callback)
    );
    m_hbox->pack_start(*m_entry_name, true, true);
    m_hbox->pack_start(*(manage(new Gtk::VSeparator())), false, false, 4);

    m_button_bpm = manage(new Gtk::Button());           /* beats per measure */
    m_button_bpm->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(down_xpm)))
    );
    m_button_bpm->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_bpm)
    );
    add_tooltip(m_button_bpm, "Time Signature. Beats per Measure");
    m_entry_bpm = manage(new Gtk::Entry());
    m_entry_bpm->set_width_chars(2);
    m_entry_bpm->set_editable(false);
    m_hbox->pack_start(*m_button_bpm , false, false);
    m_hbox->pack_start(*m_entry_bpm , false, false);
    m_hbox->pack_start(*(manage(new Gtk::Label("/"))), false, false, 4);
    m_button_bw = manage(new Gtk::Button());            /* beat width        */
    m_button_bw->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(down_xpm)))
    );
    m_button_bw->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_bw)
    );
    add_tooltip(m_button_bw, "Time Signature. Length of Beat");
    m_entry_bw = manage(new Gtk::Entry());
    m_entry_bw->set_width_chars(2);
    m_entry_bw->set_editable(false);
    m_hbox->pack_start(*m_button_bw , false, false);
    m_hbox->pack_start(*m_entry_bw , false, false);
    m_button_length = manage(new Gtk::Button());        /* length of pattern */
    m_button_length->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(length_xpm)))
    );
    m_button_length->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_length)
    );
    add_tooltip(m_button_length, "Sequence length in Bars.");
    m_entry_length = manage(new Gtk::Entry());
    m_entry_length->set_width_chars(2);
    m_entry_length->set_editable(false);
    m_hbox->pack_start(*m_button_length , false, false);
    m_hbox->pack_start(*m_entry_length , false, false);
    m_hbox->pack_start(*(manage(new Gtk::VSeparator())), false, false, 4);
    m_button_bus = manage(new Gtk::Button());           /* MIDI output bus   */
    m_button_bus->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(bus_xpm)))
    );
    m_button_bus->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::popup_midibus_menu)
    );
    add_tooltip(m_button_bus, "Select Output Bus.");
    m_entry_bus = manage(new Gtk::Entry());
    m_entry_bus->set_max_length(60);
    m_entry_bus->set_width_chars(60);
    m_entry_bus->set_editable(false);
    m_hbox->pack_start(*m_button_bus , false, false);
    m_hbox->pack_start(*m_entry_bus , true, true);
    m_button_channel = manage(new Gtk::Button());       /* MIDI channel      */
    m_button_channel->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(midi_xpm)))
    );
    m_button_channel->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::popup_midich_menu)
    );
    add_tooltip(m_button_channel, "Select Midi channel.");
    m_entry_channel = manage(new Gtk::Entry());
    m_entry_channel->set_width_chars(2);
    m_entry_channel->set_editable(false);
    m_hbox->pack_start(*m_button_channel , false, false);
    m_hbox->pack_start(*m_entry_channel , false, false);

    /*
     *  Second row of top bar
     */

    m_button_undo = manage(new Gtk::Button());              /* undo          */
    m_button_undo->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(undo_xpm)))
    );
    m_button_undo->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::undo_callback)
    );
    add_tooltip(m_button_undo, "Undo.");
    m_hbox2->pack_start(*m_button_undo , false, false);
    m_button_redo = manage(new Gtk::Button());              /* redo          */
    m_button_redo->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(redo_xpm)))
    );
    m_button_redo->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::redo_callback)
    );
    add_tooltip(m_button_redo, "Redo.");
    m_hbox2->pack_start(*m_button_redo , false, false);

    /*
     * Quantize shortcut.  This is the "Q" button, and indicates to
     * quantize (just?) notes.  Compare it to the Quantize menu entry,
     * which quantizes events.
     */

    m_button_quantize = manage(new Gtk::Button());          /* Quantize      */
    m_button_quantize->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(quantize_xpm)))
    );
    m_button_quantize->signal_clicked().connect
    (
        sigc::bind(mem_fun(*this, &seqedit::do_action), c_quantize_notes, 0)
    );
    add_tooltip(m_button_quantize, "Quantize Selection.");
    m_hbox2->pack_start(*m_button_quantize , false, false);
    m_hbox2->pack_start(*(manage(new Gtk::VSeparator())), false, false, 4);
    m_button_tools = manage(new Gtk::Button());             /* tools button  */
    m_button_tools->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(tools_xpm)))
    );
    m_button_tools->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::popup_tool_menu)
    );
    m_tooltips->set_tip(*m_button_tools, "Tools.");
    m_hbox2->pack_start(*m_button_tools , false, false);
    m_hbox2->pack_start(*(manage(new Gtk::VSeparator())), false, false, 4);
    m_button_snap = manage(new Gtk::Button());              /* snap          */
    m_button_snap->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(snap_xpm)))
    );
    m_button_snap->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_snap)
    );
    add_tooltip(m_button_snap, "Grid snap.");
    m_entry_snap = manage(new Gtk::Entry());
    m_entry_snap->set_width_chars(5);
    m_entry_snap->set_editable(false);
    m_hbox2->pack_start(*m_button_snap , false, false);
    m_hbox2->pack_start(*m_entry_snap , false, false);
    m_button_note_length = manage(new Gtk::Button());       /* note_length   */
    m_button_note_length->add
    (
        *manage
        (
            new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(note_length_xpm))
        )
    );
    m_button_note_length->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu),
            m_menu_note_length)
    );
    add_tooltip(m_button_note_length, "Note Length.");
    m_entry_note_length = manage(new Gtk::Entry());
    m_entry_note_length->set_width_chars(5);
    m_entry_note_length->set_editable(false);
    m_hbox2->pack_start(*m_button_note_length , false, false);
    m_hbox2->pack_start(*m_entry_note_length , false, false);
    m_button_zoom = manage(new Gtk::Button());              /* zoom pixels   */
    m_button_zoom->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(zoom_xpm)))
    );
    m_button_zoom->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_zoom)
    );
    add_tooltip(m_button_zoom, "Zoom. Pixels to Ticks");
    m_entry_zoom = manage(new Gtk::Entry());
    m_entry_zoom->set_width_chars(4);
    m_entry_zoom->set_editable(false);
    m_hbox2->pack_start(*m_button_zoom , false, false);
    m_hbox2->pack_start(*m_entry_zoom , false, false);
    m_hbox2->pack_start(*(manage(new Gtk::VSeparator())), false, false, 4);
    m_button_key = manage(new Gtk::Button());               /* musical key   */
    m_button_key->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(key_xpm)))
    );
    m_button_key->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_key)
    );
    add_tooltip(m_button_key, "Key of Sequence");
    m_entry_key = manage(new Gtk::Entry());
    m_entry_key->set_width_chars(5);
    m_entry_key->set_editable(false);
    m_hbox2->pack_start(*m_button_key , false, false);
    m_hbox2->pack_start(*m_entry_key , false, false);
    m_button_scale = manage(new Gtk::Button());             /* musical scale */
    m_button_scale->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(scale_xpm)))
    );
    m_button_scale->signal_clicked().connect
    (
        sigc::bind<Gtk::Menu *>(mem_fun(*this, &seqedit::popup_menu), m_menu_scale)
    );
    add_tooltip(m_button_scale, "Musical Scale");
    m_entry_scale = manage(new Gtk::Entry());
    m_entry_scale->set_width_chars(5);
    m_entry_scale->set_editable(false);
    m_hbox2->pack_start(*m_button_scale , false, false);
    m_hbox2->pack_start(*m_entry_scale , false, false);
    m_hbox2->pack_start(*(manage(new Gtk::VSeparator())), false, false, 4);
    m_button_sequence = manage(new Gtk::Button());      /* background sequence */
    m_button_sequence->add
    (
        *manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(sequences_xpm)))
    );
    m_button_sequence->signal_clicked().connect
    (
        mem_fun(*this, &seqedit::popup_sequence_menu)
    );
    add_tooltip(m_button_sequence, "Background Sequence");
    m_entry_sequence = manage(new Gtk::Entry());
    m_entry_sequence->set_width_chars(14);
    m_entry_sequence->set_editable(false);
    m_hbox2->pack_start(*m_button_sequence , false, false);
    m_hbox2->pack_start(*m_entry_sequence , true, true);

    /*
     * The following commented buttons were a visual way to select the
     * modes of note editing.  These can easily be done with the left
     * mouse button and some other tricks, though.
     */

#if 0

    m_radio_select = manage(new Gtk::RadioButton("Sel", true)); /* Select */
    m_radio_select->signal_clicked().connect
    (
        sigc::bind(mem_fun(*this, &seqedit::mouse_action), e_action_select)
    );
    m_hbox3->pack_start(*m_radio_select, false, false);

    m_radio_draw = manage(new Gtk::RadioButton("Draw"));        /* Draw */
    m_radio_draw->signal_clicked().connect
    (
        sigc::bind(mem_fun(*this, &seqedit::mouse_action), e_action_draw)
    );
    m_hbox3->pack_start(*m_radio_draw, false, false);

    m_radio_grow = manage(new Gtk::RadioButton("Grow"));        /* Grow */
    m_radio_grow->signal_clicked().connect
    (
        sigc::bind(mem_fun(*this, &seqedit::mouse_action), e_action_grow)
    );
    m_hbox3->pack_start(*m_radio_grow, false, false);

    /* Stretch */

    Gtk::RadioButton::Group g = m_radio_select->get_group();
    m_radio_draw->set_group(g);
    m_radio_grow->set_group(g);

#endif
}

    /*
     * The following commented function seems to be legacy test code.
     */

#if 0

void
seqedit::mouse_action (mouse_action_e a_action)
{
    if (a_action == e_action_select && m_radio_select->get_active())
        printf("mouse_action() select [%d]\n", a_action);

    if (a_action == e_action_draw && m_radio_draw->get_active())
        printf("mouse_action() draw [%d]\n", a_action);

    if (a_action == e_action_grow && m_radio_grow->get_active())
        printf("mouse_action() grow [%d]\n", a_action);
}

#endif

/**
 *  Pops up the given pop-up menu.
 */

void
seqedit::popup_menu (Gtk::Menu * a_menu)
{
    a_menu->popup(0, 0);
}

/**
 *  Populates the MIDI Output buss pop-up menu.  The MIDI busses are
 *  obtained by getting the mastermidibus object, and iterating through
 *  the busses that it contains.
 */

void
seqedit::popup_midibus_menu ()
{
    using namespace Gtk::Menu_Helpers;

    m_menu_midibus = manage(new Gtk::Menu());
    mastermidibus * masterbus = m_mainperf->get_master_midi_bus();

#define SET_BUS         mem_fun(*this, &seqedit::set_midi_bus)

    for (int i = 0; i < masterbus->get_num_out_buses(); i++)
    {
        m_menu_midibus->items().push_back
        (
            MenuElem(masterbus->get_midi_out_bus_name(i), sigc::bind(SET_BUS, i))
        );
    }
    m_menu_midibus->popup(0, 0);
}

/**
 *  Populates the MIDI Channel pop-up menu.
 */

void
seqedit::popup_midich_menu ()
{
    using namespace Gtk::Menu_Helpers;

    m_menu_midich = manage(new Gtk::Menu());
    int midi_bus = m_seq->get_midi_bus();
    char b[16];
    for (int i = 0; i < 16; i++)                    /* midi channel menu */
    {
        snprintf(b, sizeof(b), "%d", i + 1);
        std::string name = std::string(b);
        int instrument = global_user_midi_bus_definitions[midi_bus].instrument[i];
        if (instrument >= 0 && instrument < c_max_busses)
        {
            name = name +
            (
                std::string(" (") +
                    global_user_instrument_definitions[instrument].instrument +
                    std::string(")")
            );
        }

#define SET_CH         mem_fun(*this, &seqedit::set_midi_channel)

        m_menu_midich->items().push_back(MenuElem(name, sigc::bind(SET_CH, i)));
    }
    m_menu_midich->popup(0, 0);
}

/**
 *  Populates the "set background sequence" menu (drops from the button
 *  that has some note-bars on it at the right of the second row of the
 *  top bar).  It is populated with an "Off" menu entry, and a second
 *  "[0]" menu entry that pulls up a drop-down menu of all of the
 *  patterns/sequences that are present in the MIDI file.
 */

void
seqedit::popup_sequence_menu ()
{
    using namespace Gtk::Menu_Helpers;

    m_menu_sequences = manage(new Gtk::Menu());

#define SET_BG_SEQ     mem_fun(*this, &seqedit::set_background_sequence)

    m_menu_sequences->items().push_back
    (
        MenuElem("Off", sigc::bind(SET_BG_SEQ, -1))
    );
    m_menu_sequences->items().push_back(SeparatorElem());
    for (int ss = 0; ss < c_max_sets; ++ss)
    {
        Gtk::Menu * menu_ss = nullptr;
        bool inserted = false;
        for (int seq = 0; seq <  c_seqs_in_set; seq++)
        {
            char name[30];
            int i = ss * c_seqs_in_set + seq;
            if (m_mainperf->is_active(i))
            {
                if (! inserted)
                {
                    inserted = true;
                    snprintf(name, sizeof(name), "[%d]", ss);
                    menu_ss = manage(new Gtk::Menu());
                    m_menu_sequences->items().push_back
                    (
                        MenuElem(name, *menu_ss)
                    );
                }
                sequence *seq = m_mainperf->get_sequence(i);
                snprintf(name, sizeof(name), "[%d] %.13s", i, seq->get_name());
                menu_ss->items().push_back
                (
                    MenuElem(name, sigc::bind(SET_BG_SEQ, i))
                );
            }
        }
    }
    m_menu_sequences->popup(0, 0);
}

/**
 *  Draws the given background sequence on the Pattern editor so that the
 *  musician has something to see that can be played against.
 */

void
seqedit::set_background_sequence (int a_seq)
{
    char name[30];
    m_initial_sequence = m_sequence = a_seq;
    if (a_seq == -1 || !m_mainperf->is_active(a_seq))
    {
        m_entry_sequence->set_text("Off");
        m_seqroll_wid->set_background_sequence(false, 0);
    }
    if (m_mainperf->is_active(a_seq))
    {
        sequence *seq = m_mainperf->get_sequence(a_seq);
        snprintf(name, sizeof(name), "[%d] %.13s", a_seq, seq->get_name());
        m_entry_sequence->set_text(name);
        m_seqroll_wid->set_background_sequence(true, a_seq);
    }
}

/**
 *  Sets the manu pixmap depending on the given state, where true is a
 *  full menu (black backgroun), and empty menu (gray background).
 */

Gtk::Image *
seqedit::create_menu_image (bool a_state)
{
    if (a_state)
    {
        return manage
        (
            new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(menu_full_xpm))
        );
    }
    else
    {
        return manage
        (
            new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(menu_empty_xpm))
        );
    }
}

/**
 *  Populates the event-selection menu that drops from the "Event" button
 *  in the bottom row of the Pattern editor.
 *
 *  This menu has a large number of items.  I think they are filled in in
 *  code, but can also be loaded from ~/.seq24usr.  To be determined.
 */

void
seqedit::popup_event_menu ()
{
    using namespace Gtk::Menu_Helpers;

    bool ccs[MIDI_COUNT_MAX];
    char b[20];
    bool note_on = false;
    bool note_off = false;
    bool aftertouch = false;
    bool program_change = false;
    bool channel_pressure = false;
    bool pitch_wheel = false;
    unsigned char status, cc;
    int midi_bus = m_seq->get_midi_bus();
    int midi_ch = m_seq->get_midi_channel();
    memset(ccs, false, sizeof(bool) * MIDI_COUNT_MAX);
    m_seq->reset_draw_marker();
    while (m_seq->get_next_event(&status, &cc))
    {
        switch (status)
        {
        case EVENT_NOTE_OFF:
            note_off = true;
            break;

        case EVENT_NOTE_ON:
            note_on = true;
            break;

        case EVENT_AFTERTOUCH:
            aftertouch = true;
            break;

        case EVENT_CONTROL_CHANGE:
            ccs[cc] = true;
            break;

        case EVENT_PITCH_WHEEL:
            pitch_wheel = true;
            break;

        /* one data item */

        case EVENT_PROGRAM_CHANGE:
            program_change = true;
            break;

        case EVENT_CHANNEL_PRESSURE:
            channel_pressure = true;
            break;
        }
    }

    m_menu_data = manage(new Gtk::Menu());
    m_menu_data->items().push_back
    (
        ImageMenuElem
        (
            "Note On Velocity", *create_menu_image(note_on),
            sigc::bind
            (
                mem_fun(*this, &seqedit::set_data_type),
               (unsigned char) EVENT_NOTE_ON, 0
            )
        )
    );
    m_menu_data->items().push_back(SeparatorElem());
    m_menu_data->items().push_back
    (
        ImageMenuElem
        (
            "Note Off Velocity", *create_menu_image(note_off),
           sigc::bind
           (
                mem_fun(*this, &seqedit::set_data_type),
               (unsigned char) EVENT_NOTE_OFF, 0
            )
        )
    );
    m_menu_data->items().push_back
    (
        ImageMenuElem
        (
            "AfterTouch", *create_menu_image(aftertouch),
            sigc::bind
            (
                mem_fun(*this, &seqedit::set_data_type),
                (unsigned char) EVENT_AFTERTOUCH, 0
            )
        )
    );
    m_menu_data->items().push_back
    (
        ImageMenuElem
        (
            "Program Change", *create_menu_image(program_change),
            sigc::bind
            (
                mem_fun(*this, &seqedit::set_data_type),
                (unsigned char) EVENT_PROGRAM_CHANGE, 0
            )
        )
    );
    m_menu_data->items().push_back
    (
        ImageMenuElem
        (
            "Channel Pressure", *create_menu_image(channel_pressure),
            sigc::bind
            (
                mem_fun(*this, &seqedit::set_data_type),
               (unsigned char) EVENT_CHANNEL_PRESSURE, 0
            )
        )
    );
    m_menu_data->items().push_back
    (
        ImageMenuElem
        (
            "Pitch Wheel", *create_menu_image(pitch_wheel),
            sigc::bind
            (
                mem_fun(*this, &seqedit::set_data_type),
               (unsigned char) EVENT_PITCH_WHEEL , 0
            )
        )
    );
    m_menu_data->items().push_back(SeparatorElem());

    /**
     *  Create the 8 sub-menus for the various ranges of controller
     *  changes.
     */

    for (int i = 0; i < 8; i++)                 /* create control change */
    {
        snprintf(b, sizeof(b), "Controls %d-%d", (i * 16), (i * 16) + 15);
        Gtk::Menu * menu_cc = manage(new Gtk::Menu());
        for (int j = 0; j < 16; j++)
        {
            std::string controller_name(c_controller_names[i * 16 + j]);
            int instrument = global_user_midi_bus_definitions[midi_bus].
                instrument[midi_ch];

            if (instrument > -1 && instrument < c_max_instruments)
            {
                if
                (
                    global_user_instrument_definitions[instrument].
                        controllers_active[i * 16 + j]
                )
                {
                    controller_name =
                        global_user_instrument_definitions[instrument].
                            controllers[i * 16 + j];
                }
            }
            menu_cc->items().push_back
            (
                ImageMenuElem
                (
                    controller_name, *create_menu_image(ccs[i * 16 + j]),
                    sigc::bind
                    (
                        mem_fun(*this, &seqedit::set_data_type),
                        (unsigned char) EVENT_CONTROL_CHANGE, i * 16 + j
                    )
                )
            );
        }
        m_menu_data->items().push_back(MenuElem(std::string(b), *menu_cc));
    }
    m_menu_data->popup(0, 0);
}

/**
 *  Selects the given MIDI channel parameter in the main sequence object,
 *  so that it will use that channel.
 */

void
seqedit::set_midi_channel (int a_midichannel)
{
    char b[10];
    snprintf(b, sizeof(b), "%d", a_midichannel + 1);
    m_entry_channel->set_text(b);
    m_seq->set_midi_channel(a_midichannel);
}

/**
 *  Selects the given MIDI buss parameter in the main sequence object,
 *  so that it will use that buss.
 */

void
seqedit::set_midi_bus (int a_midibus)
{
    m_seq->set_midi_bus(a_midibus);
    mastermidibus * mmb =  m_mainperf->get_master_midi_bus();
    m_entry_bus->set_text(mmb->get_midi_out_bus_name(a_midibus));
}

/**
 *  Selects the given zoom value.  It is passed to the seqroll, seqtime,
 *  seqdata, and seqevent objects, as well.
 */

void
seqedit::set_zoom (int a_zoom)
{
    char b[10];
    snprintf(b, sizeof(b), "1:%d", a_zoom);
    m_entry_zoom->set_text(b);
    m_zoom = a_zoom;
    m_initial_zoom = a_zoom;
    m_seqroll_wid->set_zoom(m_zoom);
    m_seqtime_wid->set_zoom(m_zoom);
    m_seqdata_wid->set_zoom(m_zoom);
    m_seqevent_wid->set_zoom(m_zoom);
}

/**
 *  Selects the given snap value.  It is passed to the seqroll, seqevent,
 *  and sequence objects, as well.
 */

void
seqedit::set_snap (int a_snap)
{
    char b[10];
    snprintf(b, sizeof(b), "1/%d",   c_ppqn * 4 / a_snap);
    m_entry_snap->set_text(b);
    m_snap = a_snap;
    m_initial_snap = a_snap;
    m_seqroll_wid->set_snap(m_snap);
    m_seqevent_wid->set_snap(m_snap);
    m_seq->set_snap_tick(a_snap);
}

/**
 *  Selects the given note-length value.  It is passed to the seqroll
 *  object, as well.
 */

void
seqedit::set_note_length (int a_note_length)
{
    char b[10];
    snprintf(b, sizeof(b), "1/%d",   c_ppqn * 4 / a_note_length);
    m_entry_note_length->set_text(b);
    m_note_length = a_note_length;
    m_initial_note_length = a_note_length;
    m_seqroll_wid->set_note_length(m_note_length);
}

/**
 *  Selects the given scale value.  It is passed to the seqroll and
 *  seqkeys objects, as well.
 */

void
seqedit::set_scale (int a_scale)
{
    m_entry_scale->set_text(c_scales_text[a_scale]);
    m_scale = m_initial_scale = a_scale;
    m_seqroll_wid->set_scale(m_scale);
    m_seqkeys_wid->set_scale(m_scale);
}

/**
 *  Selects the given key (signature) value.  It is passed to the seqroll
 *  and seqkeys objects, as well.
 */

void
seqedit::set_key(int a_note)
{
    m_entry_key->set_text(c_key_text[a_note]);
    m_key = m_initial_key = a_note;
    m_seqroll_wid->set_key(m_key);
    m_seqkeys_wid->set_key(m_key);
}

/**
 *  Sets the length based on the three given parameters.  Then the
 *  seqroll, seqtime, seqdata, and seqevent objects are reset().
 */

void
seqedit::apply_length (int a_bpm, int a_bw, int a_measures)
{
    m_seq->set_length(a_measures * a_bpm * ((c_ppqn * 4) / a_bw));

    m_seqroll_wid->reset();
    m_seqtime_wid->reset();
    m_seqdata_wid->reset();
    m_seqevent_wid->reset();
}

/**
 *  Calculates the measures value based on the bpm (beats per measure),
 *  ppqn (parts per quarter note), and bw (beat width) values, and returns
 *  the resultant measures value.
 */

long
seqedit::get_measures ()
{
    long units = ((m_seq->get_bpm() * (c_ppqn * 4)) /  m_seq->get_bw());
    long measures = (m_seq->get_length() / units);
    if (m_seq->get_length() % units != 0)
        measures++;

    return measures;
}

/**
 *  Set the measures value, using the given parameter, and some internal
 *  values passed to apply_length().
 */

void
seqedit::set_measures (int a_length_measures)
{
    char b[10];
    snprintf(b, sizeof(b), "%d", a_length_measures);
    m_entry_length->set_text(b);
    m_measures = a_length_measures;
    apply_length(m_seq->get_bpm(), m_seq->get_bw(), a_length_measures);
}

/**
 *  Set the bpm (beats per measure) value, using the given parameter, and
 *  some internal values passed to apply_length().
 */

void
seqedit::set_bpm (int a_beats_per_measure)
{
    char b[4];
    snprintf(b, sizeof(b), "%d", a_beats_per_measure);
    m_entry_bpm->set_text(b);
    if (a_beats_per_measure != m_seq->get_bpm())
    {
        long length = get_measures();
        m_seq->set_bpm(a_beats_per_measure);
        apply_length(a_beats_per_measure, m_seq->get_bw(), length);
    }
}

/**
 *  Set the bw (beat width) value, using the given parameter, and
 *  some internal values passed to apply_length().
 */

void
seqedit::set_bw (int a_beat_width)
{
    char b[4];
    snprintf(b, sizeof(b), "%d", a_beat_width);
    m_entry_bw->set_text(b);
    if (a_beat_width != m_seq->get_bw())
    {
        long length = get_measures();
        m_seq->set_bw(a_beat_width);
        apply_length(m_seq->get_bpm(), a_beat_width, length);
    }
}

/**
 *  Passes the given parameter to sequence::set_rec_vol().
 */

void
seqedit::set_rec_vol (int a_rec_vol)
{
    m_seq->set_rec_vol(a_rec_vol);
}

/**
 *  Set the name for the main sequence to this object's entry name.
 *  That name is the name the user has given to the sequence being edited.
 */

void
seqedit::name_change_callback ()
{
    m_seq->set_name(m_entry_name->get_text());
}

/**
 *  Passes the play status to the sequence object.
 */

void
seqedit::play_change_callback ()
{
    m_seq->set_playing(m_toggle_play->get_active());
}

/**
 *  Passes the recording status to the sequence object.
 */

void
seqedit::record_change_callback ()
{
    m_mainperf->get_master_midi_bus()->set_sequence_input(true, m_seq);
    m_seq->set_recording(m_toggle_record->get_active());
}

/**
 *  Passes the quantized-recording status to the sequence object.
 */

void
seqedit::q_rec_change_callback ()
{
    m_seq->set_quantized_rec(m_toggle_q_rec->get_active());
}

/**
 *  Pops an undo operation from the sequence object, and then tell the
 *  segroll, seqtime, seqdata, and seqevent objects to redraw.
 */

void
seqedit::undo_callback ()
{
    m_seq->pop_undo();
    m_seqroll_wid->redraw();
    m_seqtime_wid->redraw();
    m_seqdata_wid->redraw();
    m_seqevent_wid->redraw();
}

/**
 *  Pops a redo operation from the sequence object, and then tell the
 *  segroll, seqtime, seqdata, and seqevent objects to redraw.
 */

void
seqedit::redo_callback ()
{
    m_seq->pop_redo();
    m_seqroll_wid->redraw();
    m_seqtime_wid->redraw();
    m_seqdata_wid->redraw();
    m_seqevent_wid->redraw();
}

/**
 *  Passes the MIDI Thru status to the sequence object.
 */

void
seqedit::thru_change_callback ()
{
    m_mainperf->get_master_midi_bus()->set_sequence_input(true, m_seq);
    m_seq->set_thru(m_toggle_thru->get_active());
}

/**
 *  Sets the data type based on the given parameters.
 *  To be determined.
 */

void
seqedit::set_data_type (unsigned char a_status, unsigned char a_control)
{
    m_editing_status = a_status;
    m_editing_cc = a_control;
    m_seqevent_wid->set_data_type(a_status, a_control);
    m_seqdata_wid->set_data_type(a_status, a_control);
    m_seqroll_wid->set_data_type(a_status, a_control);

    char text[100];
    char hex[20];
    char type[80];
    snprintf(hex, sizeof(hex), "[0x%02X]", a_status);
    if (a_status == EVENT_NOTE_OFF)
        snprintf(type, sizeof(type), "Note Off");
    else if (a_status == EVENT_NOTE_ON)
        snprintf(type, sizeof(type), "Note On");
    else if (a_status == EVENT_AFTERTOUCH)
        snprintf(type, sizeof(type), "Aftertouch");
    else if (a_status == EVENT_CONTROL_CHANGE)
    {
        int midi_bus = m_seq->get_midi_bus();
        int midi_ch = m_seq->get_midi_channel();
        std::string controller_name(c_controller_names[a_control]);
        int instrument =
            global_user_midi_bus_definitions[midi_bus].instrument[midi_ch];

        if (instrument > -1 && instrument < c_max_instruments)
        {
            if
            (
                global_user_instrument_definitions[instrument].
                    controllers_active[a_control]
            )
            {
                controller_name = global_user_instrument_definitions[instrument].
                    controllers[a_control];
            }
        }
        snprintf
        (
            type, sizeof(type), "Control Change - %s", controller_name.c_str()
        );
    }
    else if (a_status == EVENT_PROGRAM_CHANGE)
        snprintf(type, sizeof(type), "Program Change");
    else if (a_status == EVENT_CHANNEL_PRESSURE)
        snprintf(type, sizeof(type), "Channel Pressure");
    else if (a_status == EVENT_PITCH_WHEEL)
        snprintf(type, sizeof(type), "Pitch Wheel");
    else
        snprintf(type, sizeof(type), "Unknown MIDI Event");

    snprintf(text, sizeof(text), "%s %s", hex, type);
    m_entry_data->set_text(text);
}

/**
 *  Update the window after a time out, based on dirtiness and on playback
 *  progress.
 */

bool
seqedit::timeout ()
{
    if (m_seq->get_raise())
    {
        m_seq->set_raise(false);
        raise();
    }
    if (m_seq->is_dirty_edit())
    {
        m_seqroll_wid->redraw_events();
        m_seqevent_wid->redraw();
        m_seqdata_wid->redraw();
    }
    m_seqroll_wid->draw_progress_on_window();
    return true;
}

/**
 *  On realization, calls the base-class version, and connects the redraw
 *  timeout signal, timed at c_redraw_ms.
 */

void
seqedit::on_realize()
{
    Gtk::Window::on_realize();
    Glib::signal_timeout().connect
    (
        mem_fun(*this, &seqedit::timeout), c_redraw_ms
    );
}

/**
 *  Handles an on-delete event.  It tells the sequence to stop recording,
 *  tells the perform object's mastermidibus to stop processing input,
 *  and sets the sequence object's editing flag to false.
 *
 * \warning
 *      This function also calls "delete this"!
 *
 * \return
 *      Always returns false.
 */

bool
seqedit::on_delete_event (GdkEventAny * a_event)
{
    m_seq->set_recording(false);
    m_mainperf->get_master_midi_bus()->set_sequence_input(false, NULL);
    m_seq->set_editing(false);
    delete this;
    return false;
}

/**
 *  Handles an on-scroll event.
 */

bool
seqedit::on_scroll_event (GdkEventScroll * a_ev)
{
    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask();
    if ((a_ev->state & modifiers) == GDK_CONTROL_MASK)
    {
        if (a_ev->direction == GDK_SCROLL_DOWN)
        {
            if (m_zoom * 2 <= mc_max_zoom)
                set_zoom(m_zoom * 2);
        }
        else if (a_ev->direction == GDK_SCROLL_UP)
        {
            if (m_zoom / 2 >= mc_min_zoom)
                set_zoom(m_zoom / 2);
        }
        return true;
    }
    else if ((a_ev->state & modifiers) == GDK_SHIFT_MASK)
    {
        double val = m_hadjust->get_value();
        double step = m_hadjust->get_step_increment();
        double upper = m_hadjust->get_upper();
        if (a_ev->direction == GDK_SCROLL_DOWN)
        {
            if (val + step < upper)
                m_hadjust->set_value(val + step);
            else
                m_hadjust->set_value(upper);
        }
        else if (a_ev->direction == GDK_SCROLL_UP)
        {
            m_hadjust->set_value(val - step);
        }
        return true;
    }
    return false;  // means "not handled"
}

/**
 *  Handles a key-press event.
 */

bool
seqedit::on_key_press_event (GdkEventKey * a_ev)
{
    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask();
    if ((a_ev->state & modifiers) == GDK_CONTROL_MASK && a_ev->keyval == 'w')
        return on_delete_event((GdkEventAny*)a_ev);
    else
        return Gtk::Window::on_key_press_event(a_ev);
}

/*
 * seqedit.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
