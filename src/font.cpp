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
 * \file          font.cpp
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

#include "easy_macros.h"
#include "font.h"
#include "pixmaps/font_w.xpm"
#include "pixmaps/font_b.xpm"

/**
 *    Rote default constructor.
 */

font::font ()
 :
    m_pixmap        (),
    m_black_pixmap  (),
    m_white_pixmap  (),
    m_clip_mask     ()
{
   // empty body
}

/**
 *  Initialization function for a window.
 */

void
font::init (Glib::RefPtr<Gdk::Window> a_window)
{
    m_black_pixmap = Gdk::Pixmap::create_from_xpm
    (
        a_window->get_colormap(), m_clip_mask, font_b_xpm
    );
    m_white_pixmap = Gdk::Pixmap::create_from_xpm
    (
        a_window->get_colormap(), m_clip_mask, font_w_xpm
    );
}

/**
 *  Draws the _____
 */

void
font::render_string_on_drawable
(
    Glib::RefPtr<Gdk::GC> a_gc,
    int x, int y,
    Glib::RefPtr<Gdk::Drawable> a_draw,
    const char * str,
    font::Color col
)
{
    int length = 0;
    if (str != nullptr)
        length = strlen(str);

    int font_w = 6;
    int font_h = 10;
    for (int i = 0; i < length; ++i)
    {
        unsigned char c = (unsigned char) str[i];

        // solid:
        //
        // int pixbuf_index_x = 2;
        // int pixbuf_index_y = 0;

        int pixbuf_index_x = c % 16;
        int pixbuf_index_y = c / 16;

        pixbuf_index_x *= 9;
        pixbuf_index_y *= 13;

        pixbuf_index_x += 2;
        pixbuf_index_y += 2;

        if (col == font::BLACK)
            m_pixmap = m_black_pixmap;

        if (col == font::WHITE)
            m_pixmap = m_white_pixmap;

        a_draw->draw_drawable
        (
            a_gc,
            m_pixmap,
            pixbuf_index_x,
            pixbuf_index_y,
            x + (i * font_w),
            y,
            font_w,
            font_h
        );
    }
}

/*
 * font.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
