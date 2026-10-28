AC_COPYRIGHT([
 Copyright (c) 2025-2026 - Adjacent Link LLC, Bridgewater, New Jersey
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of Adjacent Link LLC nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 See toplevel COPYING for more information.
])

AC_INIT([simactivityservice],[0.0.3],[emane at adjacent link dot com])

AC_CONFIG_SRCDIR([src])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign subdir-objects])

: ${CXXFLAGS="-g -O3 -Wall -Wextra"}

AC_PROG_CXX

AC_PROG_CXXCPP

AX_CXX_COMPILE_STDCXX_17([noext],[mandatory])

AC_PROG_INSTALL

LT_INIT

AC_LANG([C++])

AC_ARG_WITH(profile,
           [  --with-profile          add profiling support (off)])

AC_ARG_WITH(debug,
           [  --with-debug            add debug support (off)])

AC_ARG_WITH(dot,
           [  --with-dot              Use dot with doxygen (off)])

PKG_CHECK_MODULES([libxml2], libxml-2.0)
PKG_CHECK_MODULES([protobuf], protobuf)
PKG_CHECK_MODULES([libuuid], uuid)
PKG_CHECK_MODULES([libzmq], libzmq)
PKG_CHECK_MODULES([libostatistic],[libostatistic])

LDFLAGS="$LDFLAGS -lpthread -ldl"

# options for use with gprof
if test "$with_profile" = "yes"
then
CXXFLAGS="$CXXFLAGS -g -pg"
LDFLAGS="$LDFLAGS -g -pg"
fi

# options for use with debug
if test "$with_debug" = "yes"
then
AX_CHECK_COMPILE_FLAG([-Og],
 [CXXFLAGS="$CXXFLAGS -g -Og"],
 [CXXFLAGS="$CXXFLAGS -g -O0"])
AX_CHECK_LINK_FLAG([-Og],
 [LDFLAGS="$LDFLAGS -g -Og"],
 [LDFLAGS="$LDFLAGS -g -O0"])
fi

AC_SUBST(simactivityservice_CFLAGS,
["${libxml2_CFLAGS} ${protobuf_CFLAGS} ${libzmq_CFLAGS} ${libuuid_CFLAGS} ${libostatistic_CFLAGS}"])

AC_SUBST(simactivityservice_LIBS,["${libxml2_LIBS} ${protobuf_LIBS} ${libzmq_LIBS} ${libuuid_LIBS} ${libostatistic_LIBS}"])

LANG=C
AC_SUBST(LANG)

AM_PATH_PYTHON([3.6])

# check for rpmbuild
AC_CHECK_PROG(HAVE_RPMBUILD, rpmbuild, true, false)
AM_CONDITIONAL(HAVE_RPMBUILD,$HAVE_RPMBUILD)

AC_CONFIG_FILES([Makefile
 include/Makefile
 include/simactivityservice/Makefile
 include/simactivityservice/proto/Makefile
 src/Makefile
 src/simactivityservice/Makefile
 src/python/Makefile
 simactivityservice.pc
 ])
AC_OUTPUT
