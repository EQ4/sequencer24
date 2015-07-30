#******************************************************************************
# Makefile.am (sequencer24)
#------------------------------------------------------------------------------
##
# \file       	Makefile.am
# \library    	sequencer24 application
# \author     	Chris Ahlstrom
# \date       	2015-07-24
# \update      2015-07-30
# \version    	$Revision$
# \license    	$XPC_SUITE_GPL_LICENSE$
#
# 		This module provides an Automake makefile for the sequencer24 C/C++
# 		application.
#
#------------------------------------------------------------------------------

#*****************************************************************************
# Packing/cleaning targets
#-----------------------------------------------------------------------------

AUTOMAKE_OPTIONS = foreign dist-zip dist-bzip2
MAINTAINERCLEANFILES = Makefile.in Makefile $(AUX_DIST)

#******************************************************************************
# CLEANFILES
#------------------------------------------------------------------------------

CLEANFILES = *.gc*
MOSTLYCLEANFILES = *~

#******************************************************************************
#  EXTRA_DIST
#------------------------------------------------------------------------------
#
#  getopt_test.c is not ready and is not included at this time.
#	$(TESTS) is not included because it is derived files.
#
#------------------------------------------------------------------------------

EXTRA_DIST = configwin32.h

#******************************************************************************
#  SUBDIRS
#------------------------------------------------------------------------------

SUBDIRS = pixmaps

#******************************************************************************
# Items from configure.ac
#-------------------------------------------------------------------------------
#
#  XPCCUT_LIBS is defined by the PKG_CHECK_MODULES macro.  The xpccut
#  library must be installed officially in order to be used to build the
#  unit-test application of the XPC library.
#
#-------------------------------------------------------------------------------

PACKAGE = @PACKAGE@
VERSION = @VERSION@

#******************************************************************************
# Install directories
#------------------------------------------------------------------------------
#
# 	Not needed, yet, since we won't be installing the app for awhile.
#
#------------------------------------------------------------------------------

prefix = @prefix@
libdir = @xpclibdir@
datadir = @datadir@
datarootdir = @datarootdir@
sequencer24includedir = @sequencer24includedir@
sequencer24libdir = @sequencer24libdir@

#******************************************************************************
# localedir
#------------------------------------------------------------------------------
#
# 	'localedir' is the normal system directory for installed localization
#  files.
#
#------------------------------------------------------------------------------

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

#******************************************************************************
# Local project directories
#------------------------------------------------------------------------------

top_srcdir = @top_srcdir@
builddir = @abs_top_builddir@

#******************************************************************************
# Compiler and linker flags
#------------------------------------------------------------------------------

AM_CXXFLAGS = $(GTKMM_CFLAGS) $(JACK_CFLAGS) $(LASH_CFLAGS) -Wall $(MM_WFLAGS)
sequencer24_LDADD = $(GTKMM_LIBS) $(ALSA_LIBS) $(JACK_LIBS) $(LASH_LIBS)

#******************************************************************************
# The programs to build
#------------------------------------------------------------------------------

bin_PROGRAMS = sequencer24

#******************************************************************************
# Source files
#----------------------------------------------------------------------------

sequencer24_SOURCES = \
	configfile.cpp configfile.h \
	controllers.h \
	easy_macros.cpp easy_macros.h \
	event.cpp event.h \
	font.cpp font.h \
	globals.h \
	keybindentry.cpp keybindentry.h \
	lash.cpp lash.h \
	maintime.cpp maintime.h \
	mainwid.cpp mainwid.h \
	mainwnd.cpp mainwnd.h \
	mastermidibus.cpp mastermidibus.h \
	midibus_common.h \
	midibus.cpp midibus.h \
	mastermidibus_portmidi.cpp mastermidibus_portmidi.h \
	midibus_portmidi.cpp midibus_portmidi.h \
	midifile.cpp midifile.h \
	mutex.cpp mutex.h \
	options.cpp options.h \
	optionsfile.cpp optionsfile.h \
	perfedit.cpp perfedit.h \
	perfnames.cpp perfnames.h \
	perform.cpp perform.h \
	perfroll.cpp perfroll.h \
	perfroll_input.cpp perfroll_input.h \
	perftime.cpp perftime.h \
	platform_macros.h \
	seq24.cpp \
	seqdata.cpp seqdata.h \
	seqedit.cpp seqedit.h \
	seqevent.cpp seqevent.h \
	seqkeys.cpp seqkeys.h \
	seqmenu.cpp seqmenu.h \
	seqroll.cpp seqroll.h \
	seqtime.cpp seqtime.h \
	sequence.cpp sequence.h \
	userfile.cpp userfile.h

#******************************************************************************
# Testing
#------------------------------------------------------------------------------
#
# 	   http://www.gnu.org/software/hello/manual/automake/Simple-Tests.html
#
#------------------------------------------------------------------------------

# testsubdir = test-results
# TESTS_ENVIRONMENT =
# TESTS = test_script

# test: check

#******************************************************************************
#  distclean
#------------------------------------------------------------------------------

distclean-local:
	-rm -rf $(testsubdir)

#******************************************************************************
# Makefile.am (sequencer24)
#------------------------------------------------------------------------------
# Local Variables:
# End:
#------------------------------------------------------------------------------
# 	vim: ts=3 sw=3 ft=automake
#------------------------------------------------------------------------------

