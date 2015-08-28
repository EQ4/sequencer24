#ifndef SEQ24_MUTEX_H
#define SEQ24_MUTEX_H

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
 * \file          mutex.h
 *
 *  This module declares/defines the base class for mutexes.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-27
 * \license       GNU GPLv2 or above
 *
 *  This module defines the following classes:
 *
 *      -   mutex.  A primitive wrapper for pthread_mutex_t.
 *      -   automutex. A way to lock a function more safely and easily.
 *      -   condition_var.  Provides a common usage paradigm, for the
 *          perform object.
 */

#include <pthread.h>

/**
 *  The mutex class provides a simple wrapper for the pthread_mutex_t type
 *  used as a recursive mutex.
 */

class mutex
{

private:

    /**
     *  Provides a way to disable the locking.  Mostly experimental,
     *  we want to disable locking to see if we can speed up MIDI file
     *  reading when the application is compiled for debugging.  It takes
     *  about 8 seconds to read our sample MIDI files.  This does not
     *  solve the problem of the long MIDI-file parsing, however.
     *
     *      static bool sm_mutex_enabled;
     */

    /**
     *  Provides a recursive mutex that can be used by the whole
     *  application, apparently.
     */

    static const pthread_mutex_t sm_recursive_mutex;

protected:

    /**
     *  Provides a mutex lock usable by a single module or class.
     */

    mutable pthread_mutex_t m_mutex_lock;

public:

    mutex ();
    void lock () const;
    void unlock () const;

    /*
     * static void enable (bool flag = true)        // experimental
     * {
     *     sm_mutex_enabled = flag;
     * }
     */

};

/**
 *  Provides a mutex that locks automatically when created, and unlocks
 *  when destroyed.  This has a couple of benefits.  First, it is more
 *  threadsafe in the face of exception handling.  Secondly, it can be
 *  done with just one line of code.
 */

class automutex
{

private:

    /**
     *  Provides the mutex reference to be used for locking.
     */

    mutex & m_safety_mutex;

private:        // do not allow these functions to be used

    automutex ();
    automutex (const automutex &);
    automutex & operator =(const automutex &);

public:

    /**
     *  Principal constructor gets a references to a mutex parameter, and
     *  then locks the mutex.
     */

    automutex (mutex & my_mutex) : m_safety_mutex (my_mutex)
    {
        m_safety_mutex.lock();
    }

    /**
     *  The destructor unlocks the mutex.
     */

    ~automutex ()
    {
        m_safety_mutex.unlock();
    }

};

/**
 *  A mutex works best in conjunction with a condition variable.
 *  Therefore this class derives from the mutex class.  A "has-a"
 *  relationship might be more logical than this "is-a" relationship.
 */

class condition_var : public mutex
{

private:

    /**
     *  Provides a "global" condition variable.
     */

    static const pthread_cond_t cond;

    /**
     *  Provides a class-specific condition variable.
     */

    pthread_cond_t m_cond;

public:

    condition_var ();
    void wait ();
    void signal ();

};

#endif  // SEQ24_MUTEX_H

/*
 * mutex.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
