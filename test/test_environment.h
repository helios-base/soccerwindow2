// -*-c++-*-

/*!
  \file test_environment.h
  \brief the environment of the GUI tests: a temporary home directory and
  the offscreen platform.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef SOCCERWINDOW2_TEST_ENVIRONMENT_H
#define SOCCERWINDOW2_TEST_ENVIRONMENT_H

#include <QtGlobal>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>

/*!
  \brief set the environment of a GUI test, before QApplication is created:
  HOME, XDG_CONFIG_HOME and XDG_RUNTIME_DIR in a temporary directory, so
  that the user's ~/.soccerwindow2 is neither read nor modified (MainWindow
  and the export read and write it), and the offscreen platform, even if
  QT_QPA_PLATFORM is set by the desktop session (e.g. "xcb"), so that no
  window is shown. The -platform command line option (e.g. -platform xcb)
  still overrides it.
  \param tmp_dir the temporary directory, kept while the test runs
  \return false if the directories could not be made
*/
inline
bool
set_test_environment( const QTemporaryDir & tmp_dir )
{
    if ( ! tmp_dir.isValid() )
    {
        qCritical( "failed to create a temporary directory" );
        return false;
    }

    const QString home = tmp_dir.path() + QLatin1String( "/home" );
    const QString runtime = tmp_dir.path() + QLatin1String( "/runtime" );
    if ( ! QDir().mkpath( home )
         || ! QDir().mkpath( runtime )
         || ! QFile::setPermissions( runtime, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner ) )
    {
        qCritical( "failed to set up the temporary directories" );
        return false;
    }

    qputenv( "HOME", QFile::encodeName( home ) );
    qputenv( "XDG_CONFIG_HOME", QFile::encodeName( home + QLatin1String( "/.config" ) ) );
    qputenv( "XDG_RUNTIME_DIR", QFile::encodeName( runtime ) );
    qputenv( "QT_QPA_PLATFORM", "offscreen" );
    return true;
}

/*!
  \brief the main function of a GUI test of the class TestClass, in the
  environment of set_test_environment()
*/
#define SOCCERWINDOW2_GUI_TEST_MAIN( TestClass ) \
    int \
    main( int argc, char ** argv ) \
    { \
        QTemporaryDir tmp_dir; \
        if ( ! set_test_environment( tmp_dir ) ) \
        { \
            return 1; \
        } \
        QApplication app( argc, argv ); \
        TestClass test; \
        return QTest::qExec( &test, argc, argv ); \
    }

#endif
