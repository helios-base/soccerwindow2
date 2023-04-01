// -*-c++-*-

/*!
  \file formation_editor_window.cpp
  \brief formation editor window class Source File.
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

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "formation_editor_window.h"

#include <iostream>

#include "xpm/fedit2.xpm"
// #include "xpm/chase.xpm"
// #include "xpm/delete.xpm"
// #include "xpm/insert.xpm"
// #include "xpm/new.xpm"
// #include "xpm/open.xpm"
// #include "xpm/record.xpm"
// #include "xpm/replace.xpm"
// #include "xpm/reverse.xpm"
// #include "xpm/save.xpm"
// #include "xpm/symmetry.xpm"
// #include "xpm/train.xpm"

// using namespace rcsc;

/*-------------------------------------------------------------------*/
FormationEditorWindow::FormationEditorWindow( QWidget * parent )
    : QMainWindow( parent )
{
    this->setWindowIcon( QIcon( QPixmap( fedit2_xpm ) ) );
    this->setWindowTitle( tr( "Formation Editor" ) );
    this->setMinimumSize( 280, 220 );
    this->resize( 640, 480 );
}

/*-------------------------------------------------------------------*/
FormationEditorWindow::~FormationEditorWindow()
{
    std::cerr << "delete FormationEditorWindow" << std::endl;
}

/*-------------------------------------------------------------------*/
void
FormationEditorWindow::setEditData( std::shared_ptr< FormationEditData > data )
{
    M_edit_data = data;
}
