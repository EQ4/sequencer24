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
 * \file          perfedit.cpp
 *
 *  This module declares/defines the base class for the Performance Editor,
 *  also known as the Song Editor.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-02
 * \license       GNU GPLv2 or above
 *
 */

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
#include <gtkmm/tooltips.h>             // #include <gtkmm/tooltip.h>
#include <gtkmm/invisible.h>
#include <gtkmm/arrow.h>
#include <gtkmm/image.h>

#include <sigc++/bind.h>

#include "easy_macros.h"
#include "gtk_helpers.h"
#include "perfedit.h"
#include "perform.h"
#include "perfnames.h"
#include "perfroll.h"
#include "perftime.h"

#include "pixmaps/snap.xpm"
#include "pixmaps/play2.xpm"
#include "pixmaps/stop.xpm"
#include "pixmaps/expand.xpm"
#include "pixmaps/collapse.xpm"
#include "pixmaps/loop.xpm"
#include "pixmaps/copy.xpm"
#include "pixmaps/undo.xpm"
#include "pixmaps/down.xpm"
#include "pixmaps/perfedit.xpm"

using namespace sigc;

/**
 *  Principal constructor, has a pointer to a perform object.
 *  We've reordered the pointer members and put them in the initializer
 *  list to make the constructor a bit cleaner.
 */

