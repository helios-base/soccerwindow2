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

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) )
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "mark_assignment_editor.h"

#include "mark_assignment_table_model.h"

#include "mark_cost_features_log.h"

#include "main_data.h"
#include "options.h"

#include <iomanip>
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
    createToolBars();
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
        M_model->setAssignmentGroup( MarkAssignmentGroup( "" ) );
    }

    M_time_label->setText( tr( "Time: N/A" ) );
    M_current_time.assign( -1, 0 );
    M_modified_times.clear();
    M_main_data.clearHighlightedAssignments();
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
        h->setDefaultSectionSize( 128 ); // TODO: make this configurable
        h->setMinimumSectionSize( 128 );
        h->setStretchLastSection( false );

        QHeaderView * v = M_mark_assignment_view->verticalHeader();
        v->setSectionResizeMode( QHeaderView::Fixed );
        v->setDefaultSectionSize( 24 );
    }

    this->setCentralWidget( M_mark_assignment_view );

    connect( M_mark_assignment_view->selectionModel(),
             &QItemSelectionModel::selectionChanged,
             this, &MarkAssignmentEditor::onSelectionChanged );

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
    M_accept_group_act = new QAction( tr( "Accept/Modified" ), this );
    M_accept_group_act->setStatusTip( tr( "Accept the current group and mark it as modified." ) );
    // M_accept_group_act はトグルボタンにする
    M_accept_group_act->setCheckable( true );
    connect( M_accept_group_act, &QAction::triggered,
             this, &MarkAssignmentEditor::acceptGroup );
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
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createToolBars()
{
    QToolBar * tbar = addToolBar( tr( "Edit" ) );
    tbar->setIconSize( QSize( 16, 16 ) );

    tbar->addAction( M_accept_group_act );
    if ( QToolButton * btn = qobject_cast< QToolButton * >( tbar->widgetForAction( M_accept_group_act ) ) )
    {
        btn->setStyleSheet( "QToolButton:checked { color: green; font-weight: bold; }" );
    }

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
    if ( M_modified_times.empty() )
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
                                : QString::fromStdString( Options::instance().debugLogDir() ) );
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

    this->setWindowTitle( tr( "Mark Assignment Editor - " ) + file_path );

    syncTime();
    emit assignmentsChanged();
    return true;
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::saveChanges()
{
    QString file_path = QString::fromStdString( M_main_data.markCostFeaturesLog().filePath() );
    saveChanges( file_path );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::saveChangesAs()
{
    const QString filter( tr( "CSV files (*.csv);;"
                              "All files (*)" ) );

    const QString initial_path = QString::fromStdString( M_main_data.markCostFeaturesLog().filePath() );

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

    const MarkCostFeaturesLog & log = M_main_data.markCostFeaturesLog();
    const std::string & header = log.headerLine();
    const std::size_t accepted_idx = log.acceptedFieldIndex();
    const std::size_t label_idx = log.labelFieldIndex();

    if ( accepted_idx == std::string::npos )
    {
        std::cerr << "(MarkAssignmentEditor::saveChanges) ERROR: accepted field index is npos in log" << std::endl;
        return;
    }

    if ( label_idx == std::string::npos )
    {
        std::cerr << "(MarkAssignmentEditor::saveChanges) ERROR: label field index is npos in log" << std::endl;
        return;
    }

    // print header
    if ( header.empty() )
    {
        std::cerr << "(MarkAssignmentEditor::saveChanges) ERROR: empty header line in log" << std::endl;
        return;
    }

    fout << header << '\n';

    int saved_groups = 0;
    for ( const auto & [time, group] : log.groups() )
    {
        const std::string accepted_flag = ( group.accepted_ ? "1" : "0" );
        for ( const MarkAssignment & assignment : group.assignments_ )
        {
            if ( assignment.raw_fields_.empty() )
            {
                std::cerr << "(MarkAssignmentEditor::saveChanges) warning: empty raw fields for an assignment at time " << time << std::endl;
                continue;
            }
            if ( accepted_idx >= assignment.raw_fields_.size() )
            {
                std::cerr << "(MarkAssignmentEditor::saveChanges) warning: accepted index " << accepted_idx << " is out of range for raw fields of size " << assignment.raw_fields_.size() << " for an assignment at time " << time << std::endl;
                continue;
            }
            if ( label_idx >= assignment.raw_fields_.size() )
            {
                std::cerr << "(MarkAssignmentEditor::saveChanges) warning: label index " << label_idx << " is out of range for raw fields of size " << assignment.raw_fields_.size() << " for an assignment at time " << time << std::endl;
                continue;
            }

            // output the original row with only the label column updated
            std::vector< std::string > fields = assignment.raw_fields_;
            fields[accepted_idx] = accepted_flag;
            fields[label_idx] = ( assignment.assigned_ ? "1" : "0" );
            for ( std::size_t i = 0; i < fields.size(); ++i )
            {
                if ( i > 0 ) fout << ',';
                fout << fields[i];
            }
            fout << '\n';
            ++saved_groups;
        }
    }

    std::cerr << "(MarkAssignmentEditor::saveChanges) saved groups = " << saved_groups << "/" << log.groups().size()
              << ", modified groups = " << M_modified_times.size()
              << std::endl;
    M_modified_times.clear();
    M_main_data.setMarkCostFeaturesLogFilePath( file_path.toStdString() );

    this->setWindowTitle( tr( "Mark Assignment Editor - " ) + file_path );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::acceptGroup( bool checked )
{
    M_modified_times.insert( M_current_time );

    if ( checked )
    {
        const MarkAssignmentGroup group = M_model->getAssignmentGroup();
        if ( group.assignments_.empty() )
        {
            std::cerr << "(MarkAssignmentEditor::acceptGroup) no assignments to accept" << std::endl;
            return;
        }
        M_main_data.updateMarkAssignmentGroup( M_current_time, group );
    }
    else
    {
        M_main_data.resetMarkAssignmentAcceptanceFlag( M_current_time );
    }
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::applyChanges()
{
    const MarkAssignmentGroup group = M_model->getAssignmentGroup();

    if ( group.assignments_.empty() )
    {
        std::cerr << "(MarkAssignmentEditor::applyChanges) no assignments to apply" << std::endl;
        return;
    }

    M_modified_times.insert( M_current_time );
    M_main_data.updateMarkAssignmentGroup( M_current_time, group );

    M_accept_group_act->setChecked( true );

    emit assignmentsChanged();
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::onSelectionChanged( const QItemSelection & /* selected */,
                                          const QItemSelection & /* deselected */ )
{
    std::set< std::pair< int, char > > hl_set;

    for ( const QModelIndex & idx : M_mark_assignment_view->selectionModel()->selectedIndexes() )
    {
        const Marker * marker = M_model->markerAt( idx.row() );
        const MarkTargetKey * target = M_model->targetAt( idx.column() );
        if ( marker && target )
        {
            hl_set.emplace( marker->unum_, target->id_ );
        }
    }

    M_main_data.setHighlightedAssignments( hl_set );
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
        M_accept_group_act->setChecked( false );
        M_accept_group_act->setEnabled( false );
        return;
    }

    if ( view->time() != M_current_time )
    {
        // clear selection when time changes to avoid confusion,
        // since the selected assignments may not be relevant to the new time
        M_mark_assignment_view->clearSelection();
    }

    const MarkCostFeaturesLog & log = M_main_data.markCostFeaturesLog();
    const MarkAssignmentGroup & group = log.getAssignmentGroupAt( view->time() );

    M_current_time = view->time();
    if ( group.assignments_.empty() )
    {
        // std::cerr << "(MarkAssignmentEditor::syncTime) no assignments at time " << view->time() << std::endl;
        M_time_label->setText( tr( "Time: %1 - %2, No assignments" ).arg( view->time().cycle() ).arg( view->time().stopped() ) );
    }
    else
    {
        M_time_label->setText( tr( "Time: %1 - %2" ).arg( M_current_time.cycle() ).arg( M_current_time.stopped() ) );
    }
    M_model->setAssignmentGroup( group );

    M_accept_group_act->setChecked( group.accepted_ );
    M_accept_group_act->setEnabled( ! group.assignments_.empty() );
}
