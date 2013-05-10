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

#include "main_data.h"
#include "chain_action_data.h"
#include "options.h"
#include "agent_id.h"

#include <rcsc/game_time.h>

#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>


namespace {

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
    this->setLayout( top_layout );

    M_list_view = new QListWidget();
    top_layout->addWidget( M_list_view );
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

    updateChain();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionSequenceSelector::updateChain()
{
    std::cerr << "(ActionSequenceSelector::updateChain)" << std::endl;
    //
    // clear
    //
    M_list_view->clear();


    //
    // get chain data
    //
    const boost::shared_ptr< const AgentID > pl = Options::instance().selectedAgent();
    if ( ! pl
         || pl->side() == rcsc::NEUTRAL )
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "player not selected" ),
                               QMessageBox::Ok, QMessageBox::NoButton );
        return;
    }

    const MonitorViewData::ConstPtr view_ptr = M_main_data.getCurrentViewData();
    if ( ! view_ptr )
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "no monitor view found" ),
                               QMessageBox::Ok, QMessageBox::NoButton );
        std::cerr << __FILE__ << ": (updateChain) "
                  << "no monitor view found" << std::endl;
        return;
    }

    const rcsc::GameTime current_time = view_ptr->time();

    const boost::shared_ptr< const ChainDescriptionSet > chains
        = M_main_data.chainDescriptionSetHolder().get( current_time, *pl );

    if ( ! chains )
    {
        QMessageBox::critical( this,
                               tr( "Error" ),
                               tr( "chain data not found" ),
                               QMessageBox::Ok, QMessageBox::NoButton );
        std::cerr << __FILE__ << ": (updateChain) "
                  << "chain data not found" << std::endl;
        return;
    }


    //
    // print chain data
    //

    std::ostringstream whole_buf;

    int hits = 0;

    int nth = 1;
    int n = 1;
    double last_evaluation = 0.0;

    for ( ChainDescriptionSet::MapType::const_iterator it = chains->getMap().begin(),
              end = chains->getMap().end();
          it != end;
          ++it, ++n )
    {
        const double evaluation = it->first;
        const ChainDescription & chain = *(it->second);

        if ( evaluation != last_evaluation
             || n == 1 )
        {
            nth = n;
        }
        last_evaluation = evaluation;

        ++hits;

        std::ostringstream buf;

        buf << "chain[" << nth << nth_string( nth ) << "]: "
            << "evaluation = " << boost::format( "%f" ) % evaluation
            << ", id = " << chain.chainID();

        if ( chain.actions().empty() )
        {
            buf << "\nempty chain";
        }
        else
        {
            for ( size_t i = 0; i < chain.actions().size(); ++ i )
            {
                const ActionDescription & act = chain.actions()[i];
                buf << "\n[" << act.description() << "]";
            }
        }

        if ( ! chain.evaluationDescription().empty() )
        {
            buf << '\n';

            for ( size_t i = 0; i < chain.evaluationDescription().size(); ++ i )
            {
                buf << "                    "
                    << "(eval) " << chain.evaluationDescription()[i] << '\n';
            }
        }

        buf << '\n';

        M_list_view->addItem( QString::fromStdString( buf.str() ) );

        if ( hits != 0 )
        {
            whole_buf << '\n';
        }

        whole_buf << buf.str();
    }

    std::ostringstream header_buf;
    header_buf << "CYCLE " << current_time << '\n'
               << hits << " HITS\n";

    M_list_view->insertItem( 0, QString::fromStdString( header_buf.str() ) );
}
