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

//#include <QTreeWidget>
#include <QTableWidget>

#include "label_editor_window.h"

#include <iostream>

#include "xpm/save.xpm"

/*-------------------------------------------------------------------*/
LabelEditorWindow::LabelEditorWindow( MainData & main_data,
                                      QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data )
{
    // M_tree_view = new QTreeWidget( this );
    // this->setCentralWidget( M_tree_view );
    M_table_view = new QTableWidget( this );
    this->setCentralWidget( M_table_view );

    createActions();
    createToolBars();
    createMenus();

    this->statusBar()->showMessage( tr( "Ready" ) );
}

/*-------------------------------------------------------------------*/
LabelEditorWindow::~LabelEditorWindow()
{
    std::cerr << "delete LabelEditorWindow" << std::endl;
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::createActions()
{
    M_save_act = new QAction( QIcon( QPixmap( save_xpm ) ),
                              tr( "Save data" ),
                              this );
    M_save_act->setShortcut( Qt::CTRL + Qt::Key_S );
    M_save_act->setToolTip( tr( "Save the current model." ) );
    M_save_act->setStatusTip( tr( "Save the current model." ) );
    connect( M_save_act, SIGNAL( triggered() ), this, SLOT( saveData() ) );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::createMenus()
{
    createMenuFile();
    //createMenuEdit();
    //createMenuView();
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::createMenuFile()
{
    QMenu * menu = menuBar()->addMenu( tr( "&File" ) );

    menu->addAction( M_save_act );

    menu->addSeparator();

    menu->addAction( tr( "Close" ), this, SLOT( close() ), Qt::CTRL + Qt::Key_W );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::createToolBars()
{
    M_tool_bar = new QToolBar( tr( "Edit tools" ), this );
    this->addToolBar( Qt::TopToolBarArea, M_tool_bar );

    M_tool_bar->addAction( M_save_act );
    M_tool_bar->addSeparator();
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::saveData()
{
    std::cerr << "(LabelEditorWindow::saveData)" << std::endl;
}
