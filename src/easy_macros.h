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
 * \updates       2015-07-24
 * \version       $Revision$
 * \license       GNU GPL v2 or above
 *
 *    The macros in this file cover:
 *
 *       -  Language-support macros.
 *       -  Error and information output macros.
 *
 * Copyright (C) 2013-2015 Chris Ahlstrom <ahlstromcj@gmail.com>
 */

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

#endif         /* SEQ24_EASY_MACROS_H */

/*
 * easy_macros.h
 *
 * vim: sw=4 ts=4 wm=8 et ft=c
 */
