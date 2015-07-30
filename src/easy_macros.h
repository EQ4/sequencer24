#ifndef SEQ24_EASY_MACROS_H
#define SEQ24_EASY_MACROS_H

/**
 * \file          easy_macros.h
 *
 *    This module provides macros for generating simple messages, MIDI
 *    parameters, and more.
 *
 * \library       sequencer24
 * \author        Chris Ahlstrom and other authors; see documentation
 * \date          2013-11-17
 * \updates       2015-07-30
 * \version       $Revision$
 * \license       GNU GPL v2 or above
 *
 *    The macros in this file cover:
 *
 *       -  Compiler-support macros.
 *       -  Error and information output macros.
 *       -  One or more global debugging functions that are better suited
 *          than using a macro.
 *
 * Copyright (C) 2013-2015 Chris Ahlstrom <ahlstromcj@gmail.com>
 */

#include <stdio.h>
#include "platform_macros.h"

#ifdef PLATFORM_DEBUG
#include <string>
#endif

#ifdef PLATFORM_WINDOWS
#include "configwin32.h"
#else
#include "config.h"
#endif

/**
 * Language macros:
 *
 *    Provides an alternative to NULL.
 */

#ifndef __cplus_plus
#define nullptr                  0
#elif __cplus_plus <= 199711L
#define nullptr                  0
#endif

/**
 *    Provides a way to declare functions as having either a C++ or C
 *    interface.
 */

#ifndef EXTERN_C_DEC

#ifdef __cplusplus
#define EXTERN_C_DEC extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_DEC
#define EXTERN_C_END
#endif

#endif

/**
 *    Test for being a valid pointer.
 */

#define not_nullptr(x)     ((x) != nullptr)

/**
 *    Test for being an invalid pointer.
 */

#define is_nullptr(x)      ((x) == nullptr)

/**
 *    A more obvious boolean type.
 */

#ifndef __cplus_plus
typedef int cbool_t;
#elif __cplusplus <= 199711L
typedef bool cbool_t;
#endif

/**
 *    Provides the "false" value of the wbool_t type definition.
 */

#ifndef __cplus_plus
#define false    0
#endif

/**
 *    Provides the "true" value of the wbool_t type definition.
 */

#ifndef __cplus_plus
#define true     1
#endif

/**
 *    GCC provides three magic variables which hold the name of the current
 *    function, as a string.  The first of these is `__func__', which is part
 *    of the C99 standard:
 *
 *    The identifier `__func__' is implicitly declared by the translator.  It
 *    is the name of the lexically-enclosing function.  This name is the
 *    unadorned name of the function.
 *
 *    `__FUNCTION__' is another name for `__func__'.  Older versions of GCC
 *    recognize only this name.  However, it is not standardized.  For maximum
 *    portability, use `__func__', but provide a fallback definition with
 *    the preprocessor, as done below.
 *
 *    `__PRETTY_FUNCTION__' is the decorated version of the function name.
 *    It is longer, but more informative.  It is also deprecated.  But, for
 *    now, we'll continue to use it
 *
 *    Visual Studio defines only __FUNCTION__, so a definition is provided
 *    below.
 *
 *//*-------------------------------------------------------------------------*/

#define USE_PRETTY_FUNCTION   1

#ifdef PLATFORM_GNU

#ifndef __func__
#if __STDC_VERSION__ < 199901L
#if __GNUC__ >= 2

#ifdef USE_PRETTY_FUNCTION                            /* TODO                 */
#define __func__        __PRETTY_FUNCTION__           /* adorned func names   */
#else
#define __func__        __FUNCTION__                  /* bald func names      */
#endif

#else
#define __func__        "<unknown>"
#endif  // __GNUC__
#endif  // __STDC_VERSION__
#endif  // __func__

#else   // ! PLATFORM_GNU

#ifndef __func__
#define __func__        __FUNCTION__    // Windows?
#endif

#endif  // PLATFORM_GNU

/**
 *    Provides an error reporting macro (which happens to match Chris's XPC
 *    error function.
 */

#define errprint(x)           fprintf(stderr, "? %s\n", x)

/**
 *    Provides an error reporting macro that requires a sprintf() format
 *    specifier as well.
 */

#define errprintf(fmt, x)     fprintf(stderr, fmt, x)

/**
 *    Provides a warning reporting macro (which happens to match Chris's
 *    XPC error function.
 */

#define warnprint(x)          fprintf(stderr, "! %s\n", x)

/**
 *    Provides an error reporting macro that requires a sprintf() format
 *    specifier as well.
 */

#define warnprintf(fmt, x)    fprintf(stderr, fmt, x)

/**
 *    Provides an information reporting macro (which happens to match
 *    Chris's XPC information function.
 */

#define infoprint(x)          fprintf(stderr, "* %s\n", x)

/**
 *    Provides an error reporting macro that requires a sprintf() format
 *    specifier as well.
 */

#define infoprintf(fmt, x)    fprintf(stderr, fmt, x)

/*
 * Global functions.  The not_nullptr_assert() function is a macro in
 * release mode, to speed up release mode.  It cannot do anything at
 * all, since it is used in the conditional part of if-statements.
 */

#ifdef PLATFORM_DEBUG
extern bool not_nullptr_assert (void * ptr, const std::string & context);
#else
#define not_nullptr_assert(ptr, context) (not_nullptr(ptr))
#endif

#endif         /* SEQ24_EASY_MACROS_H */

/*
 * easy_macros.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */
