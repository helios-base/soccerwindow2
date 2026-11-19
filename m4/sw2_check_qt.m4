# ===========================================================================
#   sw2_check_qt.m4 -- Qt5/Qt6 detection for soccerwindow2
# ===========================================================================
#
# SYNOPSIS
#
#   SW2_CHECK_QT(QT5-MIN-VERSION, QT6-MIN-VERSION, QT-MODULES)
#
#   e.g. SW2_CHECK_QT([5.15.0], [6.2.0], [core gui widgets network])
#
# DESCRIPTION
#
#   Searches qmake for Qt5 or Qt6 and queries the compiler/linker flags of
#   the given Qt modules.  The following configure option and variables are
#   available:
#
#     --with-qt=auto|5|6   Qt major version to use (default: auto).
#                          auto prefers Qt6 and falls back to Qt5.
#     QMAKE                The qmake command to use.  If set, the search of
#                          qmake is skipped.
#     QT_MOC               The moc command to use.  If set, the search of moc
#                          is skipped.
#
#   qmake is searched in the following order:
#
#     auto: qmake6 qmake-qt6 qmake qmake-qt5
#     6:    qmake6 qmake-qt6 qmake
#     5:    qmake-qt5 qmake
#
#   The first qmake whose Qt version satisfies the requested major version
#   and the minimum version is used.  moc is searched in the directories
#   reported by "qmake -query QT_HOST_LIBEXECS" (Qt6) and
#   "qmake -query QT_HOST_BINS" (Qt5), and then in $PATH.
#
#   The following shell variable is set to "yes" if Qt is found:
#
#     have_qt
#
#   The following variables are substituted:
#
#     QT_CXXFLAGS  QT_LIBS  QT_MOC  QMAKE  QT_VERSION_STRING
#
#   After the variables have been set, a trial compile and link is
#   performed to check the correct functioning of the meta object compiler.
#   If Qt is not found or the check fails, configure stops with an error.
#
# LICENSE
#
#   This file is based on ax_have_qt.m4 (serial 25) from the GNU Autoconf
#   Archive, and modified for soccerwindow2 to support Qt6.
#
#   Copyright (c) 2008 Bastiaan Veelo <Bastiaan@Veelo.net>
#   Copyright (c) 2014 Alex Henrie <alexhenrie24@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

# _SW2_QT_VERSION_NUMBER(VERSION)
# Print a comparable number (major * 10000 + minor * 100 + patch).
AC_DEFUN([_SW2_QT_VERSION_NUMBER],
[`echo "$1" | awk -F. '{ printf "%d", ([$]1 * 10000) + ([$]2 * 100) + [$]3 }'`])

