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

namespace {
constexpr int RANK_COLUMN = 0;
constexpr int SCORE_COLUMN = 1;
}

class IntegerDelegate
    : public QStyledItemDelegate {
public:
    explicit
    IntegerDelegate( QObject * parent = nullptr )
        : QStyledItemDelegate( parent )
      { }

    ~IntegerDelegate()
      {
          // std::cerr << "delete IntegerDelegate" << std::endl;
      }


    QWidget * createEditor( QWidget * parent,
                            const QStyleOptionViewItem & /*option*/,
                            const QModelIndex & /*index*/ ) const override
    {
        QSpinBox * editor = new QSpinBox( parent );
        editor->setFrame( false );
        editor->setMinimum( 0 );
        editor->setMaximum( 100 );
        return editor;
    }

    void setEditorData( QWidget * editor,
                        const QModelIndex & index ) const override
    {
        QSpinBox * spin_box = static_cast< QSpinBox * >( editor );
        int value = index.model()->data( index, Qt::EditRole ).toInt();
        spin_box->setValue( value );
    }

    void setModelData( QWidget * editor,
                       QAbstractItemModel * model,
                       const QModelIndex & index ) const override
    {
        QSpinBox * spin_box = static_cast< QSpinBox * >( editor );
        int value = spin_box->value();
        model->setData( index, value, Qt::EditRole );
    }

    void updateEditorGeometry( QWidget * editor,
                               const QStyleOptionViewItem & option,
                               const QModelIndex & /*index*/ ) const override
    {
        editor->setGeometry( option.rect );
    }
};



/*-------------------------------------------------------------------*/
LabelEditorWindow::LabelEditorWindow( MainData & main_data,
                                      QWidget * parent )
    : QMainWindow( parent ),
      M_main_data( main_data )
{
    createView();
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
LabelEditorWindow::createView()
{
    M_table_view = new QTableWidget();

    //M_tree_view->setRootIsDecorated( false ); // for QTreeView

    M_table_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_table_view->setSelectionMode( QAbstractItemView::SingleSelection );
    M_table_view->setSortingEnabled( true );
    M_table_view->setAlternatingRowColors( true );

    // settings for drag & drop
    // M_table_view->setAutoScroll( true ); // need for drag&drop
    // M_table_view->setDragEnabled( true );
    // M_table_view->setDragDropMode( QAbstractItemView::DragDrop );
    // M_table_view->setDragDropOverwriteMode( false );
    // M_table_view->setDropIndicatorShown( true );
    // M_table_view->viewport()->setAcceptDrops( true );

    //M_table_view->setEditTriggers( QAbstractItemView::NoEditTriggers );

    connect( M_table_view, SIGNAL( itemChanged( QTableWidgetItem * ) ),
             this, SLOT( slotItemChanged( QTableWidgetItem * ) ) );
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

    //M_tool_bar->addAction( tr( "Clear" ), this, SLOT( clearTable() ) );

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
LabelEditorWindow::clearTable()
{
    M_table_view->clear();
    M_table_view->setRowCount( 0 );
    M_table_view->setColumnCount( 0 );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::initTable()
{
   if ( ! M_features_log )
    {
        return;
    }

   //M_table_view->clear();
   clearTable();

    M_table_view->setColumnCount( 1 // rank
                                  + 1 // score
                                  + M_features_log->floatFeaturesSize()
                                  + M_features_log->catFeaturesSize() );

    // set header
    {
        QStringList names;
        names << "RankScore";
        names << "Score";
        for ( const std::string & s : M_features_log->featureNames() )
        {
            names << QString::fromStdString( s );
        }
        M_table_view->setHorizontalHeaderLabels( names );
    }

    M_table_view->setItemDelegateForColumn( 0, new IntegerDelegate( M_table_view ) );

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

    if ( M_features_log->timedMap().empty() )
    {
        return;
    }
#if 0
    // get the current field data
    MonitorViewData::ConstPtr view = M_main_data.getCurrentViewData();
    if ( ! view )
    {
        std::cerr << "(LabelEditorWindow::updateTableContents) No monitor view." << std::endl;
        return;
    }

    // get the grouped data
    WholeFeaturesLog::Map::const_iterator it = M_features_log->timedMap().find( view->time() );
    if ( it == M_features_log->timedMap().end() )
    {
        std::cerr << "(LabelEditorWindow::updateTableContents) No timed data." << std::endl;
        return;
    }
#else
    WholeFeaturesLog::Map::const_iterator it = M_features_log->timedMap().begin();
#endif
    if ( ! it->second )
    {
        std::cerr << "(LabelEditorWindow::updateTableContents) No grouped data." << std::endl;
        return;
    }

    M_table_view->setRowCount( it->second->featuresList().size() );
    std::cerr << "(LabelEditorWindow::updateTableContents) rowCount = " << M_table_view->rowCount() << std::endl;

    // std::vector< FeaturesLog::Ptr > features_list = it->second->featuresList();
    // std::sort( features_list.begin(), features_list.end(),
    //            []( const FeaturesLog::Ptr & lhs,
    //                const FeaturesLog::Ptr & rhs )
    //              {
    //                  return lhs->label() > rhs->label();
    //              } );

    // loop in the group
    int row_count = 0;
    //for ( const FeaturesLog::ConstPtr & f : features_list )
    for ( const FeaturesLog::ConstPtr & f : it->second->featuresList() )
    {
        int column_count = 0;

        // rank label
        {
            QTableWidgetItem * item = new QTableWidgetItem();
            item->setData( Qt::DisplayRole, f->rankLabel() );
            item->setFlags( Qt::ItemIsSelectable |  Qt::ItemIsEnabled | Qt::ItemIsEditable );
            M_table_view->setItem( row_count, column_count, item );
            ++column_count;
        }
        // score
        {
            QTableWidgetItem * item = new QTableWidgetItem();
            item->setData( Qt::DisplayRole, f->score() );
            item->setFlags( Qt::ItemIsSelectable |  Qt::ItemIsEnabled );
            M_table_view->setItem( row_count, column_count, item );
            ++column_count;
        }
        // float features
        for ( const double v : f->floatFeatures() )
        {
            QTableWidgetItem * item = new QTableWidgetItem();
            item->setData( Qt::DisplayRole, v );
            item->setFlags( Qt::ItemIsSelectable |  Qt::ItemIsEnabled );
            M_table_view->setItem( row_count, column_count, item );
            ++column_count;
        }
        // cat features
        for ( const std::string & v : f->catFeatures() )
        {
            QTableWidgetItem * item = new QTableWidgetItem();
            item->setData( Qt::DisplayRole, QString::fromStdString( v ) );
            item->setFlags( Qt::ItemIsSelectable |  Qt::ItemIsEnabled );
            M_table_view->setItem( row_count, column_count, item );
            ++column_count;
        }

        ++row_count;
    }

    // sort by the score column
    M_table_view->sortByColumn( SCORE_COLUMN, Qt::DescendingOrder );
    // sort by the rank label column
    M_table_view->sortByColumn( RANK_COLUMN, Qt::DescendingOrder );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::slotItemChanged( QTableWidgetItem * item )
{
    if ( item->column() != RANK_COLUMN )
    {
        std::cerr << "(LabelEditorWindow::slotItemChanged) Unexpected column " << item->column() << std::endl;
        return;
    }

    std::cerr << "(LabelEditorWindow::slotItemChanged) Changed row " << item->row() << std::endl;

}
