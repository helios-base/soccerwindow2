// -*-c++-*-

/*!
  \file chain_selector.h
  \brief action chain data selector widget Source File.
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

#include "chain_selector.h"

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
ChainSelector::ChainSelector( QWidget * parent,
                              const MainData & main_data )
    : QFrame( parent ),
      M_main_data( main_data )
{
    //
    // set default size
    //
    this->resize( 600, 600 );


    //
    // create layout
    //
    QVBoxLayout * top_layout = new QVBoxLayout();
    this->setLayout( top_layout );

    QPushButton * show_text_button = new QPushButton( "text" );
    top_layout->addWidget( show_text_button );
    connect( show_text_button, SIGNAL( clicked() ),
             this, SLOT( toggleShowTextWindow() ) );


    //
    // list
    //
    M_list = new QListWidget();
    top_layout->addWidget( M_list );


    //
    // player number chain
    //
    QHBoxLayout * unum_chain_layout = new QHBoxLayout();
    {
        unum_chain_layout->addWidget( new QLabel( "Unum filter" ) );

        // number fields
        for ( int i = 0; i < 6; ++i )
        {
            unum_chain_layout->addWidget( new QLabel( QString::number( i + 1 ) ) );
            QLineEdit * t = new QLineEdit();
            M_unum_filter.push_back( t );

            connect( t, SIGNAL( returnPressed() ), this, SLOT( updateChain() ) );
            unum_chain_layout->addWidget( t );
        }
        // clear button
        M_clear_unum_filter_button = new QPushButton( "clear" );
        connect( M_clear_unum_filter_button, SIGNAL( clicked() ),
                 this, SLOT( clearUnumFilter() ) );
        unum_chain_layout->addWidget( M_clear_unum_filter_button );
    }
    top_layout->addLayout( unum_chain_layout );

    //
    // text area
    //

    QVBoxLayout * dialog_layout = new QVBoxLayout();
    dialog_layout->setContentsMargins( 0, 0, 0, 0 );

    M_text_dialog = new QDialog( this );
    M_text_dialog->setLayout( dialog_layout );

    M_text = new QTextEdit();
    M_text->setReadOnly( true );
    dialog_layout->addWidget( M_text );

    M_text_dialog->resize( 600, 600 );
    M_text_dialog->hide();
}

/*-------------------------------------------------------------------*/
/*!

*/
ChainSelector::~ChainSelector()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
ChainSelector::showEvent( QShowEvent * event )
{
    QFrame::showEvent( event );

    updateChain();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ChainSelector::toggleShowTextWindow()
{
    M_text_dialog->setVisible( ! M_text_dialog->isVisible() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ChainSelector::clearUnumFilter()
{
    for ( size_t i = 0; i < M_unum_filter.size(); ++i )
    {
        M_unum_filter[i]->clear();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ChainSelector::updateChain()
{
    //
    // clear
    //
    M_list->clear();


    //
    // get chain data
    //
    const boost::shared_ptr< const AgentID > pl = Options::instance().selectedAgent();
    if ( ! pl || pl->side() == rcsc::NEUTRAL )
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
    // read filter
    //
    std::vector< int > unum_filter;
    for ( size_t i = 0; i < M_unum_filter.size(); ++i )
    {
        bool ok = false;
        int unum = M_unum_filter[i]->text().toInt( &ok );

        if ( ! ok
             || unum < 1
             || 11 < unum )
        {
            unum_filter.push_back( 0 );
            continue;
        }

        unum_filter.push_back( unum );
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

        //
        // check skip
        //
        bool skip = false;
        size_t i = 0;

        for ( ; i < chain.actions().size(); ++i )
        {
            const ActionDescription & act = chain.actions()[i];

            if ( i > unum_filter.size() )
            {
                break;
            }

            if ( unum_filter[i] )
            {
                if ( unum_filter[i] != act.toUnum() )
                {
                    skip = true;
                    break;
                }
            }
        }

        for ( ; i < unum_filter.size(); ++i )
        {
            if ( unum_filter[i] )
            {
                skip = true;
                break;
            }
        }

        if ( skip )
        {
            continue;
        }

        ++hits;

        std::ostringstream buf;

        buf << "chain[" << nth << nth_string( nth ) << "]: "
            << "evaluation = " << boost::format( "%f" ) % evaluation
            << ", id = " << chain.chainID();

        for ( size_t i = 0; i < chain.actions().size(); ++ i )
        {
            const ActionDescription & act = chain.actions()[i];

            buf << "\n[" << act.description() << "]";
        }

        if ( chain.actions().empty() )
        {
            buf << "\nempty chain";
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

        M_list->addItem( QString::fromStdString( buf.str() ) );

        if ( hits != 0 )
        {
            whole_buf << '\n';
        }

        whole_buf << buf.str();
    }

    std::ostringstream header_buf;
    header_buf << "CYCLE " << current_time << '\n'
               << hits << " HITS\n";

    M_text->setText( QString::fromStdString( header_buf.str() ) );
    M_text->append( QString::fromStdString( whole_buf.str() ) );

    M_list->insertItem( 0, QString::fromStdString( header_buf.str() ) );
}
