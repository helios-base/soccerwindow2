// -*-c++-*-

/*!
  \file main.cpp
  \brief main source file.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QApplication>

#include "export_batch.h"
#include "main_window.h"
#include "options.h"

#include <iostream>

int
main( int argc, char ** argv )
{
    std::cout << "******************************************************************\n"
              << " " PACKAGE_NAME " " VERSION "\n"
              << " Copyright: (C) 2005 - 2012. Hidehisa Akiyama\n"
              << " 2011- Hidehisa Akiyama and Hiroki Shimora\n"
              << " All rights reserved.\n"
              << "******************************************************************\n"
              << std::endl;

    // The export from the command line shows no window (docs/plan/2026-11-28-00_frame-export.md
    // 4.5, S14): on the platform offscreen (whatever QT_QPA_PLATFORM the
    // desktop sets; -platform of Qt still chooses another), it runs without a
    // display too.
    const bool batch = ExportBatch::isAsked( argc, argv );
    if ( batch )
    {
        qputenv( "QT_QPA_PLATFORM", "offscreen" );
    }

    QApplication app( argc, argv );

    // the name of the desktop entry (soccerwindow2.desktop), with which the
    // desktop environment associates the windows (the app_id on Wayland)
    QGuiApplication::setDesktopFileName( QStringLiteral( "soccerwindow2" ) );

    if ( batch )
    {
        // no window: the settings of the window are not read into the
        // options, and nothing is written in ~/.soccerwindow2
        if ( ! Options::instance().parseCmdLine( argc, argv ) )
        {
            return 1;
        }

        ExportBatch::Settings settings;
        QString error;
        if ( ! ExportBatch::readOptions( Options::instance(), &settings, &error ) )
        {
            std::cerr << PACKAGE_NAME << ": " << error.toStdString() << std::endl;
            return 1;
        }

        ExportBatch export_batch( settings, std::cerr );
        return export_batch.run( QString::fromLocal8Bit( Options::instance().gameLogFilePath().c_str() ) );
    }

    MainWindow win;

    if ( ! Options::instance().parseCmdLine( argc, argv ) )
    {
        return 1;
    }

    win.show();
    win.init();

    return app.exec();
}
