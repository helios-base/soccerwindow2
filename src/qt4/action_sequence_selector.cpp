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

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>


namespace {

const int ID_COLUMN = 0;
const int VALUE_COLUMN = 1;
const int DESC_COLUMN = 2;

const char *
nth_string( int n )
{
    switch( n ) {
    case 1:
        return "st";
        break;

    case 2:
        return "nd";
        break;

    case 3:
        return "rd";
        break;

    default:
        return "th";
        break;
    }
}

}

/*-------------------------------------------------------------------*/
/*!

*/
ActionSequenceSelector::ActionSequenceSelector( QWidget * parent,
                                                const MainData & main_data )
    : QFrame( parent ),
      M_main_data( main_data )
{
    //
    // set default size
    //
    this->resize( 600, 600 );

    QVBoxLayout * top_layout = new QVBoxLayout();
    top_layout->setContentsMargins( 0, 0, 0, 0 );
    this->setLayout( top_layout );

    M_tree_view = new QTreeWidget();
    M_tree_view->setSortingEnabled( true );
    M_tree_view->sortItems( VALUE_COLUMN, Qt::DescendingOrder );

    {
        QTreeWidgetItem * h = M_tree_view->headerItem();
        h->setText( ID_COLUMN, tr( "ID" ) );
        h->setText( VALUE_COLUMN, tr( "Value" ) );
        h->setText( DESC_COLUMN, tr( "Description" ) );
    }
    M_tree_view->header()->setMovable( false );

    top_layout->addWidget( M_tree_view );
}

/*-------------------------------------------------------------------*/
/*!

*/
ActionSequenceSelector::~ActionSequenceSelector()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceSelector::showEvent( QShowEvent * event )
{
    QFrame::showEvent( event );

    updateData();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceSelector::updateData()
{
    std::cerr << "(ActionSequenceSelector::updateData)" << std::endl;

    M_tree_view->clear();

    const AgentID pl = Options::instance().selectedAgent();
    if ( pl.isNull() )
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "player not selected" ),
                               QMessageBox::Ok, QMessageBox::NoButton );
        return;
    }

    const boost::shared_ptr< const DebugLogData > data = M_main_data.debugLogHolder().getData( pl.unum() );

    if ( ! data )
    {
        std::cerr << __FILE__ << ": (updateData) "
                  << "no debug log data. unum = "
                  << pl.unum() << std::endl;
        return;
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

    const boost::shared_ptr< ActionSequenceDescriptionSet > seqs = ActionSequenceLogParser::parse( buf );

    if ( ! seqs )
    {
        std::cerr << __FILE__ << ": (updateData) "
                  << "ERROR on parsing action sequence log!" << std::endl;
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "Error on parsing action sequence log!" ),
                               QMessageBox::Ok, QMessageBox::NoButton );
        return;
    }

    //
    // print sequence data
    //

    int hits = 0;

    int nth = 1;
    int n = 1;
    double last_evaluation = 0.0;

    for ( ActionSequenceDescriptionSet::MapType::const_iterator it = seqs->getMap().begin(),
              end = seqs->getMap().end();
          it != end;
          ++it, ++n )
    {
        const double evaluation = it->first;
        const ActionSequenceDescription & seq = *(it->second);

        if ( evaluation != last_evaluation
             || n == 1 )
        {
            nth = n;
        }
        last_evaluation = evaluation;

        ++hits;

        std::ostringstream buf;

        buf << "sequence[" << nth << nth_string( nth ) << "]: "
            << "evaluation = " << evaluation //boost::format( "%f" ) % evaluation
            << ", id = " << seq.id();

        if ( seq.actions().empty() )
        {
            buf << "\nempty sequence";
        }
        else
        {
            for ( size_t i = 0; i < seq.actions().size(); ++ i )
            {
                const ActionDescription & act = seq.actions()[i];
                buf << "\n[" << act.description() << "]";
            }
        }

        if ( ! seq.evaluationDescription().empty() )
        {
            buf << '\n';

            for ( size_t i = 0; i < seq.evaluationDescription().size(); ++ i )
            {
                buf << "                    "
                    << "(eval) " << seq.evaluationDescription()[i] << '\n';
            }
        }

        buf << '\n';

        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setData( ID_COLUMN, Qt::DisplayRole, seq.id() );
        item->setData( VALUE_COLUMN, Qt::DisplayRole, evaluation );
        item->setText( DESC_COLUMN, QString::fromStdString( buf.str() ) );
        M_tree_view->addTopLevelItem( item );
    }

    {
        QTreeWidgetItem * h = M_tree_view->headerItem();
        const rcsc::GameTime & t = M_main_data.debugLogHolder().currentTime();
        h->setText( DESC_COLUMN,
                    tr( "CYCLE [%1, %2] %3 hits" )
                    .arg( t.cycle() )
                    .arg( t.stopped() )
                    .arg( hits ) );
    }
}
