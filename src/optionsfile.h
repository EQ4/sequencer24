#ifndef SEQ24_OPTIONSFILE_H
#define SEQ24_OPTIONSFILE_H

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
 * \file          optionsfile.h
 *
 *  This module declares/defines the base class for managind the ~/.seq24rc
 *  configuration file.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-05
 * \license       GNU GPLv2 or above
 *
 */

#include "configfile.h"

class perform;

/**
 *  Provides a file for reading and writing the application' main
 *  configuration file.
 */

class optionsfile : public configfile
{

public:

    optionsfile (const std::string & a_name);
    ~optionsfile ();

    bool parse (perform * a_perf);
    bool write (perform * a_perf);

};

#endif   // SEQ24_OPTIONSFILE_H

/*
 * optionsfile.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
