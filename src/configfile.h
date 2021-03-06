#ifndef SEQ24_CONFIGFILE_H
#define SEQ24_CONFIGFILE_H

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
 * \file          configfile.h
 *
 *  This module declares the abstract base class for configuration and
 *  options files.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-10
 * \license       GNU GPLv2 or above
 *
 */

#include <fstream>
#include <string>
#include <list>
#include <gtkmm/drawingarea.h>

class perform;

/**
 *  A manifest constant for controlling the length of a line-reading
 *  array in a configuration file.
 */

#define SEQ24_LINE_MAX                 1024

/**
 *    This class is the abstract base class for optionsfile and userfile.
 */

class configfile
{

protected:

    /**
     *  Provides the name of the file.
     */

    std::string m_name;

    /**
     *   Points to an allocated buffer that holds the data for the
     *   configuration file.
     */

    unsigned char * m_d;

    /**
     *  The current line of text being processed.  This member receives
     *  an input line, and so needs to be a character buffer.
     */

    char m_line[SEQ24_LINE_MAX];

protected:

    void next_data_line (std::ifstream & a_file);
    void line_after (std::ifstream & a_file, const std::string & a_tag);

public:

    configfile (const std::string & a_name);

    /**
     *  A rote constructor needed for a base class.
     */

    virtual ~configfile()
    {
        // empty body
    }

    virtual bool parse (perform * a_perf) = 0;
    virtual bool write (perform * a_perf) = 0;

};

#endif  // SEQ24_CONFIGFILE_H

/*
 * configfile.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
