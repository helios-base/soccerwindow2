// -*-c++-*-

/*!
  \file label_editor_window.h
  \brief label editor window Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#include <QTreeWidget>

#include "label_editor_window.h"

#include <iostream>

/*-------------------------------------------------------------------*/
LabelEditorWindow::LabelEditorWindow( MainData & main_data,
                                      QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data )
{
    M_tree_view = new QTreeWidget( this );
    this->setCentralWidget( M_tree_view );
}

/*-------------------------------------------------------------------*/
LabelEditorWindow::~LabelEditorWindow()
{
    std::cerr << "delete LabelEditorWindow" << std::endl;
}