perfedit::perfedit (perform * a_perf)
 :
    m_mainperf          (a_perf),
    m_table             (manage(new Table(6, 3, false))),
    m_vadjust           (manage(new Adjustment(0, 0, 1, 1, 1, 1))),
    m_hadjust           (manage(new Adjustment(0, 0, 1, 1, 1, 1))),
    m_vscroll           (manage(new VScrollbar(*m_vadjust))),
    m_hscroll           (manage(new HScrollbar(*m_hadjust))),
    m_perfnames         (manage(new perfnames(m_mainperf, m_vadjust))),
    m_perfroll          (manage(new perfroll(m_mainperf, m_hadjust, m_vadjust))),
    m_perftime          (manage(new perftime(m_mainperf, m_hadjust))),
    m_menu_snap         (manage(new Menu())),
    m_button_snap       (manage(new Button())),
    m_entry_snap        (manage(new Entry())),
    m_button_stop       (manage(new Button())),
    m_button_play       (manage(new Button())),
    m_button_loop       (manage(new ToggleButton())),
    m_button_expand     (manage(new Button())),
    m_button_collapse   (manage(new Button())),
    m_button_copy       (manage(new Button())),
    m_button_grow       (manage(new Button())),
    m_button_undo       (manage(new Button())),
    m_button_bpm        (manage(new Button())),
    m_entry_bpm         (manage(new Entry())),
    m_button_bw         (manage(new Button())),
    m_entry_bw          (manage(new Entry())),
    m_hbox              (manage(new HBox(false, 2))),
    m_hlbox             (manage(new HBox(false, 2))),
    m_tooltips          (manage(new Tooltips())),
    m_menu_bpm          (manage(new Menu())),
    m_menu_bw           (manage(new Menu())),
    m_snap              (8),                    // (c_ppqn / 4),
    m_bpm               (4),
    m_bw                (4)
{
    using namespace Menu_Helpers;

    set_icon(Gdk::Pixbuf::create_from_xpm_data(perfedit_xpm));
    set_title("Sequencer24 - Song Editor");                   /* main window */
    set_size_request(700, 400);
    m_table->set_border_width(2);
    m_hlbox->set_border_width(2);

    m_button_grow->add(*manage(new Arrow(Gtk::ARROW_RIGHT, Gtk::SHADOW_OUT)));
    m_button_grow->signal_clicked().connect(mem_fun(*this, &perfedit::grow));
    add_tooltip(m_button_grow, "Increase size of Grid.");

    /*
     * Fill the table
     */

    m_table->attach(*m_hlbox,  0, 3, 0, 1,  Gtk::FILL, Gtk::SHRINK, 2, 0);
    m_table->attach(*m_perfnames,    0, 1, 2, 3, Gtk::SHRINK, Gtk::FILL);
    m_table->attach(*m_perftime, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
    m_table->attach
    (
        *m_perfroll, 1, 2, 2, 3, Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK
    );
    m_table->attach(*m_vscroll, 2, 3, 2, 3, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);
    m_table->attach(*m_hbox,  0, 1, 3, 4,  Gtk::FILL, Gtk::SHRINK, 0, 2);
    m_table->attach(*m_hscroll, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    m_table->attach(*m_button_grow, 2, 3, 3, 4, Gtk::SHRINK, Gtk::SHRINK);

    m_menu_snap->items().push_back
    (
        MenuElem("1/1", sigc::bind(mem_fun(*this, &perfedit::set_snap), 1))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/2", sigc::bind(mem_fun(*this, &perfedit::set_snap), 2))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/4", sigc::bind(mem_fun(*this, &perfedit::set_snap), 4))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/8", sigc::bind(mem_fun(*this, &perfedit::set_snap), 8))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/16", sigc::bind(mem_fun(*this, &perfedit::set_snap), 16))
    );
    m_menu_snap->items().push_back
    (
        MenuElem("1/32", sigc::bind(mem_fun(*this, &perfedit::set_snap), 32))
    );

    m_button_snap->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(snap_xpm)))
    );
    m_button_snap->signal_clicked().connect
    (
        bind<Menu *>(mem_fun(*this, &perfedit::popup_menu), m_menu_snap)
    );
    add_tooltip(m_button_snap, "Grid snap. (Fraction of Measure Length)");

    m_entry_snap->set_size_request(40, -1);
    m_entry_snap->set_editable(false);

    m_menu_bw->items().push_back
    (
        MenuElem("1", sigc::bind(mem_fun(*this, &perfedit::set_bw), 1))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("2", sigc::bind(mem_fun(*this, &perfedit::set_bw), 2))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("4", sigc::bind(mem_fun(*this, &perfedit::set_bw), 4))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("8", sigc::bind(mem_fun(*this, &perfedit::set_bw), 8))
    );
    m_menu_bw->items().push_back
    (
        MenuElem("16", sigc::bind(mem_fun(*this, &perfedit::set_bw), 16))
    );

    char b[20];
    for (int i = 0; i < 16; i++)
    {
        snprintf(b, sizeof(b), "%d", i + 1);
        m_menu_bpm->items().push_back
        (
            MenuElem(b, sigc::bind(mem_fun(*this, &perfedit::set_bpm), i + 1))
        );
    }

    m_button_bpm->add                               /* beats per measure */
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(down_xpm)))
    );
    m_button_bpm->signal_clicked().connect
    (
        bind<Menu *>(mem_fun(*this, &perfedit::popup_menu), m_menu_bpm)
    );
    add_tooltip(m_button_bpm, "Time Signature. Beats per Measure");

    m_entry_bpm->set_width_chars(2);
    m_entry_bpm->set_editable(false);

    m_button_bw->add                                /* beat width */
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(down_xpm)))
    );
    m_button_bw->signal_clicked().connect
    (
        bind<Menu *>(mem_fun(*this, &perfedit::popup_menu), m_menu_bw)
    );
    add_tooltip(m_button_bw, "Time Signature.  Length of Beat");

    m_entry_bw->set_width_chars(2);
    m_entry_bw->set_editable(false);

    m_button_undo->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(undo_xpm)))
    );
    m_button_undo->signal_clicked().connect
    (
        mem_fun(*this, &perfedit::undo)
    );
    add_tooltip(m_button_undo, "Undo.");

    m_button_expand->add                            /* expand */
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(expand_xpm)))
    );
    m_button_expand->signal_clicked().connect
    (
        mem_fun(*this, &perfedit::expand)
    );
    add_tooltip(m_button_expand, "Expand between L and R markers.");

    m_button_collapse->add                          /* collapse */
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(collapse_xpm)))
    );
    m_button_collapse->signal_clicked().connect
    (
        mem_fun(*this, &perfedit::collapse)
    );
    add_tooltip(m_button_collapse, "Collapse between L and R markers.");

    m_button_copy->add                              /* expand & copy    */
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(copy_xpm)))
    );
    m_button_copy->signal_clicked().connect(mem_fun(*this, &perfedit::copy));
    add_tooltip(m_button_copy, "Expand and copy between L and R markers.");

    m_button_loop->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(loop_xpm)))
    );
    m_button_loop->signal_toggled().connect
    (
        mem_fun(*this, &perfedit::set_looped)
    );
    add_tooltip(m_button_loop, "Play looped between L and R.");

    m_button_stop->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(stop_xpm)))
    );
    m_button_stop->signal_clicked().connect
    (
        mem_fun(*this, &perfedit::stop_playing)
    );
    add_tooltip(m_button_stop, "Stop playing.");

    m_button_play->add
    (
        *manage(new Image(Gdk::Pixbuf::create_from_xpm_data(play2_xpm)))
    );
    m_button_play->signal_clicked().connect
    (
        mem_fun(*this, &perfedit::start_playing)
    );
    add_tooltip(m_button_play, "Begin playing at L marker.");

    m_hlbox->pack_end(*m_button_copy , false, false);
    m_hlbox->pack_end(*m_button_expand , false, false);
    m_hlbox->pack_end(*m_button_collapse , false, false);
    m_hlbox->pack_end(*m_button_undo , false, false);
    m_hlbox->pack_start(*m_button_stop , false, false);
    m_hlbox->pack_start(*m_button_play , false, false);
    m_hlbox->pack_start(*m_button_loop , false, false);
    m_hlbox->pack_start(*(manage(new VSeparator())), false, false, 4);
    m_hlbox->pack_start(*m_button_bpm , false, false);
    m_hlbox->pack_start(*m_entry_bpm , false, false);
    m_hlbox->pack_start(*(manage(new Label("/"))), false, false, 4);
    m_hlbox->pack_start(*m_button_bw , false, false);
    m_hlbox->pack_start(*m_entry_bw , false, false);
    m_hlbox->pack_start(*(manage(new Label("x"))), false, false, 4);
    m_hlbox->pack_start(*m_button_snap , false, false);
    m_hlbox->pack_start(*m_entry_snap , false, false);

    this->add(*m_table);                    /* add table */

    set_snap(8);
    set_bpm(4);
    set_bw(4);
    add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
}

