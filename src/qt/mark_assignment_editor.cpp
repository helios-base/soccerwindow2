// -*-c++-*-

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

#include "mark_assignment_editor.h"

#include "mark_assignment_table_model.h"

#include "mark_cost_features_log_parser.h"


#include "main_data.h"
#include "options.h"

#include <fstream>

using namespace std;

/*-------------------------------------------------------------------*/
MarkAssignmentEditor::MarkAssignmentEditor( MainData & main_data,
                                            QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data ),
      M_mark_assignment_view( nullptr )
{
    this->setWindowTitle( tr( "Mark Assignment Editor" ) );

    createView();
    createActions();
    createMenus();
    createToolBars();
}

/*-------------------------------------------------------------------*/
MarkAssignmentEditor::~MarkAssignmentEditor()
{
    // std::cerr << "delete MarkAssignmentEditor" << std::endl;
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::clearAll()
{
    if ( M_model )
    {
        M_model->setAssignments( std::vector< MarkAssignment::Ptr >() );
    }
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createView()
{
    M_mark_assignment_view = new QTableView();
    M_model = new MarkAssignmentTableModel( this );

    M_mark_assignment_view->setModel( M_model );

    this->setCentralWidget( M_mark_assignment_view );

    //
    connect( M_model, &QAbstractTableModel::dataChanged,
             this,
             [this]( const QModelIndex &, const QModelIndex &, const QVector< int > & )
             {
                 emit assignmentsChanged();
             } );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createActions()
{
    // TODO
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createMenus()
{
    QMenu * file_menu = menuBar()->addMenu( tr( "File" ) );
    file_menu->addAction( tr( "Open Data File" ), this, SLOT( openMarkCostFeaturesLog() ),
                          Qt::CTRL + Qt::Key_O );
    file_menu->addAction( tr( "Close" ), this, SLOT( close() ),
                          Qt::CTRL + Qt::Key_W );

    QMenu * edit_menu = menuBar()->addMenu( tr( "Edit" ) );
    edit_menu->addAction( tr( "Sync" ), this, SLOT( syncCycle() ),
                          Qt::CTRL + Qt::Key_S );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createToolBars()
{
    QToolBar * tbar = addToolBar( tr( "Edit" ) );
    tbar->setIconSize( QSize( 16, 16 ) );

    tbar->addAction( tr( "Sync" ), this, SLOT( syncCycle() ) );

    this->addToolBar( Qt::TopToolBarArea, tbar );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::openMarkCostFeaturesLog()
{
    QString filter( tr( "CSV files (*.csv);;" 
                        "All files (*)" ) );
    QString default_dir = ( Options::instance().debugLogDir().empty()
                            ? tr( "" )
                            : QString::fromStdString( Options::instance().debugLogDir() ) )    ;
    QString default_extension = ".csv";
    QString file_path = QFileDialog::getOpenFileName( this,
                                                      tr( "Open a csv file as" ),
                                                      default_dir,
                                                      filter );
    if ( file_path.isEmpty() )
    {
        // The user canceled the dialog.
        return;
    }

    // {
    //     QFileInfo fileinfo( file_path );
    //     QString extension = fileinfo.suffix();
    //     if ( extension.isEmpty() )
    //     {
    //         file_path += default_extension;
    //     }
    // }

    openMarkCostFeaturesLog( file_path );
}

/*-------------------------------------------------------------------*/
bool
MarkAssignmentEditor::openMarkCostFeaturesLog( const QString & file_path )
{
    if ( file_path.isEmpty() )
    {
        std::cerr << "(MarkAssignmentEditor::openMarkCostFeaturesLog) empty file path" << std::endl;
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Empty file path." ),
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    if ( ! M_main_data.openMarkCostFeaturesLog( file_path.toStdString() ) )
    {
        std::cerr << "(MarkAssignmentEditor::openMarkCostFeaturesLog) could not open " << file_path.toStdString() << std::endl;
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Could not open the file. " ) + file_path,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    syncCycle();
    
    return true;
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::syncCycle()
{
    MonitorViewData::ConstPtr view = M_main_data.getCurrentViewData();
    if ( ! view )
    {
        std::cerr << "(MarkAssignmentEditor::syncCycle) no current view data" << std::endl;
        return;
    }

    const MarkCostFeaturesLog & log = M_main_data.markCostFeaturesLog();

    const std::vector< MarkAssignment::Ptr > & assignments = log.getAssignmentsAt( view->time() );
    if ( assignments.empty() )
    {
        std::cerr << "(MarkAssignmentEditor::syncCycle) no assignments at the current time" << std::endl;
        return;
    }

    M_model->setAssignments( assignments );
}
