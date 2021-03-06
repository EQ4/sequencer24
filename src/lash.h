#ifndef SEQ24_LASH_H
#define SEQ24_LASH_H

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
 * \file          lash.h
 *
 *  This module declares/defines the base class for LASH support.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-30
 * \license       GNU GPLv2 or above
 *
 */

#include "easy_macros.h"               // platform and debugging macros

#ifdef LASH_SUPPORT
#include <lash/lash.h>
#endif

class perform;

/**
 *  This class supports LASH operations, if compiled with LASH support
 *  (i.e LASH_SUPPORT is defined). All of the #ifdef skeleton work is done
 *  in this class in such a way that any other part of the code can use
 *  this class whether or not lash support is actually built in; the
 *  functions will just do nothing.
 */

class lash
{

private:

    perform * m_perform;

#ifdef LASH_SUPPORT
    lash_client_t * m_client;
    lash_args_t * m_lash_args;
#endif

    bool m_is_lash_supported;

public:

    lash (int argc, char ** argv);
    void init (perform * perform);
    void set_alsa_client_id (int id);
    void start ();

#ifdef LASH_SUPPORT
private:

    bool process_events ();
    void handle_event (lash_event_t * conf);
    void handle_config (lash_config_t * conf);
#endif

};

/*
 * Global LASH driver, defined in seq24.cpp.
 */

#ifdef LASH_SUPPORT
extern lash * global_lash_driver;
#endif

#endif // SEQ24_LASH_H

/*
 * lash.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