/**
 *  This rote constructor does nothing.  We're going to have to run the
 *  application through valgrind to make sure that nothing is left behind.
 */

perfedit::~perfedit ()
{
    // Empty body
}

/**
 *  Implement the undo feature (Ctrl-Z).  We pop an Undo trigger, and then
 *  ask the perfroll to queue up a (re)drawing action.
 */

void
perfedit::undo ()
{
    m_mainperf->pop_trigger_undo();
    m_perfroll->queue_draw();
}

/**
 *  Implement the playing.  JACK will be used if it is present and, in the
 *  application, enabled.
 */
\
void
perfedit::start_playing ()
{
    m_mainperf->position_jack(true);
    m_mainperf->start_jack();
    m_mainperf->start(true);
}

/**
 *  Stop the playing.
 */

void
perfedit::stop_playing ()
{
    m_mainperf->stop_jack();
    m_mainperf->stop();
}

/**
 *  Implement the collapse action.  This action removes all events between
 *  the L and R (left and right) markers.  This action is preceded by
 *  pushing an Undo operation in the perform object, not moving its
 *  triggers (the go away), and telling the perfroll to redraw.
 */

void
perfedit::collapse ()
{
    m_mainperf->push_trigger_undo();
    m_mainperf->move_triggers(false);
    m_perfroll->queue_draw();
}

/**
 *  Implement the copy (actually, expand-and-copy) action.  This action
 *  opens up a space  events between the L and R (left and right) markers,
 *  and copies the information from the same amount of event that follow
 *  the R marker.  This action is preceded by pushing an Undo operation in
 *  the perform object, copying its triggers, and telling the perfroll to
 *  redraw.
 */

void
perfedit::copy ()
{
    m_mainperf->push_trigger_undo();
    m_mainperf->copy_triggers();
    m_perfroll->queue_draw();
}

/**
 *  Implement the expand action.  This action opens up a space  events
 *  between the L and R (left and right) markers.  This action is preceded
 *  by pushing an Undo operation in the perform object, moving its
 *  triggers, and telling the perfroll to redraw.
 */

void
perfedit::expand ()
{
    m_mainperf->push_trigger_undo();
    m_mainperf->move_triggers(true);
    m_perfroll->queue_draw();
}

/**
 *  Set the looping in the perfrom object.
 */

void
perfedit::set_looped ()
{
    m_mainperf->set_looping(m_button_loop->get_active());
}

/**
 *  Opens the given popup menu.
 */

void
perfedit::popup_menu (Menu * a_menu)
{
    a_menu->popup(0, 0);
}

/**
 *  Sets the guides, which are (what) user-interface element.
 *  See the set_snap() function.
 *
 *  The c_ppqn variable should be copied into a member.
 */

