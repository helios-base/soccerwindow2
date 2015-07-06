// -*-c++-*-

/*!
  \file action_sequence_selector.h
  \brief action sequence data selector widget Source File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA, Hidehisa AKIYAMA

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

#include <QtGui>

#include "action_sequence_tree_view.h"


/*-------------------------------------------------------------------*/
/*!

*/
ActionSequenceTreeView::ActionSequenceTreeView( QWidget * parent )
    : QTreeWidget( parent )
{
    //this->setContextMenuPolicy( Qt::CustomContextMenu );
    this->setSelectionBehavior( QAbstractItemView::SelectRows );
    this->setSelectionMode( QAbstractItemView::SingleSelection );
    this->setSortingEnabled( true );
    this->setAlternatingRowColors( true );
    this->setAutoScroll( true );
    this->setRootIsDecorated( false );
#if 0
    this->setDragEnabled( true );
    this->setDropIndicatorShown( true );
    this->setDragDropMode( QAbstractItemView::InternalMove );
    this->viewport()->setAcceptDrops( true );
#endif
    this->setEditTriggers( QAbstractItemView::NoEditTriggers ); // handled only by double click
    //this->setEditTriggers( QAbstractItemView::SelectedClicked );

    {
        QTreeWidgetItem * h = this->headerItem();
        h->setText( ID_COLUMN, tr( "ID" ) );
        h->setText( VALUE_COLUMN, tr( "Value" ) );
        h->setText( LENGTH_COLUMN, tr( "Len" ) );
        h->setText( SEQ_COLUMN, tr( "Seq" ) );
        h->setText( DESC_COLUMN, tr( "Description" ) );
    }

    this->header()->setMovable( false );
    //this->header()->setResizeMode( QHeaderView::ResizeToContents );
    //this->header()->setSortIndicatorShown( false );

    const QFontMetrics metrics = this->fontMetrics();
    this->setColumnWidth( ID_COLUMN, metrics.width( tr( "00000" ) ) );
    this->setColumnWidth( VALUE_COLUMN, metrics.width( tr( "000000.000" ) ) );
    this->setColumnWidth( LENGTH_COLUMN, metrics.width( tr( "00" ) ) );
    this->setColumnWidth( SEQ_COLUMN, metrics.width( tr( "0000" ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
ActionSequenceTreeView::~ActionSequenceTreeView()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceTreeView::dropEvent( QDropEvent * event )
{
#if 1
    QTreeWidget::dropEvent( event );
#else
    QModelIndex drop_index = this->indexAt( event->pos() );

    if ( drop_index.isValid() == false)
    {
        event->setDropAction( Qt::IgnoreAction );
        event->accept();
        return;
    }

    QTreeWidget::dropEvent( event );

    DropIndicatorPosition drop_pos = this->dropIndicatorPosition();
    if ( dp == QAbstractItemView::BelowItem )
    {
        drop_index = drop_index.sibling(droppedIndex.row() + 1, // adjust the row number
                                            droppedIndex.column());
    }
#endif
}
