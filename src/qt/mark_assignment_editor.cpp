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

namespace {

class SelectionToggleFilter 
    : public QObject {
private:
    QTableView * M_view;
    QPersistentModelIndex M_pressed_index;
    bool M_was_selected;

public:
    SelectionToggleFilter( QTableView * view, QObject * parent )
        : QObject( parent ),
          M_view( view ),
          M_was_selected( false )
    {
        M_view->viewport()->installEventFilter( this );

        connect( M_view, &QTableView::clicked,
                 this, [this]( const QModelIndex & index )
                 {
                     if ( index.isValid()
                          && index == M_pressed_index
                          && M_was_selected )
                     {
                         M_view->selectionModel()->select( index, QItemSelectionModel::Deselect );
                        //  std::cerr << "SelectionToggleFilter: deselect index=" << index.row() << "," << index.column() << std::endl;
                     }
                 } );
    }

    bool eventFilter( QObject * obj, QEvent * event ) override
    {
        if ( obj == M_view->viewport()
             && event->type() == QEvent::MouseButtonPress )
        {
            QMouseEvent * mouse_event = static_cast< QMouseEvent * >( event );
            QModelIndex index = M_view->indexAt( mouse_event->pos() );
            M_was_selected = index.isValid() && M_view->selectionModel()->isSelected( index );
            M_pressed_index = index;
            // std::cerr << "SelectionToggleFilter: index=" << index.row() << "," << index.column()
            //           << " was_selected=" << M_was_selected << std::endl;
        }
        return false;
    }
};

} // namespace

/*-------------------------------------------------------------------*/
MarkAssignmentEditor::MarkAssignmentEditor( MainData & main_data,
                                            QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data ),
      M_mark_assignment_view( nullptr ),
      M_current_time( 0, 0 )
{
    this->setWindowTitle( tr( "Mark Assignment Editor" ) );
    this->setWindowFlags( this->windowFlags() | Qt::Dialog );

    createView();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    readSettings();
}

