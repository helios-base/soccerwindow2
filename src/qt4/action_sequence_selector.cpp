// -*-c++-*-

/*!
  \file action_sequence_selector.h
  \brief action sequence data selector widget Source File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

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

#include "action_sequence_selector.h"

#include "agent_id.h"
#include "main_data.h"
#include "action_sequence_description.h"
#include "action_sequence_log_parser.h"
#include "debug_log_data.h"
#include "options.h"

#include <rcsc/common/logger.h>
#include <rcsc/game_time.h>

#include <boost/shared_ptr.hpp>
//#include <boost/format.hpp>

#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>


namespace {

const int ID_COLUMN = 0;
const int VALUE_COLUMN = 1;
const int LENGTH_COLUMN = 2;
const int SEQ_COLUMN = 3;
const int DESC_COLUMN = 4;

}

/*-------------------------------------------------------------------*/
/*!

*/
ActionSequenceSelector::ActionSequenceSelector( QWidget * parent,
                                                MainData & main_data )
    : QFrame( parent ),
      M_main_data( main_data )
{
    //
    // set default size
    //
    //this->resize( 800, 600 );

    QVBoxLayout * top_layout = new QVBoxLayout();
    top_layout->setContentsMargins( 0, 0, 0, 0 );
    this->setLayout( top_layout );

    M_info_label = new QLabel();
    //M_info_label->setAlignment( Qt::AlignRight );

    top_layout->addWidget( M_info_label );

    M_tree_view = new QTreeWidget();

    M_tree_view->setSelectionBehavior( QAbstractItemView::SelectRows );
    M_tree_view->setSelectionMode( QAbstractItemView::SingleSelection );
    M_tree_view->setSortingEnabled( true );
    M_tree_view->setAlternatingRowColors( true );
    M_tree_view->setAutoScroll( true );
    M_tree_view->setRootIsDecorated( false );

    {
        QTreeWidgetItem * h = M_tree_view->headerItem();
        h->setText( ID_COLUMN, tr( "ID" ) );
        h->setText( VALUE_COLUMN, tr( "Value" ) );
        h->setText( LENGTH_COLUMN, tr( "Len" ) );
        h->setText( SEQ_COLUMN, tr( "Seq" ) );
        h->setText( DESC_COLUMN, tr( "Description" ) );
    }
    M_tree_view->header()->setMovable( false );

    top_layout->addWidget( M_tree_view );


    connect( M_tree_view, SIGNAL( itemSelectionChanged() ),
             this, SLOT( slotItemSelectionChanged() ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
ActionSequenceSelector::~ActionSequenceSelector()
{
    // std::cerr << "delete ActionSequenceSelector" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceSelector::showEvent( QShowEvent * event )
{
    QFrame::showEvent( event );

    updateListView();
    //updateTreeView();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ActionSequenceSelector::updateSequenceData()
{
    //std::cerr << "(ActionSequenceSelector::updateDataListView)" << std::endl;

    const AgentID pl = Options::instance().selectedAgent();
    if ( pl.isNull() )
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "player not selected" ),
                               QMessageBox::Ok, QMessageBox::NoButton );
        return false;
    }

    const boost::shared_ptr< const DebugLogData > data = M_main_data.debugLogHolder().getData( pl.unum() );

    if ( ! data )
    {
        std::cerr << __FILE__ << ": (updateSequenceData) "
                  << "no debug log data. unum = "
                  << pl.unum() << std::endl;
        return false;
    }

    std::stringstream buf;

    for ( DebugLogData::TextCont::const_iterator it = data->textCont().begin(),
              end = data->textCont().end();
          it != end;
          ++ it )
    {
        if ( it->level_ & rcsc::Logger::ACTION_CHAIN )
        {
            buf << it->msg_;
        }
    }

    ActionSequenceHolder::ConstPtr seqs = ActionSequenceLogParser::parse( buf );

    if ( ! seqs )
    {
        std::cerr << __FILE__ << ": (updateSequenceData) "
                  << "ERROR on parsing action sequence log!" << std::endl;
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "Error on parsing action sequence log!" ),
                               QMessageBox::Ok, QMessageBox::NoButton );
        return false;
    }

    M_main_data.setActionSequenceHolder( M_main_data.debugLogHolder().currentTime(), seqs );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ActionSequenceSelector::updateListView()
{
    //std::cerr << "(ActionSequenceSelector::updateDataListView)" << std::endl;

    if ( ! updateSequenceData() )
    {
        return;
    }

    const ActionSequenceHolder::ConstPtr & seqs = M_main_data.actionSequenceHolder();
    if ( ! seqs )
    {
        return;
    }

    M_tree_view->clear();

    //
    // print sequence data
    //

    int hits = 0;

    for ( ActionSequenceHolder::Cont::const_iterator it = seqs->data().begin(),
              end = seqs->data().end();
          it != end;
          ++it )
    {
        const ActionSequenceDescription & seq = *(it->second);

        ++hits;

        std::ostringstream buf;
        QString seq_str;

        if ( seq.actions().empty() )
        {
            buf << "\nempty sequence";
        }
        else
        {
            for ( std::vector< ActionDescription >::const_iterator a = seq.actions().begin(),
                      a_end = seq.actions().end();
                  a != a_end;
                  ++a )
            {
                seq_str += QString::number( a->id() );
                seq_str += ',';
                buf << "\n[" << a->description() << "]";
            }

            seq_str.chop( 1 );
        }

        // if ( ! seq.evaluationDescription().empty() )
        // {
        //     buf << '\n';
        //     for ( std::vector< std::string >::const_iterator e = seq.evaluationDescription().begin(),
        //               e_end = seq.evaluationDescription().end();
        //           e != e_end;
        //           ++e )
        //     {
        //         buf << "                    "
        //             << "(eval) " << *e << '\n';
        //     }
        // }

        buf << '\n';

        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setData( ID_COLUMN, Qt::DisplayRole, seq.id() );
        item->setData( VALUE_COLUMN, Qt::DisplayRole, seq.value() );
        item->setData( LENGTH_COLUMN, Qt::DisplayRole, static_cast< int >( seq.actions().size() ) );
        item->setText( SEQ_COLUMN, seq_str );
        item->setText( DESC_COLUMN, QString::fromStdString( buf.str() ) );
        M_tree_view->addTopLevelItem( item );
    }

    const rcsc::GameTime & current = M_main_data.debugLogHolder().currentTime();

    M_info_label->setText( tr( "Time=[%1, %2] %3 hits" )
                           .arg( current.cycle() )
                           .arg( current.stopped() )
                           .arg( hits ) );

    M_tree_view->sortItems( VALUE_COLUMN, Qt::DescendingOrder );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceSelector::updateTreeView()
{

    if ( ! updateSequenceData() )
    {
        return;
    }

    const ActionSequenceHolder::ConstPtr & seqs = M_main_data.actionSequenceHolder();
    if ( ! seqs )
    {
        return;
    }

    M_tree_view->clear();

    std::map< int, QTreeWidgetItem * > item_map;


    int hits = 0;

    for ( ActionSequenceHolder::Cont::const_iterator it = seqs->data().begin(),
              end = seqs->data().end();
          it != end;
          ++it )
    {
        const ActionSequenceDescription & seq = *(it->second);
        ++hits;

        const std::vector< ActionDescription >::const_iterator a_end = seq.actions().end();
        std::vector< ActionDescription >::const_iterator parent_a = a_end;
        QTreeWidgetItem * parent = static_cast< QTreeWidgetItem * >( 0 );

        for ( std::vector< ActionDescription >::const_iterator a = seq.actions().begin();
              a != a_end;
              ++a )
        {
            std::map< int, QTreeWidgetItem * >::iterator m = item_map.find( a->id() );
            if ( m == item_map.end() )
            {
                break;
            }
            parent_a = a;
            parent = m->second;
        }

        std::vector< ActionDescription >::const_iterator new_a;
        if ( parent_a == a_end )
        {
            new_a = seq.actions().begin();
        }
        else
        {
            new_a = parent_a + 1;
        }

        for ( ; new_a != a_end; ++new_a )
        {
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setData( ID_COLUMN, Qt::DisplayRole, new_a->id() );
            item->setData( VALUE_COLUMN, Qt::DisplayRole, seq.value() );
            item->setData( LENGTH_COLUMN, Qt::DisplayRole,
                           static_cast< int >( std::distance( seq.actions().begin(), new_a ) + 1 ) );

            QString text = tr( "[" );
            text += QString::fromStdString( new_a->description() );
            text += tr( "]" );
            item->setText( DESC_COLUMN, text );

            if ( parent )
            {
                parent->addChild( item );
            }
            else
            {
                M_tree_view->addTopLevelItem( item );
            }

            item_map.insert( std::pair< int, QTreeWidgetItem * >( new_a->id(), item ) );
            parent = item;
        }
    }

    M_tree_view->expandAll();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceSelector::clearSelection()
{
    if ( ! M_tree_view->selectedItems().isEmpty() )
    {
        M_tree_view->clearSelection();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceSelector::slotItemSelectionChanged()
{
    QTreeWidgetItem * item = M_tree_view->currentItem();
    if ( ! item )
    {
        std::cerr << "(ActionSequenceSelector) NULL" << std::endl;
        return;
    }

    bool ok = false;
    int id = item->data( ID_COLUMN, Qt::DisplayRole ).toInt( &ok );
    if ( ok )
    {
        emit selected( id );
    }
}
