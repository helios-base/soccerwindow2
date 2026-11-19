#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([soccerwindow2], [2026], [akky@users.sourceforge.jp])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

# ----------------------------------------------------------
#AM_INIT_AUTOMAKE([gnu check-news])
AM_INIT_AUTOMAKE([gnu])

# it is necessary to check c++ header files.
AC_LANG([C++])

# ----------------------------------------------------------
# Checks for programs.
#AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL

# ----------------------------------------------------------
# Checks for configure options
#AC_ARG_ENABLE(static,
#              [  --enable-static         statically linked],
#              [ enable_static=$enableval ],
#              [ enable_static=no ])
#if test "$enable_static" == "yes"; then
#  CFLAGS="-static $CFLAGS"
#  CXXFLAGS="-static $CXXFLAGS"
#fi

# ----------------------------------------------------------
# check C++
# Qt6 requires C++17, so check it before Qt.

AX_CXX_COMPILE_STDCXX_17(noext)

# ----------------------------------------------------------
# check Qt (Qt5 >= 5.15 or Qt6 >= 6.2)

SW2_CHECK_QT([5.15.0], [6.2.0], [core gui widgets network])
AM_CONDITIONAL([BUILD_QT], [test "x$have_qt" = "xyes"])

# ----------------------------------------------------------
# check boost (header-only)
AX_BOOST_BASE([1.41.0])
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"

# check boost library for wx
#if test "x$enable_wx" == "xyes"; then
#  AX_BOOST_SIGNALS
#fi

# check librcsc path
AC_SUBST(LIBRCSCLIB)
LIBRCSCLIB="/usr/local/lib"
librcsc_prefix=""
AC_ARG_WITH(librcsc,
            [  --with-librcsc=PREFIX prefix where librcsc is installed (optional)],
            librcsc_prefix="$withval",
            librcsc_prefix="")
if test x$librcsc_prefix != x; then
  CPPFLAGS="$CPPFLAGS -I$librcsc_prefix/include"
  LDFLAGS="$LDFLAGS -L$librcsc_prefix/lib"
  LIBRCSCLIB="$librcsc_prefix/lib"
else
  for librcsc_path_tmp in $HOME/local $HOME/.local /opt/local /usr/local /usr; do
    if test -d "$librcsc_path_tmp/include/rcsc" && test -r "$librcsc_path_tmp/include/rcsc/types.h" ; then
      CPPFLAGS="$CPPFLAGS -I$librcsc_path_tmp/include"
      LDFLAGS="$LDFLAGS -L$librcsc_path_tmp/lib"
      LIBRCSCLIB="$librcsc_path_tmp/lib"
      break;
    fi
  done
fi

# ----------------------------------------------------------
# Checks for libraries.
AC_CHECK_LIB([m], [cos],
             [LIBS="-lm $LIBS"],
             [AC_MSG_ERROR([*** -lm not found! ***])])
AC_CHECK_LIB([z], [deflate])

AC_CHECK_LIB([rcsc], [main],
             [LIBS="-lrcsc $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc not found! ***])])

# ----------------------------------------------------------
# Checks for header files.
AC_CHECK_HEADERS([netinet/in.h],
                 break,
                 [AC_MSG_ERROR([*** netinet/in.h not found ***])])

# ----------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_STRUCT_TM

# ----------------------------------------------------------
# Checks for library functions.
AC_HEADER_STDC
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
#AC_CHECK_FUNCS([atexit],
#               break,
#               [AC_MSG_ERROR([*** atexit function not found!])])
AC_CHECK_FUNCS([floor memset pow rint sqrt strtol])
AC_CHECK_FUNC([rint],
              break,
              [AC_CHECK_LIB(m, rint, break,
                            [AC_MSG_ERROR([*** rint function not found!])]
                           )
              ]
            )

# ----------------------------------------------------------
#CFLAGS="$CFLAGS $SDL_CFLAGS"
#CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
#LIBS="$LIBS $SDL_LIBS"

# ----------------------------------------------------------
AC_CONFIG_FILES([Doxyfile
                 soccerwindow2.spec
                 Makefile
                 src/sswindow2
                 src/Makefile
                 src/model/Makefile
                 src/qt/Makefile],
                 [test -f src/sswindow2 && chmod +x src/sswindow2])
AC_OUTPUT
