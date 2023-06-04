// -*-c++-*-

/*!
  \file simple_label_selector.h
  \brief simple label selector window Source File.
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

#include "simple_label_selector.h"

#include "monitor_view_data.h"
#include "main_data.h"
#include "options.h"

#include <fstream>

namespace {
constexpr int TIME_COLUMN = 0;
constexpr int LABEL_COLUMN = 1;
constexpr int MAX_COLUMN = 2;

constexpr int MAX_LABELS = 3;
}

/*-------------------------------------------------------------------*/
SimpleLabelSelector::SimpleLabelSelector( MainData & main_data,
                                          QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data ),
      M_init( false )
{
    this->setWindowTitle( tr( "Simple Label Selector" ) );
    this->setWindowFlags( this->windowFlags() | Qt::WindowStaysOnTopHint );

    createView();

    createActions();
    createToolBars();
    createMenus();

    this->statusBar()->showMessage( tr( "Ready" ) );
    this->statusBar()->hide();
}

/*-------------------------------------------------------------------*/
SimpleLabelSelector::~SimpleLabelSelector()
{

}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::showEvent( QShowEvent * event )
{
    updateData();

    //event->accept();
    QMainWindow::showEvent( event );
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::createView()
{
    M_model = new QStandardItemModel( 0, MAX_COLUMN, this );
    M_model->setHeaderData( TIME_COLUMN, Qt::Horizontal, tr( "time" ) );
    M_model->setHeaderData( LABEL_COLUMN, Qt::Horizontal, tr( "label" ) );

    M_time_label_view = new QTableView();
    //M_time_label_view->verticalHeader()->hide();
    M_time_label_view->setShowGrid( false );
    M_time_label_view->setAlternatingRowColors( true );
    M_time_label_view->setSortingEnabled( false );
    M_time_label_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_time_label_view->setSelectionMode( QAbstractItemView::ExtendedSelection );
    M_time_label_view->setEditTriggers( QAbstractItemView::NoEditTriggers );
    //M_time_label_view->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    M_time_label_view->horizontalHeader()->setStretchLastSection( true );
    //M_time_label_view->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
    M_time_label_view->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
    //M_time_label_view->horizontalHeader()->setSectionResizeMode( TIME_COLUMN, QHeaderView::ResizeToContents );

    M_time_label_view->setModel( M_model );

    {
        QItemSelectionModel * selection_model = M_time_label_view->selectionModel();
        connect( selection_model, SIGNAL( currentRowChanged( const QModelIndex &, const QModelIndex & ) ),
                 this, SLOT( onTableSelectionChanged( const QModelIndex &, const QModelIndex & ) ) );
    }

    // this->setCentralWidget( M_time_label_view );
    QFrame * top_frame = new QFrame( this );
    this->setCentralWidget( top_frame );

    QHBoxLayout * top_box = new QHBoxLayout();
    top_box->setMargin( 2 );
    top_box->setSpacing( 2 );

    top_frame->setLayout( top_box );

    top_box->addWidget( M_time_label_view );
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::createActions()
{
    M_save_csv_act = new QAction( tr( "Save CSV" ), this );
    M_save_csv_act->setToolTip( tr( "Save the current values in CSV format." ) );
    M_save_csv_act->setStatusTip( tr( "Save the current values in CSV format." ) );
    connect( M_save_csv_act, SIGNAL( triggered() ), this, SLOT( saveCSV() ) );
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::createMenus()
{
    QMenu * menu = menuBar()->addMenu( tr( "&File" ) );

    menu->addAction( M_save_csv_act );
    menu->addSeparator();
    menu->addAction( tr( "Close" ), this, SLOT( close() ), Qt::CTRL + Qt::Key_W );
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::createToolBars()
{
    M_tool_bar = new QToolBar( tr( "Edit tools" ), this );
    //this->addToolBar( Qt::TopToolBarArea, M_tool_bar );
    this->addToolBar( Qt::LeftToolBarArea, M_tool_bar );

    // M_tool_bar->addAction( M_save_csv_act );
    // M_tool_bar->addSeparator();

    for ( int i = 0; i < MAX_LABELS; ++i )
    {
        QPushButton * label_button = new QPushButton( tr( "Label %1" ).arg( i ), M_tool_bar );
        connect( label_button, &QRadioButton::clicked,
                 [this, i]()
                   {
                       selectLabel( i );
                   } );

        M_tool_bar->addWidget( label_button );
    }
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::updateData()
{
    const int row_size = M_main_data.viewHolder().monitorViewCont().size();

    if ( M_model->rowCount() != row_size
         || ! M_init )
    {
        M_model->setRowCount( row_size );

        int row = 0;
        for ( const MonitorViewData::ConstPtr & d : M_main_data.viewHolder().monitorViewCont() )
        {
            // std::cerr << "updateData row = " << row << std::endl;
            if ( ! M_model->item( row, TIME_COLUMN ) )
            {
                M_model->setData( M_model->index( row, TIME_COLUMN ),
                                  tr( "%1, %2" ).arg( d->time().cycle() ).arg( d->time().stopped() ),
                                  Qt::DisplayRole );
            }

            if ( ! M_model->item( row, LABEL_COLUMN ) )
            {
                M_model->setData( M_model->index( row, LABEL_COLUMN ),
                                  0, //tr( "" ),
                                  Qt::DisplayRole );
            }

            ++row;
        }

        M_init = true;
    }
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::clearAll()
{
    M_model->clear();
    M_init = false;

    M_model->setColumnCount( MAX_COLUMN );
    M_model->setHeaderData( TIME_COLUMN, Qt::Horizontal, tr( "time" ) );
    M_model->setHeaderData( LABEL_COLUMN, Qt::Horizontal, tr( "label" ) );
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::saveCSV()
{
    QString filter( tr( "CSV files (*.csv);;"
                        "All files (*)" ) );
    //QString default_dir = QString::fromStdString( Options::instance().debugLogDir() );
    QString default_dir = tr( "" );
    QString default_extension = ".csv";
    QString filepath = QFileDialog::getSaveFileName( this,
                                                     tr( "Save a csv file as" ),
                                                     default_dir,
                                                     filter );
    if ( filepath.isEmpty() )
    {
        std::cerr << "(SimpleLabelSelector::saveCSV) empty file path" << std::endl;
        return;
    }

    {
        QFileInfo fileinfo( filepath );
        QString extension = fileinfo.suffix();
        if ( extension.isEmpty() )
        {
            filepath += default_extension;
        }
    }

    saveCSV( filepath );
}

/*-------------------------------------------------------------------*/
bool
SimpleLabelSelector::saveCSV( const QString & filepath )
{
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
    if ( fileinfo.exists() )
    {
        if ( QMessageBox::warning( this,
                                   tr( "Warning" ),
                                   tr( "Already exists a same named file. overwrite? \n" ) + filepath,
                                   QMessageBox::Ok | QMessageBox::Cancel,
                                   QMessageBox::NoButton ) != QMessageBox::Ok )
        {
            return false;
        }
    }

    std::ofstream fout( filepath.toStdString() );

    const int row_count = M_model->rowCount();

    for ( int r = 0; r < row_count; ++r )
    {
        QStandardItem * time_item = M_model->item( r, TIME_COLUMN );
        QStandardItem * label_item = M_model->item( r, LABEL_COLUMN );
        if ( time_item
             && label_item
             && ! time_item->text().isEmpty()
             && ! label_item->text().isEmpty() )
        {
            fout << time_item->text().toStdString()
                 << ", " << label_item->text().toStdString()
                 << '\n';
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::selectLabel( const int label )
{
    QItemSelectionModel * selection_model = M_time_label_view->selectionModel();
    if ( ! selection_model )
    {
        return;
    }

    for ( const QModelIndex & index : selection_model->selectedRows( LABEL_COLUMN ) )
    {
        QStandardItem * label_item = M_model->item( index.row(), LABEL_COLUMN );
        if ( label_item )
        {
            label_item->setData( label, Qt::DisplayRole );
        }
    }
}

/*-------------------------------------------------------------------*/
void
SimpleLabelSelector::onTableSelectionChanged( const QModelIndex & index,
                                              const QModelIndex & /*previous*/ )
{
    const QStandardItem * time_item = M_model->item( index.row(), TIME_COLUMN );
    if ( ! time_item )
    {
        return;
    }

    const QStringList num_strs = time_item->text().split( "," );
    if ( num_strs.size() != 2 )
    {
        return;
    }

    bool ok1, ok2;
    const int cycle = num_strs[0].toInt( &ok1 );
    const int stopped = num_strs[1].toInt( &ok2 );
    if ( ok1 && ok2 )
    {
        emit cycleSelected( rcsc::GameTime( cycle, stopped ) );
    }
}
