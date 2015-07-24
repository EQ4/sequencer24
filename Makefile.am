#*****************************************************************************
# Makefile.am (sequencer24)
#-----------------------------------------------------------------------------
##
# \file        Makefile.am
# \library     sequencer24
# \author      Chris Ahlstrom
# \date        2015-07-24
# \updates     2015-07-24
# \version     $Revision$
# \license     $XPC_SUITE_GPL_LICENSE$
#
#     This is the top-level project makefile for the sequencer24 project.
#
#     This makefile provides the skeleton needed to build the library
#     and application directories using GNU autotools.
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Packing targets
#-----------------------------------------------------------------------------

AUTOMAKE_OPTIONS = foreign dist-zip dist-bzip2
MAINTAINERCLEANFILES = Makefile.in Makefile $(AUX_DIST)
MOSTLYCLEANFILES = *~

#*****************************************************************************
# Extra files in the top-level directory
#-----------------------------------------------------------------------------

EXTRA_DIST = bootstrap pack README VERSION COPYING RTC AUTHORS INSTALL SEQ24 NEWS ChangeLog seq24usr.example

#*****************************************************************************
# Packaging
#-----------------------------------------------------------------------------
#
#     This section is recommended for the top-level Makefile.am by the
#     reference.
#
#-----------------------------------------------------------------------------

PACKAGE = @PACKAGE@
VERSION = @VERSION@

MIDICVT_API_MAJOR = @MIDICVT_API_MAJOR@
MIDICVT_API_MINOR = @MIDICVT_API_MINOR@
MIDICVT_API_PATCH = @MIDICVT_API_PATCH@
MIDICVT_API_VERSION = @MIDICVT_API_VERSION@

MIDICVT_LT_CURRENT = @MIDICVT_LT_CURRENT@
MIDICVT_LT_REVISION = @MIDICVT_LT_REVISION@
MIDICVT_LT_AGE = @MIDICVT_LT_AGE@

#*****************************************************************************
# Installed directories
#-----------------------------------------------------------------------------
#
# The 'libdir' define is necessary to cause the proper subdirectory to
# be made during installation.  'sequencer24libdir' is defined in the
# configure.ac script.  The 'libdir' define is needed for work with
# libtool.  Not sure about 'pkglibdr'.
#
#     pkglibdir=$(sequencer24libdir)
#
#	Directories and macros:
#
#		prefix                = /usr/local
#		libdir                = /usr/local/lib/sequencer24
#		datadir               = /usr/local/share
#		datarootdir           = /usr/local/share
#		sequencer24libdir     = /usr/local/lib/sequencer24
#		sequencer24docdir     = /usr/local/share/doc/sequencer24-1.1
#		sequencer24doxygendir = doc
#		sequencer24includedir = /usr/local/include/sequencer24-1.1
#		localedir     = /usr/local/share/locale
#
#  'localedir' is the normal system directory for installed localization
#  files.
#
#-----------------------------------------------------------------------------

prefix = @prefix@
libdir = @sequencer24libdir@
datadir = @datadir@
datarootdir = @datarootdir@
sequencer24docdir = @sequencer24docdir@
sequencer24doxygendir = @sequencer24doxygendir@
sequencer24includedir = @sequencer24includedir@
sequencer24libdir = @sequencer24libdir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

#******************************************************************************
# Local project directories
#------------------------------------------------------------------------------
#
#		top_srcdir    = ../../.. [XPC Basic directory]
#		builddir      = /home/ahlstrom/ca/mls/git/XPC Basic-1.1/debug
#
#------------------------------------------------------------------------------

top_srcdir = @top_srcdir@
builddir = @abs_top_builddir@

#******************************************************************************
# aclocal support
#------------------------------------------------------------------------------

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

#*****************************************************************************
# libtool
#-----------------------------------------------------------------------------
#	${SHELL} $(top_srcdir)/config.status --recheck
#-----------------------------------------------------------------------------

LIBTOOL_DEPS = @LIBTOOL_DEPS@

libtool: $(LIBTOOL_DEPS)
	$(top_srcdir)/config.status --recheck

#*****************************************************************************
# SUBDIRS
#-----------------------------------------------------------------------------

SUBDIRS = man src

#*****************************************************************************
# DIST_SUBDIRS
#-----------------------------------------------------------------------------

DIST_SUBDIRS = $(SUBDIRS)

#*****************************************************************************
# all-local
#-----------------------------------------------------------------------------

all-local:
	@echo "Top source-directory 'top_srcdir' is $(top_srcdir)"
	@echo "* * * * * All build items completed * * * * *"

#******************************************************************************
# Debugging targets
#------------------------------------------------------------------------------

show:
	@echo "Install directories:"
	@echo
	@echo "prefix           = $(prefix)"
	@echo "datadir          = $(datadir)"
	@echo "datarootdir      = $(datarootdir)"
	@echo "libdir           = $(libdir)"
	@echo "sequencer24libdir      = $(sequencer24libdir)"
	@echo "sequencer24docdir      = $(sequencer24docdir)"
	@echo "sequencer24doxygendir  = $(sequencer24doxygendir)"
	@echo "sequencer24includedir  = $(sequencer24includedir)"
	@echo "localedir        = $(localedir)"
	@echo
	@echo "Local directories:"
	@echo
	@echo "top_srcdir       = $(top_srcdir) [project root directory]"
	@echo "srcdir           = $(srcdir)"
	@echo "top_builddir     = $(top_builddir)"
	@echo "builddir         = $(builddir)"

showver:
	@echo "PACKAGE            = $(PACKAGE)"
	@echo "VERSION            = $(VERSION)"
	@echo "MIDICVT_API_MAJOR    = $(MIDICVT_API_MAJOR)"
	@echo "MIDICVT_API_MINOR    = $(MIDICVT_API_MINOR)"
	@echo "MIDICVT_API_PATCH    = $(MIDICVT_API_PATCH)"
	@echo "MIDICVT_API_VERSION  = $(MIDICVT_API_VERSION)"
	@echo "MIDICVT_LT_CURRENT   = $(MIDICVT_LT_CURRENT)"
	@echo "MIDICVT_LT_REVISION  = $(MIDICVT_LT_REVISION)"
	@echo "MIDICVT_LT_AGE       = $(MIDICVT_LT_AGE)"

#****************************************************************************
# Makefile.am (sequencer24 top-level)
#----------------------------------------------------------------------------
# vim: ts=3 sw=3 ft=automake
#----------------------------------------------------------------------------
