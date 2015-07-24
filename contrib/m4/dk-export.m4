## Copyright (c) 2004-2007  Daniel Elstner  <daniel.kitta@gmail.com>
##
## This file is part of danielk's Autostuff.
##
## danielk's Autostuff is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2 of the License, or (at
## your option) any later version.
##
## danielk's Autostuff is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with danielk's Autostuff; if not, write to the Free Software Foundation,
## Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#serial 20070113

## DK_LINK_VERSION_SCRIPT(variable, filename)
##
## Check whether the linker accepts the --version-script flag.  On success,
## set the output <variable> to "-Wl,--version-script=$srcdir/<filename>".
## The <filename> should be a path relative to the top source directory.
##
AC_DEFUN([DK_LINK_VERSION_SCRIPT],
[dnl
m4_if([$2],, [AC_FATAL([2 arguments expected])])[]dnl
dnl
AC_CACHE_CHECK([whether the linker accepts --version-script],
               [dk_cv_link_version_script],
[
  DK_SH_VAR_PUSH([LDFLAGS], ["$LDFLAGS -Wl,--version-script=$srcdir/$2"])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
                 [dk_cv_link_version_script=yes],
                 [dk_cv_link_version_script=no])
  DK_SH_VAR_POP([LDFLAGS])
])
AS_IF([test "x$dk_cv_link_version_script" = xyes],
      [$1='-Wl,--version-script=$(top_srcdir)/'"$2"],
      [$1=])
AC_SUBST([$1])[]dnl
])
