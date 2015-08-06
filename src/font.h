#ifndef SEQ24_FONT_H
#define SEQ24_FONT_H

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
 * \file          font.h
 *
 *  This module declares/defines the base class for font handling.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-05
 * \license       GNU GPLv2 or above
 *
 */

#include <gtkmm/image.h>

using namespace Gtk;

class font
{

public:

    /**
     *  A simple enumeration to describe the basic colors used in writing
     *  text.
     */

    enum Color
    {
        BLACK = 0,
        WHITE = 1
    };

private:

    Glib::RefPtr<Gdk::Pixmap> m_pixmap;
    Glib::RefPtr<Gdk::Pixmap> m_black_pixmap;
    Glib::RefPtr<Gdk::Pixmap> m_white_pixmap;
    Glib::RefPtr<Gdk::Bitmap> m_clip_mask;

public:

    font ();

    void init (Glib::RefPtr<Gdk::Window> a_window);
    void render_string_on_drawable
    (
        Glib::RefPtr<Gdk::GC> m_gc,
        int x,
        int y,
        Glib::RefPtr<Gdk::Drawable> a_draw,
        const char * str,
        font::Color col
    );

};

/*
 * Global symbols
 */

extern font * p_font_renderer;

#endif   // SEQ24_FONT_H

/*
 * font.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
