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
 * \file          seq24.cpp
 *
 *  This module declares/defines the main module for the application.
 *
 * \library       sequencer24 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-24
 * \updates       2015-08-07
 * \license       GNU GPLv2 or above
 *
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "easy_macros.h"               // full platform configuration
#include "font.h"

#ifdef LASH_SUPPORT
#include "lash.h"
#endif

#include "globals.h"
#include "mainwnd.h"
#include "midifile.h"
#include "optionsfile.h"
#include "perform.h"
#include "userfile.h"

/**
 *  A structure for command parsing.
 */

static struct option long_options[] =
{
    {"help",                0, 0, 'h'},
    {"showmidi",            0, 0, 's'},
    {"show_keys",           0, 0, 'k'},
    {"stats",               0, 0, 'S'},
    {"priority",            0, 0, 'p'},
    {"ignore",              required_argument, 0, 'i'},
    {"interaction_method",  required_argument, 0, 'x'},
    {"jack_transport",      0, 0, 'j'},
    {"jack_master",         0, 0, 'J'},
    {"jack_master_cond",    0, 0, 'C'},
    {"jack_start_mode",     required_argument, 0, 'M'},
    {"jack_session_uuid",   required_argument, 0, 'U'},
    {"manual_alsa_ports",   0, 0, 'm'},
    {"pass_sysex",          0, 0, 'P'},
    {"version",             0, 0, 'V'},
    {0, 0, 0, 0}
};

static const char versiontext[] = PACKAGE " " VERSION "\n";

/*
 * Global pointer!  Declared in font.h.
 */

font * p_font_renderer = nullptr;

#ifdef LASH_SUPPORT
lash * lash_driver = nullptr;
#endif

#ifdef PLATFORM_WINDOWS
#define HOME "HOMEPATH"
#define SLASH "\\"
#else
#define HOME "HOME"
#define SLASH "/"
#endif

const char * const g_help_1 =
"Usage: seq24 [OPTIONS] [FILENAME]\n\n"
"Options:\n"
"   -h, --help               Show this message.\n"
"   -v, --version            Show program version information.\n"
"   -m, --manual_alsa_ports  Don't attach ALSA ports.\n"
"   -s, --showmidi           Dump incoming MIDI events to the screen.\n"
"   -p, --priority           Runs higher priority with FIFO scheduler\n"
"                            (must be root)\n"
"   -P, --pass_sysex         Passes incoming SysEx messages to all outputs.\n"
"   -i, --ignore n           Ignore ALSA device number.\n"
    ;

const char * const g_help_2 =
"   -k, --show_keys          Prints pressed key value.\n"
"   -j, --jack_transport     Sync to JACK transport\n"
"   -J, --jack_master        Try to be JACK master\n"
"   -C, --jack_master_cond   JACK master will fail if there's already a master.\n"
"   -M, --jack_start_mode m  When synced to JACK, the following play modes\n"
"                            are available: 0 = live mode;\n"
"                            1 = song mode (the default).\n"
"   -S, --stats              Show statistics\n"
"   -x, --interaction_method n  See .seq24rc for methods to use.\n"
"   -U, --jack_session_uuid u   Set UUID for JACK session\n"
"\n\n\n"
    ;

/**
 *  The standard C/C++ entry point to this application.
 */

