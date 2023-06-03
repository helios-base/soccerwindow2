// -*-c++-*-

/*!
  \file ranking_editor_window.h
  \brief ranking editor window Source File.
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

#include "monitor_view_data.h"
#include "main_data.h"

#include <iostream>
#include <fstream>

#include "xpm/open.xpm"
#include "xpm/save.xpm"

using namespace rcsc;

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
      M_main_data( main_data )
{
    this->setWindowTitle( tr( "Ranking Editor" ) );

    createWidgets();

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
LabelEditorWindow::createWidgets()
{
    M_splitter = new QSplitter( Qt::Horizontal );
    M_splitter->setChildrenCollapsible( false );

    createTimeView();
    M_splitter->addWidget( M_time_view );

    createLabelView();
    M_splitter->addWidget( M_label_view );

    createValuesView();
    M_splitter->addWidget( M_values_view );

    this->setCentralWidget( M_splitter );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::createTimeView()
{
    M_time_view = new QTreeWidget();

    M_time_view->setRootIsDecorated( false ); // for QTreeView

    M_time_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_time_view->setSelectionMode( QAbstractItemView::SingleSelection );
    //M_time_view->setSortingEnabled( true );
    M_time_view->setAlternatingRowColors( true );

    {
        QTreeWidgetItem * h = M_time_view->headerItem();
        h->setText( 0, tr( "Time" ) );
    }

    connect( M_time_view, SIGNAL( itemSelectionChanged() ),
             this, SLOT( selectTimeItem() ) );
}


/*-------------------------------------------------------------------*/
void
LabelEditorWindow::createLabelView()
{
    M_label_view = new QTreeWidget();

    M_label_view->setRootIsDecorated( false ); // for QTreeView

    M_label_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_label_view->setSelectionMode( QAbstractItemView::SingleSelection );
    M_label_view->setSortingEnabled( true );
    M_label_view->setAlternatingRowColors( true );

    // settings for drag & drop
    // M_label_view->setAutoScroll( true ); // need for drag&drop
    // M_label_view->setDragEnabled( true );
    // M_label_view->setDragDropMode( QAbstractItemView::DragDrop );
    // M_label_view->setDragDropOverwriteMode( false );
    // M_label_view->setDropIndicatorShown( true );
    // M_label_view->viewport()->setAcceptDrops( true );

    M_label_view->setEditTriggers( QAbstractItemView::NoEditTriggers ); // handled only by double click

    {
        QTreeWidgetItem * h = M_label_view->headerItem();
        h->setText( INDEX_COLUMN, tr( "Index" ) );
        h->setText( RANK_COLUMN, tr( "(EditScore)" ) );
        h->setText( SCORE_COLUMN, tr( "OriginalScore") );
        //h->setText( DESC_COLUMN, tr( "Description" ) );
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    M_label_view->header()->setSectionsMovable( false );
#else
    M_label_view->header()->setMovable( false );
    //M_label_view->header()->setResizeMode( QHeaderView::ResizeToContents );
    //M_label_view->header()->setSortIndicatorShown( false );
#endif

    {
        const QFontMetrics metrics = M_label_view->fontMetrics();
        M_label_view->setColumnWidth( INDEX_COLUMN, metrics.width( tr( "0000000--" ) ) );
        M_label_view->setColumnWidth( RANK_COLUMN, metrics.width( tr( "(EditScore)--" ) ) );
        M_label_view->setColumnWidth( SCORE_COLUMN, metrics.width( tr( "OriginalScore--" ) ) );
    }
    {
        RankEditDelegate * delegate = new RankEditDelegate( M_label_view );
        M_label_view->setItemDelegate( delegate );
    }

    connect( M_label_view, SIGNAL( itemSelectionChanged() ),
             this, SLOT( selectLabelItem() ) );
    connect( M_label_view, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SLOT( slotLabelItemDoubleClicked( QTreeWidgetItem *, int ) ) );
    connect( M_label_view, SIGNAL( itemChanged( QTreeWidgetItem *, int ) ),
             this, SLOT( slotLabelItemChanged( QTreeWidgetItem *, int ) ) );
}


/*-------------------------------------------------------------------*/
void
LabelEditorWindow::createValuesView()
{
    M_values_view = new QTreeWidget();

    M_values_view->setRootIsDecorated( false ); // for QTreeView

    //M_values_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_values_view->setSelectionMode( QAbstractItemView::NoSelection );
    M_values_view->setSortingEnabled( true );
    M_values_view->setAlternatingRowColors( true );


    {
        QTreeWidgetItem * h = M_values_view->headerItem();
        h->setText( 0, tr( "Name" ) );
        h->setText( 1, tr( "Value" ) );
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    M_values_view->header()->setSectionsMovable( false );
#else
    M_values_view->header()->setMovable( false );
    //M_values_view->header()->setResizeMode( QHeaderView::ResizeToContents );
    //M_values_view->header()->setSortIndicatorShown( false );
#endif
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

    if ( ! M_main_data.openFeaturesLog( filepath.toStdString() ) )
    {
        QMessageBox::warning( this,
                              tr( "Warning" ),
                              tr( "Could not open the file \n" ) + filepath,
                              QMessageBox::Ok,
                              QMessageBox::NoButton );
        return false;

    }

    initView();

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
LabelEditorWindow::initView()
{
    if ( ! M_main_data.featuresLog() )
    {
        return;
    }

    M_time_view->clear();
    M_label_view->clear();
    M_values_view->clear();

    initTimeView();
    initValuesView();
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::initTimeView()
{
    if ( ! M_main_data.featuresLog() )
    {
        std::cerr << "(LabelEditorWindow::initTimeView) no features log" << std::endl;
        return;
    }

    M_time_view->clear();

    for ( const WholeFeaturesLog::Map::value_type & i : M_main_data.featuresLog()->timedMap() )
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setData( 0, Qt::DisplayRole,
                       QString( "%1,%2" )
                       .arg( i.first.cycle(), 5, 10, QChar( '0' ) )
                       .arg( i.first.stopped(), 3, 10, QChar( '0' ) ) );
        M_time_view->addTopLevelItem( item );
    }

    M_time_view->sortItems( INDEX_COLUMN, Qt::AscendingOrder );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::initValuesView()
{
    WholeFeaturesLog::ConstPtr features_log = M_main_data.featuresLog();

    if ( ! features_log )
    {
        std::cerr << "(LabelEditorWindow::initValuesView) no features log" << std::endl;
        return;
    }

    M_values_view->clear();

    const size_t feature_size = features_log->floatFeaturesSize() + features_log->catFeaturesSize();
    for ( size_t i = 0; i < feature_size; ++i )
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        M_values_view->addTopLevelItem( item );
    }

    // set name column
    int row = 0;
    if ( features_log->featureNames().size() == feature_size )
    {
        for ( const std::string & name : features_log->featureNames() )
        {
            QTreeWidgetItem * item = M_values_view->topLevelItem( row );
            if ( item )
            {
                item->setData( 0, Qt::DisplayRole, QString::fromStdString( name ) );
            }
            ++row;
        }
    }
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::selectTimeItem()
{
    QTreeWidgetItem * item = M_time_view->currentItem();
    if ( ! item )
    {
        std::cerr << "(LabelEditorWindow::selectTimeItem) selected item not found." << std::endl;
        return;
    }

    const std::string time_string = item->text( 0 ).toStdString();
    int cycle, stopped;
    if ( std::sscanf( time_string.c_str(), " %d,%d", &cycle, &stopped ) != 2 )
    {
        std::cerr << "(LabelEditorWindow::selectTimeItem) Could not parse time values." << std::endl;
        return;
    }

    M_main_data.setSelectedFeaturesGroupTime( GameTime( cycle, stopped ) );

    updateLabelView();

    emit cycleChanged( cycle );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::updateLabelView()
{
    WholeFeaturesLog::ConstPtr features_log = M_main_data.featuresLog();
    if ( ! features_log )
    {
        return;
    }

    M_label_view->clear();

    if ( features_log->timedMap().empty() )
    {
        return;
    }

    GroupedFeaturesLog::ConstPtr group = features_log->findGroup( M_main_data.selectedFeaturesGroupTime() );

    if ( ! group )
    {
        std::cerr << "(LabelEditorWindow::updateTreeView) No grouped data." << std::endl;
        return;
    }

    // int index = 0;
    for ( const FeaturesLog::ConstPtr & f : group->featuresList() )
    {
        // ++index;

        QTreeWidgetItem * item = new QTreeWidgetItem();
        //item->setData( INDEX_COLUMN, Qt::DisplayRole, index );
        item->setData( INDEX_COLUMN, Qt::DisplayRole, f->index() );
        item->setData( RANK_COLUMN,  Qt::DisplayRole, f->rankLabel() );
        item->setData( SCORE_COLUMN, Qt::DisplayRole, f->score() );

        Qt::ItemFlags flags = item->flags();
        flags |= Qt::ItemIsEditable;
        flags &= ~Qt::ItemIsDropEnabled;
        item->setFlags( flags );

        M_label_view->addTopLevelItem( item );
    }

    M_label_view->sortItems( INDEX_COLUMN, Qt::DescendingOrder );
    M_label_view->sortItems( RANK_COLUMN,  Qt::DescendingOrder );
    M_label_view->sortItems( SCORE_COLUMN, Qt::DescendingOrder );
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::selectLabelItem()
{
    QTreeWidgetItem * item = M_label_view->currentItem();
    if ( ! item )
    {
        std::cerr << "(LabelEditorWindow::selectLabelItem) selected item not found." << std::endl;
        return;
    }

    bool ok = false;
    int idx = item->data( INDEX_COLUMN, Qt::DisplayRole ).toInt( &ok );
    if ( ok )
    {
        // std::cerr << "select index " << idx << std::endl;
        showFeatureValues( idx );
    }
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::slotLabelItemDoubleClicked( QTreeWidgetItem * item,
                                               int column )
{
    if ( ! item )
    {
        return;
    }

    if ( column == RANK_COLUMN )
    {
        M_label_view->editItem( item, RANK_COLUMN );
    }
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::slotLabelItemChanged( QTreeWidgetItem * item,
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

    std::cerr << "(LabelEditorWindow::slotLabelItemChanged) index = " << idx << " column=" << column << std::endl;
}

/*-------------------------------------------------------------------*/
void
LabelEditorWindow::showFeatureValues( const int index )
{
    WholeFeaturesLog::ConstPtr features_log = M_main_data.featuresLog();
    if ( ! features_log )
    {
        std::cerr << "(LabelEditorWindow::showFeatureValues) no features log" << std::endl;
        return;
    }

    GroupedFeaturesLog::ConstPtr selected_group = features_log->findGroup( M_main_data.selectedFeaturesGroupTime() );
    if ( ! selected_group )
    {
        std::cerr << "(LabelEditorWindow::showFeatureValues) no selected group" << std::endl;
        return;
    }

    if ( index < 0 )
    {
        std::cerr << "(LabelEditorWindow::showFeatureValues) Illegal index " << index << std::endl;
        return;
    }

    const FeaturesLog::ConstPtr f = selected_group->findFeaturesLog( index );
    if ( ! f )
    {
        std::cerr << "(LabelEditorWindow::showFeatureValues) Null features log." << std::endl;
        return;
    }

    int row = 0;
    for ( const double v : f->floatFeatures() )
    {
        QTreeWidgetItem * item = M_values_view->topLevelItem( row );
        if ( item )
        {
            item->setData( 1, Qt::DisplayRole, v );
        }
        ++row;
    }

    for ( const std::string & v : f->catFeatures() )
    {
        QTreeWidgetItem * item = M_values_view->topLevelItem( row );
        if ( item )
        {
            item->setData( 1, Qt::DisplayRole, QString::fromStdString( v ) );
        }
        ++row;
    }

    emit featuresLogSelected( f->index() );
}