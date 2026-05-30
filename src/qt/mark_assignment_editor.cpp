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

#include "mark_cost_features_log.h"


#include "main_data.h"
#include "options.h"

#include <fstream>
#include <filesystem>
#include <ctime>

using namespace rcsc;

/*-------------------------------------------------------------------*/
MarkAssignmentEditor::MarkAssignmentEditor( MainData & main_data,
                                            QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data ),
      M_mark_assignment_view( nullptr ),
      M_current_time( 0, 0 )
{
    this->setWindowTitle( tr( "Mark Assignment Editor" ) );

    createView();
    createActions();
    createMenus();
    //createToolBars();
    createStatusBar();
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
        M_model->setAssignments( std::vector< MarkAssignment >() );
        M_mark_assignments_changes.clear();
    }
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createView()
{
    M_mark_assignment_view = new QTableView();
    M_model = new MarkAssignmentTableModel( this );

    M_mark_assignment_view->setModel( M_model );

    {
        QHeaderView * h = M_mark_assignment_view->horizontalHeader();
        h->setSectionResizeMode( QHeaderView::Fixed );
        h->setDefaultSectionSize( 128 );   // TODO: make this configurable
        h->setMinimumSectionSize( 128 );
        h->setStretchLastSection( false );

        QHeaderView * v = M_mark_assignment_view->verticalHeader();
        v->setSectionResizeMode( QHeaderView::Fixed );
        v->setDefaultSectionSize( 24 );
    }

    this->setCentralWidget( M_mark_assignment_view );

    //
    connect( M_model, &QAbstractTableModel::dataChanged,
             this,
             [this]( const QModelIndex &, const QModelIndex &, const QVector< int > & )
             {
                 emit assignmentsChanged();
             } );

    connect( M_model, &QAbstractTableModel::dataChanged,
             this,
             [this]( const QModelIndex &, const QModelIndex &, const QVector< int > & )
             {
                 this->applyChanges();
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
    file_menu->addAction( tr( "Save Changes" ), this, SLOT( saveChanges() ),
                          Qt::CTRL + Qt::Key_S );
    file_menu->addAction( tr( "Save Changes As" ), this, SLOT( saveChangesAs() ),
                          Qt::CTRL + Qt::SHIFT + Qt::Key_S );
    file_menu->addAction( tr( "Close" ), this, SLOT( close() ),
                          Qt::CTRL + Qt::Key_W );

    // QMenu * edit_menu = menuBar()->addMenu( tr( "Edit" ) );
    // edit_menu->addAction( tr( "Sync with Field" ), this, SLOT( syncTime() ) );
    // edit_menu->addAction( tr( "Apply Changes" ), this, SLOT( applyChanges() ) );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createToolBars()
{
    QToolBar * tbar = addToolBar( tr( "Edit" ) );
    tbar->setIconSize( QSize( 16, 16 ) );

    //tbar->addAction( tr( "Sync with Field" ), this, SLOT( syncTime() ) );
    tbar->addAction( tr( "Apply Changes" ), this, SLOT( applyChanges() ) );

    this->addToolBar( Qt::TopToolBarArea, tbar );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createStatusBar()
{
    this->statusBar()->showMessage( tr( "Ready" ) );

    this->statusBar()->addPermanentWidget( M_time_label = new QLabel() );
    M_time_label->setText( tr( "Time: N/A" ) ); 
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::adjustWindowSizeToTable()
{
    if ( ! M_mark_assignment_view )
    {
        return;
    }

    const int max_cols = 11;
    const int max_rows = 10;
    const int col_width = M_mark_assignment_view->horizontalHeader()->defaultSectionSize();
    const int row_height = M_mark_assignment_view->verticalHeader()->defaultSectionSize();

    int table_w = M_mark_assignment_view->verticalHeader()->width()
                + M_mark_assignment_view->frameWidth() * 2
                + col_width * max_cols
                + M_mark_assignment_view->style()->pixelMetric( QStyle::PM_ScrollBarExtent );

    int table_h = M_mark_assignment_view->horizontalHeader()->height()
                + M_mark_assignment_view->frameWidth() * 2
                + row_height * max_rows;

    table_w += M_mark_assignment_view->style()->pixelMetric( QStyle::PM_ScrollBarExtent );
    table_h += M_mark_assignment_view->style()->pixelMetric( QStyle::PM_ScrollBarExtent );

    const QSize frame_extra = this->size() - this->centralWidget()->size();
    QSize new_size( table_w + frame_extra.width(),
                    table_h + frame_extra.height() );

    const QRect avail = this->screen()->availableGeometry();
    new_size.setWidth( std::min( new_size.width(), avail.width() ) );
    new_size.setHeight( std::min( new_size.height(), avail.height() ) );

    this->resize( new_size );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::showEvent( QShowEvent * event )
{
    QMainWindow::showEvent( event );
    Options::instance().setMarkAssignmentView( true );
    syncTime();

    //if ( ! M_initial_auto_resize_done )
    {
        adjustWindowSizeToTable();
        M_initial_auto_resize_done = true;
    }
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::closeEvent( QCloseEvent * event )
{
    if ( ! checkAndWarnUnsavedChanges() )
    {
        // The user chose not to discard changes, so ignore the close event.
        event->ignore();
        return;
    }
    
    Options::instance().setMarkAssignmentView( false );
    event->accept();
}

/*-------------------------------------------------------------------*/
bool
MarkAssignmentEditor::checkAndWarnUnsavedChanges()
{
    if ( M_mark_assignments_changes.empty() )
    {
        return true;
    }

    int ret = QMessageBox::warning( this,
                                    tr( "Warning" ),
                                    tr( "There are unsaved changes. Do you want to discard them?" ),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No );
    if ( ret != QMessageBox::Yes )
    {
        // The user chose not to discard changes.
        return false;
    }

    return true;
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
    if ( ! checkAndWarnUnsavedChanges() )
    {
        // The user chose not to discard changes, so do not open a new file.
        return false;
    }

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

    // clear existing data and changes
    clearAll();

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

    syncTime();
    emit assignmentsChanged();
    return true;
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
namespace {
std::string get_current_datetime_str()
{
    std::time_t t = std::time(nullptr);
    char buf[20];
    if ( std::strftime( buf, sizeof(buf), "%Y%m%d-%H%M%S", std::localtime(&t) ) )
    {
        return std::string( buf );
    }
    else
    {
        return "unknown_datetime";
    }
}
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::saveChanges()
{
    if ( M_saved_file_path.isEmpty() )
    {
        saveChangesAs();
    }
    else
    {
        saveChanges( M_saved_file_path );
    }

}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::saveChangesAs()
{
    const QString filter( tr( "CSV files (*.csv);;" 
                          "All files (*)" ) );
    const QString default_dir = ( Options::instance().debugLogDir().empty()
                                  ? tr( "" )
                                  : QString::fromStdString( Options::instance().debugLogDir() ) );

    // determine default file name
    const std::filesystem::path data_file_path = M_main_data.markCostFeaturesLog().filePath();
    const std::string stem_str = data_file_path.stem().string();
    const std::string datetime_str = get_current_datetime_str();

    std::string default_file_name;
    if ( stem_str.compare( 0, 19, "mark_cost_features_" ) != 0 )
    {
        std::cerr << "(MarkAssignmentEditor::saveChangesAs) warning: unexpected data file name: " << data_file_path << std::endl;
        default_file_name = "assignments_" + stem_str + ".csv";
    }
    else
    {
        // remove "mark_cost_features_" prefix and possible datetime suffix from the stem
        default_file_name = "assignments_" + stem_str.substr( 19 ) + "_" + datetime_str + ".csv";
    }

    const QString default_name = QString::fromStdString( default_file_name );
    const QString initial_path = ( default_dir.isEmpty()
                                   ? default_name
                                   : QDir( default_dir ).filePath( default_name ) );


    QString file_path = QFileDialog::getSaveFileName( this,
                                                      tr( "Save changes to a csv file as" ),
                                                      initial_path,
                                                      filter );
    if ( file_path.isEmpty() )
    {
        // The user canceled the dialog.
        return;
    }

    saveChanges( file_path );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::saveChanges( const QString & file_path ) 
{
    std::ofstream fout( file_path.toStdString() );
    if ( ! fout )    
    {
        std::cerr << "(MarkAssignmentEditor::saveChanges) could not open " << file_path.toStdString() << " for writing" << std::endl;
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Could not open the file for writing. " ) + file_path,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return;
    }

    // print header
    fout << "Time,MarkerUnum,MarkerX,MarkerY,TargetId,TargetUnum,TargetX,TargetY" << std::endl;

    for ( const auto & [ time, assignments ] : M_mark_assignments_changes )
    {
        const std::string time_str = std::to_string( time.cycle() ) + "-" + std::to_string( time.stopped() );
        for ( const auto & assignment : assignments )
        {
            fout << time_str << ","
                 << assignment.marker_.unum_ << ","
                 << assignment.marker_.pos_.x << ","
                 << assignment.marker_.pos_.y << ","
                 << assignment.target_.id_ << ","
                 << assignment.target_.unum_ << ","
                 << assignment.target_.pos_.x << ","
                 << assignment.target_.pos_.y << std::endl;
        }
    }

    M_saved_file_path = file_path;
    std::cerr << "(MarkAssignmentEditor::saveChanges) saved changes: count = " << M_mark_assignments_changes.size() << std::endl;
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::applyChanges()
{
    std::vector< MarkAssignment > assignments = M_model->getAssignments();

    if ( assignments.empty() )
    {
        std::cerr << "(MarkAssignmentEditor::applyChanges) no assignments to apply" << std::endl;
        return;
    }

    M_mark_assignments_changes[M_current_time] = assignments;
    M_main_data.updateMarkAssignments( M_current_time, assignments );
    
    emit assignmentsChanged();
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::syncTime()
{
    if ( ! this->isVisible() )
    {
        // std::cerr << "(MarkAssignmentEditor::syncTime) editor is not visible" << std::endl;
        return;
    }

    MonitorViewData::ConstPtr view = M_main_data.getCurrentViewData();
    if ( ! view )
    {
        // std::cerr << "(MarkAssignmentEditor::syncTime) no current view data" << std::endl;
        return;
    }

    const MarkCostFeaturesLog & log = M_main_data.markCostFeaturesLog();
    const std::vector< MarkAssignment > & assignments = log.getAssignmentsAt( view->time() );

    M_current_time = view->time();
    if ( assignments.empty() )
    {
        // std::cerr << "(MarkAssignmentEditor::syncTime) no assignments at time " << view->time() << std::endl;
        M_time_label->setText( tr( "Time: %1 - %2, No assignments" ).arg( view->time().cycle() ).arg( view->time().stopped() ) );
    }
    else
    {
        M_time_label->setText( tr( "Time: %1 - %2" ).arg( M_current_time.cycle() ).arg( M_current_time.stopped() ) );
    }
    M_model->setAssignments( assignments );
}
