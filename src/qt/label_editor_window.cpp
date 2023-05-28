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

#include "features_log_parser.h"
#include "monitor_view_data.h"
#include "main_data.h"

#include <iostream>
#include <fstream>

#include "xpm/open.xpm"
#include "xpm/save.xpm"

/*-------------------------------------------------------------------*/
LabelEditorWindow::LabelEditorWindow( MainData & main_data,
                                      QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data )
{
    M_table_view = new QTableWidget();
    this->setCentralWidget( M_table_view );

    createActions();
    createToolBars();
    createMenus();

    this->statusBar()->showMessage( tr( "Ready" ) );

    // QPoint move_pos;

    // int width_offset = parent->frameGeometry().width() - parent->geometry().width();
    // int height_offset = parent->frameGeometry().height() - parent->geometry().height();
    // // move_pos.setX( std::max( 0, parent->pos().x() + std::max( 0, ( parent->width() - this->width() ) / 2 ) - this->width() / 2 ) );
    // // move_pos.setY( std::max( 0, parent->pos().y() + std::max( 0, ( parent->height() - this->height() ) / 2 ) - this->height() / 2 ) );
    // move_pos.setX( parent->frameGeometry().left() + width_offset + 10 );
    // move_pos.setY( parent->frameGeometry().top() + height_offset + 10 );
    // //this->move( move_pos );
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
    M_open_act = new QAction( QIcon( QPixmap( open_xpm ) ),
                              tr( "Open features file" ),
                              this );
    M_open_act->setShortcut( Qt::CTRL + Qt::Key_O );
    M_open_act->setToolTip( tr( "Open a features file." ) );
    M_open_act->setStatusTip( tr( "Open a features file." ) );
    connect( M_open_act, SIGNAL( triggered() ), this, SLOT( openFile() ) );
    //
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

    menu->addAction( M_open_act );
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

    M_tool_bar->addAction( M_open_act );
    M_tool_bar->addAction( M_save_act );
    M_tool_bar->addSeparator();
}

/*-------------------------------------------------------------------*/
bool
LabelEditorWindow::saveChanges()
{
    return true;
}

/*-------------------------------------------------------------------*/
bool
LabelEditorWindow::openFile( const QString & filepath )
{
    std::cerr << "(LabelEditorWindow::openFile) " << filepath.toStdString()
              << std::endl;

    if ( filepath.isEmpty() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Empty file path." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    QFileInfo fileinfo( filepath );

    if ( ! fileinfo.exists()
         || ! fileinfo.isReadable() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "No such a file or not readable. \n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    std::ifstream fin( filepath.toStdString() );

    if ( ! fin.is_open() )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Could not open the file \n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    FeaturesLogParser parser;
    M_features_log = parser.parse( fin );

    if ( ! M_features_log )
    {
        std::cerr << "Null Features Log" << std::endl;
        return false;
    }

    //std::cout << "features log size = " << M_features_log->timedMap().size() << std::endl;

    initTable();

    return true;
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::openFile()
{
    if ( ! saveChanges() )
    {
        // data changed, but save operation is canceled.
        return;
    }

    QString filter( tr( "Features file (*.features);;"
                        "All files (*)" ) );
    QString filepath = QFileDialog::getOpenFileName( this,
                                                     tr( "Open Features fille" ),
                                                     tr( "" ),
                                                     filter );
    if ( filepath.isEmpty() )
    {
        return;
    }

    openFile( filepath );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::saveData()
{
    std::cerr << "(LabelEditorWindow::saveData)" << std::endl;
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::initTable()
{
   if ( ! M_features_log )
    {
        return;
    }

    M_table_view->clear();

    M_table_view->setColumnCount( 1 // label
                                  + M_features_log->floatFeaturesSize()
                                  + M_features_log->catFeaturesSize() );

    // set header
    {
        QStringList names;
        names << "label";
        for ( const std::string & s : M_features_log->featureNames() )
        {
            names << QString::fromStdString( s );
        }
        M_table_view->setHorizontalHeaderLabels( names );
    }

    updateTableContents();
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::updateTableContents()
{
    if ( ! M_features_log )
    {
        return;
    }

    M_table_view->clearContents();

    // get the current field data
    MonitorViewData::ConstPtr view = M_main_data.getCurrentViewData();
    if ( ! view )
    {
        std::cerr << "(LabelEditorWindow::updateTable) No monitor view." << std::endl;
        return;
    }

    // get the grouped data
    WholeFeaturesLog::Map::const_iterator it = M_features_log->timedMap().find( view->time() );
    if ( it == M_features_log->timedMap().end() )
    {
        std::cerr << "(LabelEditorWindow::updateTable) No timed data." << std::endl;
        return;
    }

    if ( ! it->second )
    {
        std::cerr << "(LabelEditorWindow::updateTable) No grouped data." << std::endl;
        return;
    }

    M_table_view->setRowCount( it->second->featuresList().size() );
    std::cerr << "(LabelEditorWindow::updateTableContents) rowCount = " << M_table_view->rowCount() << std::endl;

    // loop in the group
    int row_count = 0;
    for ( const FeaturesLog::ConstPtr & f : it->second->featuresList() )
    {
        int column_count = 0;

        M_table_view->setItem( row_count, column_count, new QTableWidgetItem( QString::number( f->label() ) ) );
        ++column_count;

        for ( const double v : f->floatFeatures() )
        {
            M_table_view->setItem( row_count, column_count, new QTableWidgetItem( QString::number( v ) ) );
            ++column_count;
        }

        for ( const std::string & v : f->catFeatures() )
        {
            M_table_view->setItem( row_count, column_count, new QTableWidgetItem( QString::fromStdString( v ) ) );
            ++column_count;
        }

        ++row_count;
    }

}
