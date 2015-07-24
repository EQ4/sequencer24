#*******************************************************************************
# Makefile.am (m4)
#-------------------------------------------------------------------------------
##
# \file       	Makefile.am
# \library    	sequencer24
# \author     	Chris Ahlstrom
# \date       	2015-07-24
# \update      2015-07-24
# \version    	$Revision$
# \license    	$XPC_SUITE_GPL_LICENSE$
#
#     This module provides an Automake makefile for sequencer24's
#     main 'm4' directory.
#
#     This makefile provides mostly just a way to make sure the m4 files are
#     included in the 'dist' target.
#
#-------------------------------------------------------------------------------

AUTOMAKE_OPTIONS = foreign dist-zip dist-bzip2
MAINTAINERCLEANFILES = Makefile.in Makefile

PACKAGE = @PACKAGE@
VERSION = @VERSION@
API_VERSION = @API_VERSION@
datarootdir = @datarootdir@
xpcincludedir=@xpcincludedir@

#*******************************************************************************
# EXTRA_DIST
#-------------------------------------------------------------------------------

EXTRA_DIST =

#*******************************************************************************
# dist
#-------------------------------------------------------------------------------

dist_man_MANS = sequencer24.1

#******************************************************************************
# Makefile.am (m4)
# 	vim: ts=3 sw=3 ft=automake
#------------------------------------------------------------------------------