AC_DEFUN([SW2_CHECK_QT],
[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH([qt],
    [AS_HELP_STRING([--with-qt=VERSION],
                    [Qt major version to use: auto, 5 or 6 @<:@default=auto@:>@])],
    [sw2_qt_request="$withval"],
    [sw2_qt_request=auto])
  AC_ARG_VAR([QMAKE], [qmake command of the Qt to use])
  AC_ARG_VAR([QT_MOC], [moc command of the Qt to use])

  case "$sw2_qt_request" in
    auto|yes)
      sw2_qt_request=auto
      sw2_qmake_candidates="qmake6 qmake-qt6 qmake qmake-qt5" ;;
    6)
      sw2_qmake_candidates="qmake6 qmake-qt6 qmake" ;;
    5)
      sw2_qmake_candidates="qmake-qt5 qmake" ;;
    no)
      AC_MSG_ERROR([Qt is required to build soccerwindow2. --without-qt is not supported.]) ;;
    *)
      AC_MSG_ERROR([invalid value for --with-qt: $sw2_qt_request (expected auto, 5 or 6)]) ;;
  esac

  sw2_qt5_min_number=_SW2_QT_VERSION_NUMBER([$1])
  sw2_qt6_min_number=_SW2_QT_VERSION_NUMBER([$2])

  # sw2_qt_check_qmake QMAKE_COMMAND
  #   Set sw2_qt_version and return 0 if the Qt of the given qmake satisfies
  #   the requested major version and the minimum version.
  sw2_qt_check_qmake()
  {
    sw2_qt_version=`$[1] -query QT_VERSION 2>/dev/null`
    test -n "$sw2_qt_version" || return 1
    sw2_qt_major=`echo "$sw2_qt_version" | cut -d. -f1`
    sw2_qt_number=_SW2_QT_VERSION_NUMBER([$sw2_qt_version])
    echo "$[1]: Qt $sw2_qt_version" >&AS_MESSAGE_LOG_FD
    case "$sw2_qt_request:$sw2_qt_major" in
      auto:5|5:5) test "$sw2_qt_number" -ge "$sw2_qt5_min_number" ;;
      auto:6|6:6) test "$sw2_qt_number" -ge "$sw2_qt6_min_number" ;;
      *) return 1 ;;
    esac
  }

  have_qt=no
  AC_MSG_CHECKING([for qmake of Qt ($sw2_qt_request)])
  if test -n "$QMAKE"; then
    if sw2_qt_check_qmake "$QMAKE"; then
      have_qt=yes
    else
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([QMAKE=$QMAKE does not provide the requested Qt (--with-qt=$sw2_qt_request, Qt5 >= $1 or Qt6 >= $2).])
    fi
  else
    for sw2_qmake in $sw2_qmake_candidates; do
      AS_IF([command -v "$sw2_qmake" >/dev/null 2>&1], [
        if sw2_qt_check_qmake "$sw2_qmake"; then
          QMAKE=`command -v "$sw2_qmake"`
          have_qt=yes
          break
        fi
      ])
    done
  fi
  if test "x$have_qt" != xyes; then
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([Qt not found. soccerwindow2 requires Qt5 >= $1 or Qt6 >= $2.
                  Install the Qt development package, or specify the qmake command by QMAKE=/path/to/qmake.])
  fi
  QT_VERSION_STRING="$sw2_qt_version"
  AC_MSG_RESULT([$QMAKE (Qt $QT_VERSION_STRING)])

  # Look for moc in the directories reported by qmake, then in $PATH.
  sw2_qt_tool_path=""
  for sw2_qt_query in QT_HOST_LIBEXECS QT_HOST_BINS; do
    sw2_qt_tool_dir=`$QMAKE -query $sw2_qt_query 2>/dev/null`
    if test -n "$sw2_qt_tool_dir" && test -d "$sw2_qt_tool_dir"; then
      sw2_qt_tool_path="$sw2_qt_tool_path$sw2_qt_tool_dir$PATH_SEPARATOR"
    fi
  done
  AC_PATH_PROGS([QT_MOC], [moc moc-qt$sw2_qt_major], [], [$sw2_qt_tool_path$PATH])
  if test -z "$QT_MOC"; then
    AC_MSG_ERROR([moc of Qt $QT_VERSION_STRING not found. Specify it by QT_MOC=/path/to/moc.])
  fi

  # Query the compiler and linker flags from qmake.
  AC_MSG_CHECKING([for the compiler and linker flags of Qt])
  sw2_qt_dir=`mktemp -d`
  cat > "$sw2_qt_dir/sw2_qt.pro" << EOF