int
main (int argc, char * argv [])
{
    /*
     *  Scan the argument vector and strip off all parameters known to
     *  GTK+.
     */

    Gtk::Main kit(argc, argv);

    /*
     *  Initialize the lash driver (strips lash specific command line
     *  arguments, but does not connect to daemon)
     */

#ifdef LASH_SUPPORT
    lash_driver = new lash(argc, argv);
#endif

    int c;
    while (true)                /* parse parameters */
    {
        int option_index = 0;   /* getopt_long stores index here */
        c = getopt_long
        (
            argc, argv, "C:hi:jJmM:pPsSU:Vx:", long_options, &option_index
        );
        if (c == -1)            /* detect the end of the options */
            break;

        switch (c)
        {
        case '?':
        case 'h':
            printf(g_help_1);
            printf(g_help_2);
            return EXIT_SUCCESS;
            break;

        case 'S':
            global_stats = true;
            break;

        case 's':
            global_showmidi = true;
            break;

        case 'p':
            global_priority = true;
            break;

        case 'P':
            global_pass_sysex = true;
            break;

        case 'k':
            global_print_keys = true;
            break;

        case 'j':
            global_with_jack_transport = true;
            break;

        case 'J':
            global_with_jack_master = true;
            break;

        case 'C':
            global_with_jack_master_cond = true;
            break;

        case 'M':
            if (atoi(optarg) > 0)
                global_jack_start_mode = true;
            else
                global_jack_start_mode = false;
            break;

        case 'm':
            global_manual_alsa_ports = true;
            break;

        case 'i':                           /* ignore ALSA device */
            global_device_ignore = true;
            global_device_ignore_num = atoi(optarg);
            break;

        case 'V':
            printf("%s", versiontext);
            return EXIT_SUCCESS;
            break;

        case 'U':
            global_jack_session_uuid = std::string(optarg);
            break;

        case 'x':
            global_interactionmethod = (interaction_method_e)atoi(optarg);
            break;

        default:
            break;
        }
    }

    /*
     *  Prepare global MIDI definitions.
     */

    for (int i = 0; i < c_max_busses; i++)
    {
        for (int j = 0; j < 16; j++)
            global_user_midi_bus_definitions[i].instrument[j] = -1;
    }

    for (int i = 0; i < c_max_instruments; i++)
    {
        for (int j = 0; j < 128; j++)
            global_user_instrument_definitions[i].controllers_active[j] = false;
    }

    perform p;                         /* the main performance object */
    p_font_renderer = new font();      /* set the font renderer       */
    if (getenv(HOME) != NULL)          /* is $HOME set?               */
    {
        std::string home(getenv(HOME));
        std::string total_file = home + SLASH + global_config_filename;
        global_last_used_dir = home;
        if (Glib::file_test(total_file, Glib::FILE_TEST_EXISTS))
        {
            printf("Reading [%s]\n", total_file.c_str());
            optionsfile options(total_file);
            if (! options.parse(&p))
                printf("Error Reading [%s]\n", total_file.c_str());
        }
        total_file = home + SLASH + global_user_filename;
        if (Glib::file_test(total_file, Glib::FILE_TEST_EXISTS))
        {
            printf("Reading [%s]\n", total_file.c_str());
            userfile user(total_file);
            if (! user.parse(&p))
                printf("Error Reading [%s]\n", total_file.c_str());
        }
    }
    else
        printf("Error calling getenv( \"%s\" )\n", HOME);

    p.init();
    p.launch_input_thread();
    p.launch_output_thread();
    p.init_jack();

    mainwnd seq24_window(&p);          /* push a mainwnd onto stack */
    if (optind < argc)
    {
        if (Glib::file_test(argv[optind], Glib::FILE_TEST_EXISTS))
            seq24_window.open_file(argv[optind]);
        else
            printf("File not found: %s\n", argv[optind]);
    }

#ifdef LASH_SUPPORT

    /*
     * Connect to LASH daemon and poll events.
     */

    lash_driver->init(&p);
    lash_driver->start();

#endif

    kit.run(seq24_window);
    p.deinit_jack();
    if (getenv(HOME) != NULL)
    {
        std::string home(getenv(HOME));
        std::string total_file = home + SLASH + global_config_filename;
        printf("Writing [%s]\n", total_file.c_str());
        optionsfile options(total_file);
        if (!options.write(&p))
            printf("Error writing [%s]\n", total_file.c_str());
    }
    else
    {
        printf("Error calling getenv( \"%s\" )\n", HOME);
    }

#ifdef LASH_SUPPORT
    delete lash_driver;
#endif

    return EXIT_SUCCESS;
}

/*
 * seq24.cpp
 *
 * vim: sw=4 ts=4 wm=8 et ft=cpp
 */

