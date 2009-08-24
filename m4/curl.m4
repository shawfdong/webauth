dnl curl.m4 -- Find the cURL libraries.
dnl
dnl Defines the macro WEBAUTH_LIB_CURL, which probes for the cURL libraries
dnl and defines the output variables CURL_CPPFLAGS and CURL_LIBS to the
dnl appropriate preprocessor and linker flags.
dnl
dnl Written by Russ Allbery <rra@stanford.edu>
dnl Copyright 2002, 2003, 2004, 2006, 2009
dnl     Board of Trustees, Leland Stanford Jr. University
dnl
dnl See LICENSE for licensing terms.

AC_DEFUN([WEBAUTH_LIB_CURL],
[AC_REQUIRE([AC_CANONICAL_HOST])
AC_ARG_WITH([curl],
    AC_HELP_STRING([--with-curl=PATH], [Path to cURL install]),
    [if test x"$withval" != xno && test x"$withval" != xyes ; then
        CURL_LDFLAGS=-L$withval/lib
        CURL_CPPFLAGS=-I$withval/include
     fi])
WEBAUTH_LDFLAGS_save=$LDFLAGS
LDFLAGS="$LDFLAGS $CURL_LDFLAGS"
CURL_LIBS=
if test x"$reduced_depends" != xtrue ; then
    AC_CHECK_LIB([z], [inflate], [CURL_LIBS=-lz])
fi
AC_CHECK_LIB([curl], [curl_easy_init], [CURL_LIBS=-lcurl], ,
    [$CURL_LIBS $SSL_LIBS])
LDFLAGS=$WEBAUTH_LDFLAGS_save
CURL_LIBS=`echo "$CURL_LDFLAGS $CURL_LIBS" | sed 's/^  *//'`
AC_SUBST(CURL_LIBS)
AC_SUBST(CURL_CPPFLAGS)])