/*-------------------------------------------------------------------*/
MarkAssignmentEditor::~MarkAssignmentEditor()
{
    // std::cerr << "delete MarkAssignmentEditor" << std::endl;
    writeSettings();
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
    M_main_data.markAssignmentData().clearHighlightedAssignments();
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::readSettings()
{
#ifndef Q_WS_WIN
    QSettings settings( QDir::homePath() + "/.soccerwindow2",
                        QSettings::IniFormat );
#else
    QSettings settings( QDir::currentPath() + "/soccerwindow2.ini",
                        QSettings::IniFormat );
#endif
    settings.beginGroup( "MarkAssignmentEditor" );

    QVariant val;

    val = settings.value( tr( "mark_log_dir_path" ) );
    if ( val.isValid() && val.canConvert< QString >() )
    {
        M_log_dir_path = val.toString();
    }

    settings.endGroup();
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::writeSettings()
{
#ifndef Q_WS_WIN
    QSettings settings( QDir::homePath() + "/.soccerwindow2",
                        QSettings::IniFormat );
#else
    QSettings settings( QDir::currentPath() + "/soccerwindow2.ini",
                        QSettings::IniFormat );
#endif
    settings.beginGroup( "MarkAssignmentEditor" );

    settings.setValue( tr( "mark_log_dir_path" ), M_log_dir_path );

    settings.endGroup();
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::createView()
{
    M_mark_assignment_view = new QTableView();
    M_model = new MarkAssignmentTableModel( this );

    M_mark_assignment_view->setModel( M_model );

    {
        const int col_width = M_mark_assignment_view->fontMetrics().horizontalAdvance( "(-00.0, -00.0)" ) + 8;

        QHeaderView * h = M_mark_assignment_view->horizontalHeader();
        h->setSectionResizeMode( QHeaderView::Fixed );
        h->setDefaultSectionSize( col_width );
        h->setMinimumSectionSize( col_width );
        h->setStretchLastSection( false );

        QHeaderView * v = M_mark_assignment_view->verticalHeader();
        v->setSectionResizeMode( QHeaderView::Fixed );
        v->setDefaultSectionSize( 24 );
    }

    this->setCentralWidget( M_mark_assignment_view );

    connect( M_mark_assignment_view->selectionModel(),
             &QItemSelectionModel::selectionChanged,
             this, &MarkAssignmentEditor::onSelectionChanged );

    new SelectionToggleFilter( M_mark_assignment_view, this );

    //
    connect( M_model, &QAbstractTableModel::dataChanged,
             this,
             [this]( const QModelIndex &, const QModelIndex &, const QVector< int > & )
             {
                 emit viewUpdateRequested();
             } );

    connect( M_model, &QAbstractTableModel::dataChanged,
             this,
             [this]( const QModelIndex &, const QModelIndex &, const QVector< int > & roles )
             {
                 if ( roles.isEmpty()
                      || roles.contains( Qt::CheckStateRole ) )
                 {
                    this->applyChanges();
                 }
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
    // file_menu->addAction( tr( "Save Changes" ), this, SLOT( saveChanges() ),
    //                       Qt::CTRL + Qt::Key_S );
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

    QWidget * spacer = new QWidget( this );
    spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    tbar->addWidget( spacer );

    M_show_mark_assignment_view_cb = new QCheckBox( tr( "Show on Field" ), this );
    M_show_mark_assignment_view_cb->setStatusTip( tr( "Show mark assignments on the field." ) );
    connect( M_show_mark_assignment_view_cb, &QCheckBox::toggled,
             [this]( bool checked ) {
                 Options::instance().setMarkAssignmentView( checked );
                 emit viewUpdateRequested(); // refresh the field canvas to show/hide mark assignments
             } );
    tbar->addWidget( M_show_mark_assignment_view_cb );

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

    M_show_mark_assignment_view_cb->setChecked( true );
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

    M_show_mark_assignment_view_cb->setChecked( false );
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
    const QString filter( tr( "CSV files (*.csv);;"
                        "All files (*)" ) );
    const QString default_dir = M_log_dir_path.isEmpty() ? QDir::homePath() : M_log_dir_path;
    const QString default_extension = ".csv";
    const QString file_path = QFileDialog::getOpenFileName( this,
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

    if ( ! M_main_data.markAssignmentData().open( file_path.toStdString() ) )
    {
        std::cerr << "(MarkAssignmentEditor::openMarkCostFeaturesLog) could not open " << file_path.toStdString() << std::endl;
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Could not open the file. " ) + file_path,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;
    }

    M_log_dir_path = QFileInfo( file_path ).absolutePath();

    this->setWindowTitle( tr( "Mark Assignment Editor - " ) + file_path );

    syncTime();
    emit viewUpdateRequested();
    return true;
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::saveChanges()
{
    QString file_path = QString::fromStdString( M_main_data.markAssignmentData().log().filePath() );
    saveChanges( file_path );
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentEditor::saveChangesAs()
{
    const QString filter( tr( "CSV files (*.csv);;"
                              "All files (*)" ) );

    const QString initial_path = QString::fromStdString( M_main_data.markAssignmentData().log().filePath() );

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

    const MarkCostFeaturesLog & log = M_main_data.markAssignmentData().log();
    const std::string & header = log.headerLine();
    const std::size_t accepted_idx = log.acceptedFieldIndex();
    const std::size_t label_idx = log.labelFieldIndex();
    const std::size_t same_in_last_step_idx = log.sameInLastStepFieldIndex();

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

    int saved_assignments = 0;

    const MarkAssignmentGroup * prev_group = nullptr;
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

            //
            // "SameInLastStep" field is updated based on whether the same marker-target pair is assigned in the previous time step,
            //
            if ( same_in_last_step_idx != std::string::npos
                 && prev_group )
            {
                bool same_in_last_step = false;
                for ( const MarkAssignment & prev_assignment : prev_group->assignments_ )
                {
                    if ( assignment.marker_.unum_ == prev_assignment.marker_.unum_
                         && assignment.target_.id_ == prev_assignment.target_.id_
                         && assignment.assigned_
                         && prev_assignment.assigned_ )
                    {
                        same_in_last_step = true;
                        break;
                    }
                }
                fields[same_in_last_step_idx] = ( same_in_last_step ? "1" : "0" );
            }

            for ( std::size_t i = 0; i < fields.size(); ++i )
            {
                if ( i > 0 ) fout << ',';
                fout << fields[i];
            }
            fout << '\n';
            ++saved_assignments;
        }

        prev_group = &group;
    }

    std::cerr << "(MarkAssignmentEditor::saveChanges) saved assignments = " << saved_assignments 
              << ", groups = " << log.groups().size()
              << ", modified groups = " << M_modified_times.size()
              << std::endl;
    M_modified_times.clear();
    M_main_data.markAssignmentData().log().setFilePath( file_path.toStdString() );

    M_log_dir_path = QFileInfo( file_path ).absolutePath();
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
        M_main_data.markAssignmentData().log().updateAssignmentGroup( M_current_time, group );
    }
    else
    {
        M_main_data.markAssignmentData().log().resetAcceptanceFlag( M_current_time );
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
    M_main_data.markAssignmentData().log().updateAssignmentGroup( M_current_time, group );

    M_accept_group_act->setChecked( true );

    emit viewUpdateRequested();
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

    M_main_data.markAssignmentData().setHighlightedAssignments( hl_set );
    emit viewUpdateRequested();
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

    const MarkCostFeaturesLog & log = M_main_data.markAssignmentData().log();
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