void
perfedit::set_guides ()
{
    long measure_ticks = (c_ppqn * 4) * m_bpm / m_bw;
    long snap_ticks = measure_ticks / m_snap;
    long beat_ticks = (c_ppqn * 4) / m_bw;
    m_perfroll->set_guides(snap_ticks, measure_ticks, beat_ticks);
    m_perftime->set_guides(snap_ticks, measure_ticks);
}

/**
 *  Sets the snap text and values to the given value, and then calls
 *  set_guides().
 */

void
perfedit::set_snap (int a_snap)
{
    char b[10];
    snprintf(b, sizeof(b), "1/%d", a_snap);
    m_entry_snap->set_text(b);
    m_snap = a_snap;
    set_guides();
}

/**
 *  Sets the BPM (beats per minute) text and values to the given value,
 *  and then calls set_guides().
 */

void
perfedit::set_bpm (int a_beats_per_measure)
{
    char b[10];
    snprintf(b, sizeof(b), "%d", a_beats_per_measure);
    m_entry_bpm->set_text(b);
    m_bpm = a_beats_per_measure;
    set_guides();
}

/**
 *  Sets the BW (beat width, or the denominator in the time signature)
 *  text and values to the given value, and then calls set_guides().
 */

void
perfedit::set_bw (int a_beat_width)
{
    char b[10];
    snprintf(b, sizeof(b), "%d", a_beat_width);
    m_entry_bw->set_text(b);
    m_bw = a_beat_width;
    set_guides();
}

/**
 *  Increments the size of the perfroll and perftime objects.
 */

void
perfedit::grow ()
{
    m_perfroll->increment_size();
    m_perftime->increment_size();
}

/**
 *  This function forwards its call to the perfroll function of the same
 *  name.  It does not seem to need to also forward to the perftime
 *  function of the same name.
 */

void
perfedit::init_before_show ()
{
    m_perfroll->init_before_show();
}

/**
 *  Handles a drawing timeout.  It redraws "dirty" sequences in the
 *  perfroll and the perfnames objects, and shows draw progress on the
 *  perfroll.
 */

bool
perfedit::timeout ()
{
    m_perfroll->redraw_dirty_sequences();
    m_perfroll->draw_progress();
    m_perfnames->redraw_dirty_sequences();
    return true;
}

/**
 *  This callback function calls the base-class on_realize() fucntion, and
 *  then connects the perfedit::timeout() function to the Glib
 *  signal-timeout, with a redraw timeout of c_redraw_ms.
 */

void
perfedit::on_realize ()
{
    Gtk::Window::on_realize();
    Glib::signal_timeout().connect
    (
        mem_fun(*this, &perfedit::timeout), c_redraw_ms
    );
}

/**
 *  All this callback function does is return false.
 */

bool
perfedit::on_delete_event (GdkEventAny * a_event)
{
    return false;
}

/**
 *  This function is the callback for a key-press event.
 */

bool
perfedit::on_key_press_event (GdkEventKey * a_ev)
{
    bool event_was_handled = false;
    if (a_ev->type == GDK_KEY_PRESS)
    {
        if (global_print_keys)
        {
            printf
            (
                "key_press[%d] == %s\n",
                a_ev->keyval, gdk_keyval_name(a_ev->keyval)
            );
        }

        /*
         *  By default, the space-bar starts the playing, and the Escape
         *  key stops the playing.  The start/end key may be the same key
         *  (i.e. space-bar), allow toggling when the same key is mapped
         *  to both triggers.
         */

        bool dont_toggle = m_mainperf->m_key_start != m_mainperf->m_key_stop;
        if
        (
            a_ev->keyval == m_mainperf->m_key_start &&
            (dont_toggle || !m_mainperf->is_running())
        )
        {
            start_playing();
            return true;
        }
        else if
        (
            a_ev->keyval == m_mainperf->m_key_stop &&
            (dont_toggle || m_mainperf->is_running())
        )
        {
            stop_playing();
            return true;
        }

        if
        (
            a_ev->keyval == m_mainperf->m_key_start ||
            a_ev->keyval == m_mainperf->m_key_stop
        )
        {
            event_was_handled = true;
        }
    }
    if (! event_was_handled)
        return Gtk::Window::on_key_press_event(a_ev);

    return false;
}

/*
 * perfedit.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
