#/usr/bin/sed
#
# This file fixes the banner at the top of each header file.  Makes the
# header look a lot nicer, in my opinion.

10,20 s/^\/\/------------*/ *\//
1,20 s/^\/\/------------*/\/*/
1,20 s/^\/\// */

