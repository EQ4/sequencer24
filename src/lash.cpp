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
 * \file          lash.cpp
 *
 *  This module declares/defines the base class for LASH support.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-09
 * \license       GNU GPLv2 or above
 *
 *  Not totally sure that the LASH support is completely finished, at this
 *  time.  The version that ships with Debian Sid does not have it
 *  enabled.
 */

#include <string>
#include <sigc++/slot.h>
#include <gtkmm.h>

#include "easy_macros.h"
#include "lash.h"
#include "midifile.h"
#include "perform.h"

/**
 *  This constructor calls lash_extract(), using the command-line
 *  arguments, if LASH_SUPPORT is enabled.  We fixed the crazy usage of
 *  argc and argv here and in the client code in the seq24 module.
 */

lash::lash (int argc, char ** argv)
#ifdef LASH_SUPPORT
 :
    m_perform       (nullptr),
    m_client        (nullptr),
    m_lash_args     (nullptr)
#endif // LASH_SUPPORT
{
#ifdef LASH_SUPPORT
    m_lash_args = lash_extract_args(&argc, &argv);
#endif // LASH_SUPPORT
}

/**
 *  Initializes LASH support, if enabled.
 */

void lash::init (perform * p)
{
    if (not_nullptr(p))
    {
#ifdef LASH_SUPPORT
        m_perform = p;
        m_client = lash_init
        (
            m_lash_args, PACKAGE_NAME, LASH_Config_File, LASH_PROTOCOL(2, 0)
        );
        if (m_client == NULL)
        {
            fprintf(stderr, "Cannot connect to LASH; no session management.\n");
        }
        else
        {
            lash_event_t * event = lash_event_new_with_type(LASH_Client_Name);
            lash_event_set_string(event, "Seq24");
            lash_send_event(m_client, event);
            printf("[Connected to LASH]\n");
        }
#endif // LASH_SUPPORT
    }
}

/**
 *  Make ourselves a LASH ALSA client.
 */

void
lash::set_alsa_client_id (int id)
{
#ifdef LASH_SUPPORT
    lash_alsa_client_id(m_client, id);
#endif
}

/**
 *  Process any LASH events every 250 msec, which is an arbitrarily chosen
 *  interval.
 */

void
lash::start()
{
#ifdef LASH_SUPPORT
    Glib::signal_timeout().connect
    (
        sigc::mem_fun(*this, &lash::process_events), 250
    );
#endif // LASH_SUPPORT
}

#ifdef LASH_SUPPORT

/**
 *  Process LASH events.
 */

bool
lash::process_events ()
{
    lash_event_t * ev = NULL;
    while ((ev = lash_get_event(m_client)) != NULL)     // process events
    {
        handle_event(ev);
        lash_event_destroy(ev);
    }
    return true;
}

/**
 *  Handle a LASH event.
 */

void
lash::handle_event (lash_event_t * ev)
{
    LASH_Event_Type type = lash_event_get_type(ev);
    const char * cstring = lash_event_get_string(ev);
    std::string str = (cstring == NULL) ? "" : cstring;
    if (type == LASH_Save_File)
    {
        midifile f(str + "/seq24.mid");
        f.write(m_perform);
        lash_send_event(m_client, lash_event_new_with_type(LASH_Save_File));
    }
    else if (type == LASH_Restore_File)
    {
        midifile f(str + "/seq24.mid");
        f.parse(m_perform, 0);
        lash_send_event(m_client, lash_event_new_with_type(LASH_Restore_File));
    }
    else if (type == LASH_Quit)
    {
        m_client = NULL;
        Gtk::Main::quit();
    }
    else
    {
        fprintf(stderr, "Warning:  Unhandled LASH event.\n");
    }
}

/*
 * ca 2015-07-24
 * Eliminate this annoying warning.  Will do it for Microsoft's bloddy
 * compiler later.
 */

#ifdef PLATFORM_GNU
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

/**
 *  Handle a LASH configuration item.
 */
void
lash::handle_config (lash_config_t * conf)
{
    const char * key = lash_config_get_key(conf);
    const void * val = lash_config_get_value(conf);
    size_t val_size = lash_config_get_value_size(conf);

    /*
     * Nothing is done with these, just gives warnings about "unused"
     */
}

#endif // LASH_SUPPORT

/*
 * lash.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
