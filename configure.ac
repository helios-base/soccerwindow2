#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([soccerwindow2], [20160613], [akky@users.sourceforge.jp])

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
# check Qt4
AC_ARG_ENABLE(qt4,
              AC_HELP_STRING([--enable-qt4],
                             [use Qt4 GUI tool kit (default yes)]))
#if test "x$enable_qt4" == "xyes"; then
if test "x$enable_qt4" != "xno"; then
  AC_MSG_NOTICE(enabled Qt4)

  QT4MODULES="QtCore QtGui QtNetwork"

  AC_ARG_ENABLE(gl,
                AC_HELP_STRING([--enable-gl],
                               [enable GLWidget for Qt4 (default yes)]))
  if test "x$enable_gl" == "xyes"; then
    AC_MSG_NOTICE(enabled GLWidget)
    CFLAGS="-DUSE_GLWIDGET $CFLAGS"
    CXXFLAGS="-DUSE_GLWIDGET $CXXFLAGS"
    QT4MODULES="$QT4MODULES QtOpenGL"
  fi

  AX_QT4([4.3.0],[$QT4MODULES])

  if test x$have_qt4 != xyes ; then
    AC_MSG_ERROR([$QT4MODULES could not be found.])
  fi

  AM_CONDITIONAL(BUILD_QT4, [test "1" = "1"])
else
  AM_CONDITIONAL(BUILD_QT4, [test "1" = "0"])
fi

# ----------------------------------------------------------
# check wxWidgets
#AC_ARG_ENABLE(wx,
#              AC_HELP_STRING([--enable-wx],
#                             [use wxGTK-2.6.1 or later (default no)]))
#if test "x$enable_wx" == "xyes"; then
#  AM_OPTIONS_WXCONFIG
#  AM_PATH_WXCONFIG(2.6.1, have_wx="yes")
#  if test "x$have_wx" != "xyes"; then
#    AC_MSG_ERROR([
#  wxGTK must be installed on your system
#  but wx-config script couldn't be found.
#
#  Please check that wx-config is in path, the directory
#  where wxGTK libraries are installed (returned by
#  'wx-config --libs' command) is in LD_LIBRARY_PATH or
#  equivalent variable and wxGTK version is 2.6.1 or later.
#    ])
#  fi
#  AM_CONDITIONAL(BUILD_WX, [test "1" = "1"])
#else
#  AM_CONDITIONAL(BUILD_WX, [test "1" = "0"])
#fi

##################################################
# check C++
##################################################

AX_CXX_COMPILE_STDCXX_14(noext)

# ----------------------------------------------------------
# check boost
AX_BOOST_BASE([1.38.0])
AX_BOOST_SYSTEM
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
LIBS="$LIBS $BOOST_SYSTEM_LIB"

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
  for librcsc_path_tmp in $HOME $HOME/local $HOME/rcss /opt/robocup /opt /opt/local ; do
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

AC_CHECK_LIB([rcsc_geom], [main],
             [LIBS="-lrcsc_geom $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_geom not found! ***])])
AC_CHECK_LIB([rcsc_rcg], [main],
             [LIBS="-lrcsc_rcg $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_rcg not found! ***])])
# defined HAVE_LIBRCSC_GZ and set LIBS="-lrcsc_gz $LIBS"
AC_CHECK_LIB([rcsc_gz], [main])
AC_CHECK_LIB([rcsc_param], [main],
             [LIBS="-lrcsc_param $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_param not found! ***])])
AC_CHECK_LIB([rcsc_time], [main],
             [LIBS="-lrcsc_time $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_time not found! ***])])
AC_CHECK_LIB([rcsc_color], [main],
             [LIBS="-lrcsc_color $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_color not found! ***])])
AC_CHECK_LIB([rcsc_net], [main],
             [LIBS="-lrcsc_net $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_time not found! ***])])
AC_CHECK_LIB([rcsc_ann], [main],
             [LIBS="-lrcsc_ann $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_ann not found! ***])])
AC_CHECK_LIB([rcsc_clang], [main],
             [LIBS="-lrcsc_clang $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_clang not found! ***])])
AC_CHECK_LIB([rcsc_agent], [main],
             [LIBS="-lrcsc_agent $LIBS"],
             [AC_MSG_ERROR([*** -lrcsc_agent not found! ***])])


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
                 src/qt4/Makefile],
                 [test -f src/sswindow2 && chmod +x src/sswindow2])
AC_OUTPUT
