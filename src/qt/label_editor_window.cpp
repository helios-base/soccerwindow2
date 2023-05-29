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
//#include <QTableWidget>

#include "label_editor_window.h"

#include "features_log_parser.h"
#include "monitor_view_data.h"
#include "main_data.h"

#include <iostream>
#include <fstream>

#include "xpm/open.xpm"
#include "xpm/save.xpm"

namespace {
constexpr int INDEX_COLUMN = 0;
constexpr int RANK_COLUMN = 1;
constexpr int SCORE_COLUMN = 2;
}

class RankEditDelegate
    :  public QItemDelegate
{
public:
    explicit RankEditDelegate( QObject* parent = nullptr )
        : QItemDelegate( parent )
    { }

    QWidget * createEditor( QWidget * parent,
                            const QStyleOptionViewItem & option,
                            const QModelIndex & index ) const override
    {
        if ( index.column() == RANK_COLUMN )
        {
            QSpinBox * editor = new QSpinBox( parent );
            editor->setFrame( false );
            editor->setMinimum( 0 );
            editor->setMaximum( 100 );
            return editor;
        }
        return QItemDelegate::createEditor( parent, option, index );
    }

    void setEditorData( QWidget * editor,
                        const QModelIndex & index) const override
    {
        if ( index.column() == RANK_COLUMN )
        {
            QSpinBox * spin_box = static_cast< QSpinBox * >( editor );
            int value = index.model()->data( index, Qt::EditRole ).toInt();
            spin_box->setValue( value );
        }
        else
        {
            QItemDelegate::setEditorData( editor, index );
        }
    }

    void setModelData( QWidget * editor,
                       QAbstractItemModel * model,
                       const QModelIndex & index) const override
    {
        if ( index.column() == RANK_COLUMN )
        {
            QSpinBox * spin_box = static_cast< QSpinBox * >( editor );
            int value = spin_box->value();
            model->setData( index, value, Qt::EditRole );
        }
        else
        {
            QItemDelegate::setModelData( editor, model, index );
        }
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
      M_main_data( main_data ),
      M_selected_time( -1, 0 )
{
    createTreeView();

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
LabelEditorWindow::createTreeView()
{
    M_tree_view = new QTreeWidget();

    M_tree_view->setRootIsDecorated( false ); // for QTreeView

    M_tree_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_tree_view->setSelectionMode( QAbstractItemView::SingleSelection );
    M_tree_view->setSortingEnabled( true );
    M_tree_view->setAlternatingRowColors( true );

    // settings for drag & drop
    // M_tree_view->setAutoScroll( true ); // need for drag&drop
    // M_tree_view->setDragEnabled( true );
    // M_tree_view->setDragDropMode( QAbstractItemView::DragDrop );
    // M_tree_view->setDragDropOverwriteMode( false );
    // M_tree_view->setDropIndicatorShown( true );
    // M_tree_view->viewport()->setAcceptDrops( true );

    M_tree_view->setEditTriggers( QAbstractItemView::NoEditTriggers ); // handled only by double click

    {
        QTreeWidgetItem * h = M_tree_view->headerItem();
        h->setText( INDEX_COLUMN, tr( "Index" ) );
        h->setText( RANK_COLUMN, tr( "RankScore" ) );
        h->setText( SCORE_COLUMN, tr( "Score") );
        //h->setText( DESC_COLUMN, tr( "Description" ) );
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    M_tree_view->header()->setSectionsMovable( false );
#else
    M_tree_view->header()->setMovable( false );
    //M_tree_view->header()->setResizeMode( QHeaderView::ResizeToContents );
    //M_tree_view->header()->setSortIndicatorShown( false );
#endif

    RankEditDelegate * delegate = new RankEditDelegate( M_tree_view );
    M_tree_view->setItemDelegate( delegate );

    connect( M_tree_view, SIGNAL( itemSelectionChanged() ),
             this, SLOT( slotItemSelectionChanged() ) );
    connect( M_tree_view, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SLOT( slotItemDoubleClicked( QTreeWidgetItem *, int ) ) );
    connect( M_tree_view, SIGNAL( itemChanged( QTreeWidgetItem *, int ) ),
             this, SLOT( slotItemChanged( QTreeWidgetItem *, int ) ) );

    this->setCentralWidget( M_tree_view );
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
void
LabelEditorWindow::clearAll()
{
    M_tree_view->clear();
    M_features_log.reset();
    M_selected_time.assign( -1, 0 );
    M_selected_group.reset();
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

    clearAll();

    FeaturesLogParser parser;
    M_features_log = parser.parse( fin );

    if ( ! M_features_log )
    {
        std::cerr << "Null Features Log" << std::endl;
        return false;
    }

    //std::cout << "features log size = " << M_features_log->timedMap().size() << std::endl;

    initTreeView();

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
LabelEditorWindow::initTreeView()
{
    if ( ! M_features_log )
    {
        return;
    }

    updateTreeView();
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::updateTreeView()
{
    if ( ! M_features_log )
    {
        return;
    }

    M_tree_view->clear();
    M_selected_group.reset();

    if ( M_features_log->timedMap().empty() )
    {
        return;
    }

    WholeFeaturesLog::Map::const_iterator it = M_features_log->timedMap().find( M_selected_time );
    if ( it == M_features_log->timedMap().end() )
    {
        it = M_features_log->timedMap().begin();
    }

    if ( ! it->second )
    {
        std::cerr << "(LabelEditorWindow::updateTreeView) No grouped data." << std::endl;
        return;
    }

    M_selected_group = it->second;

    int index = 0;
    for ( const FeaturesLog::ConstPtr & f : it->second->featuresList() )
    {
        ++index;

        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setData( INDEX_COLUMN, Qt::DisplayRole, index );
        item->setData( RANK_COLUMN,  Qt::DisplayRole, f->rankLabel() );
        item->setData( SCORE_COLUMN, Qt::DisplayRole, f->score() );

        Qt::ItemFlags flags = item->flags();
        flags |= Qt::ItemIsEditable;
        flags &= ~Qt::ItemIsDropEnabled;
        item->setFlags( flags );

        M_tree_view->addTopLevelItem( item );
    }

    M_tree_view->sortItems( INDEX_COLUMN, Qt::DescendingOrder );
    M_tree_view->sortItems( RANK_COLUMN,  Qt::DescendingOrder );
    M_tree_view->sortItems( SCORE_COLUMN, Qt::DescendingOrder );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::slotItemSelectionChanged()
{
    QTreeWidgetItem * item = M_tree_view->currentItem();
    if ( ! item )
    {
        std::cerr << "(LabelEditorWindow::slotItemSelectionChanged) selected item not found." << std::endl;
        return;
    }

    bool ok = false;
    int idx = item->data( INDEX_COLUMN, Qt::DisplayRole ).toInt( &ok );
    if ( ok )
    {
        std::cerr << "select index " << idx << std::endl;
        showFeaturesValue( idx );
    }
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::slotItemDoubleClicked( QTreeWidgetItem * item,
                                          int column )
{
    if ( ! item )
    {
        return;
    }

    if ( column == RANK_COLUMN )
    {
        M_tree_view->editItem( item, RANK_COLUMN );
    }
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::slotItemChanged( QTreeWidgetItem * item,
                                    int column )
{
    if ( ! item ) return;
    if ( column != RANK_COLUMN ) return;

    bool ok = false;
    int idx = item->data( INDEX_COLUMN, Qt::DisplayRole ).toInt( &ok );
    if ( ! ok )
    {
        return;
    }

    std::cerr << "(LabelEditorWindow::slotItemChanged) index = " << idx << " column=" << column << std::endl;
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::showFeaturesValue( const int index )
{
    std::cerr << "(LabelEditorWindow::showDetailDialog) index = " << index << std::endl;

    if ( ! M_selected_group )
    {
        std::cerr << "(LabelEditorWindow::showDetailDialog) no selected group" << std::endl;
        return;
    }

    if ( index < 0
         || (int)M_selected_group->featuresList().size() <= index )
    {
        std::cerr << "(LabelEditorWindow::showDetailDialog) Illegal index " << index << std::endl;
        return;
    }

    const FeaturesLog::ConstPtr f = M_selected_group->featuresList().at( index - 1 );
    if ( ! f )
    {
        std::cerr << "(LabelEditorWindow::showDetailDialog) Null features log." << std::endl;
        return;
    }



}