QT = $3
CONFIG += release
percent.target = %
percent.commands = @echo -n "\$(\$(@))\ "
QMAKE_EXTRA_TARGETS += percent
EOF
  (cd "$sw2_qt_dir" && $QMAKE sw2_qt.pro -o Makefile) >&AS_MESSAGE_LOG_FD 2>&1
  QT_CXXFLAGS=`cd "$sw2_qt_dir" && ${MAKE-make} -s -f Makefile CXXFLAGS INCPATH 2>&AS_MESSAGE_LOG_FD`
  QT_LIBS=`cd "$sw2_qt_dir" && ${MAKE-make} -s -f Makefile LIBS 2>&AS_MESSAGE_LOG_FD`
  rm -rf "$sw2_qt_dir"
  if test -z "$QT_LIBS"; then
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([Failed to get the flags of the Qt modules ($3) from $QMAKE.])
  fi
  AC_MSG_RESULT([yes])
  echo "QT_CXXFLAGS=$QT_CXXFLAGS" >&AS_MESSAGE_LOG_FD
  echo "QT_LIBS=$QT_LIBS" >&AS_MESSAGE_LOG_FD

  AC_SUBST([QT_CXXFLAGS])
  AC_SUBST([QT_LIBS])
  AC_SUBST([QT_MOC])
  AC_SUBST([QMAKE])
  AC_SUBST([QT_VERSION_STRING])

  # Check the correct functioning of the Qt installation.
  AC_MSG_CHECKING([correct functioning of Qt installation])
  cat > sw2_qt_test.h << EOF
#include <QObject>
class Test : public QObject
{
Q_OBJECT
public:
  Test() {}
  ~Test() {}
public slots:
  void receive() {}
signals:
  void send();
};
EOF

  cat > sw2_qt_main.$ac_ext << EOF
#include "sw2_qt_test.h"
#include <QApplication>
int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  Test t;
  QObject::connect( &t, SIGNAL(send()), &t, SLOT(receive()) );
}
EOF

  sw2_qt_test_result=failure
  sw2_try_1="$QT_MOC sw2_qt_test.h -o moc_sw2_qt_test.$ac_ext >/dev/null 2>/dev/null"
  AC_TRY_EVAL(sw2_try_1)
  if test x"$ac_status" != x0; then
    echo "configure: could not run $QT_MOC on:" >&AS_MESSAGE_LOG_FD
    cat sw2_qt_test.h >&AS_MESSAGE_LOG_FD
  else
    sw2_try_2="$CXX $QT_CXXFLAGS -c $CXXFLAGS -o moc_sw2_qt_test.o moc_sw2_qt_test.$ac_ext >/dev/null 2>/dev/null"
    AC_TRY_EVAL(sw2_try_2)
    if test x"$ac_status" != x0; then
      echo "configure: could not compile:" >&AS_MESSAGE_LOG_FD
      cat moc_sw2_qt_test.$ac_ext >&AS_MESSAGE_LOG_FD
    else
      sw2_try_3="$CXX $QT_CXXFLAGS -c $CXXFLAGS -o sw2_qt_main.o sw2_qt_main.$ac_ext >/dev/null 2>/dev/null"
      AC_TRY_EVAL(sw2_try_3)
      if test x"$ac_status" != x0; then
        echo "configure: could not compile:" >&AS_MESSAGE_LOG_FD
        cat sw2_qt_main.$ac_ext >&AS_MESSAGE_LOG_FD
      else
        sw2_try_4="$CXX -o sw2_qt_main sw2_qt_main.o moc_sw2_qt_test.o $QT_LIBS $LIBS >/dev/null 2>/dev/null"
        AC_TRY_EVAL(sw2_try_4)
        if test x"$ac_status" = x0; then
          sw2_qt_test_result=success
        fi
      fi
    fi
  fi
  rm -f sw2_qt_test.h moc_sw2_qt_test.$ac_ext moc_sw2_qt_test.o \
        sw2_qt_main.$ac_ext sw2_qt_main.o sw2_qt_main
  AC_MSG_RESULT([$sw2_qt_test_result])
  if test x"$sw2_qt_test_result" != xsuccess; then
    AC_MSG_ERROR([Failed to build a test program with Qt $QT_VERSION_STRING.
                  The detected qmake ($QMAKE) and moc ($QT_MOC) may belong to
                  different Qt installations. See config.log for details.])
  fi
])
