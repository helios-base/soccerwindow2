// -*-c++-*-

/*!
  \file chain_action_log_parser.cpp
  \brief parser class to parse chain action log format Source File.
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

#include "chain_action_log_parser.h"

#include <rcsc/geom/vector_2d.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>

/*-------------------------------------------------------------------*/
/*!

*/
boost::shared_ptr< ChainDescriptionSet >
ChainActionLogParser::parse( std::istream & in )
{
    std::string line;

    boost::shared_ptr< ChainDescriptionSet > data( new ChainDescriptionSet() );

    boost::shared_ptr< ChainDescription > chain;
    std::vector< std::string > evaluation_details;
    std::vector< std::string > last_evaluation_details;

    double last_evaluation = 0.0;

    while ( std::getline( in, line ) )
    {
        //
        // read chain header [id, evaluation]
        //
        {
            int chain_id = -1;
            double evaluation = 0.0;
            int n_read = 0;
            if ( std::sscanf( line.c_str(),
                              "%d: evaluation=%lf%n",
                              &chain_id, &evaluation, &n_read ) == 2
                 && n_read == static_cast<int>( line.length() ) )
            {
                if ( chain )
                {
                    chain->setEvaluationDescription( last_evaluation_details );
                    data->insert( chain, last_evaluation );
                }
                chain = boost::shared_ptr< ChainDescription >( new ChainDescription( chain_id ) );

                last_evaluation = evaluation;
                last_evaluation_details = evaluation_details;
                evaluation_details.clear();

                continue;
            }
        }

        //
        // read evaluation detail
        //
        {
            static const std::string eval_prefix = "(eval) ";

            if ( line.compare( 0, eval_prefix.size(), eval_prefix ) == 0 )
            {
                evaluation_details.push_back( line.substr( eval_prefix.size() ) );
                continue;
            }
        }


        char action_name[ 256 ];
        action_name[ sizeof( action_name ) - 1 ] = '\0';
        int action_number = 0;

        int duration_time = 0;
        int depth = 0;

        int from = -1;
        int to = -1;
        double from_x = 0.0;
        double from_y = 0.0;

        double to_x = 0.0;
        double to_y = 0.0;

        int safe_level = 0.0;

        int n_read = 0;

        ActionDescription act;
        act.setDescription( line );

        //
        // read each chain entry
        //
        if ( chain )
        {
            n_read = 0;
            if ( std::sscanf( line.c_str(),
                              "__ %d: pass (%255[0-9a-zA-Z_-][%d]) t=%d"
                              " from[%d](%lf %lf)-to[%d](%lf %lf), safe=%d%n",
                              &depth, action_name, &action_number, &duration_time,
                              &from, &from_x, &from_y,
                              &to, &to_x, &to_y,
                              &safe_level,
                              &n_read ) == 11
                 && n_read == static_cast< int >( line.length() ) )
            {
                act.setPass( action_name, action_number, duration_time,
                             from, from_x, from_y,
                             to, to_x, to_y,
                             safe_level );
                chain->add( act );
                continue;
            }

            n_read = 0;
            if ( std::sscanf( line.c_str(),
                              "__ %d: dribble (%255[0-9a-zA-Z_-][%d]) t=%d"
                              " from[%d](%lf %lf)-to(%lf %lf), safe=%d%n",
                              &depth, action_name, &action_number, &duration_time,
                              &from, &from_x, &from_y,
                              &to_x, &to_y, &safe_level,
                              &n_read ) == 10
                 && n_read == static_cast< int >( line.length() ) )
            {
                act.setDribble( action_name, action_number, duration_time,
                                from, from_x, from_y,
                                to_x, to_y,
                                safe_level );
                chain->add( act );
                continue;
            }

            n_read = 0;
            if ( std::sscanf( line.c_str(),
                              "__ %d: shoot (%255[0-9a-zA-Z_-]) t=%d"
                              " from[%d](%lf %lf)-to(%lf %lf), safe=%d%n",
                              &depth, action_name, &duration_time,
                              &from, &from_x, &from_y,
                              &to_x, &to_y, &safe_level,
                              &n_read ) == 9
                 && n_read == static_cast< int >( line.length() ) )
            {
                act.setShoot( action_name, duration_time,
                              from, from_x, from_y,
                              to_x, to_y,
                              safe_level );
                chain->add( act );
                continue;
            }

            n_read = 0;
            if ( std::sscanf( line.c_str(),
                              "__ %d: hold (%255[0-9a-zA-Z_-]) t=%d"
                              " from[%d](%lf %lf), safe=%d%n",
                              &depth, action_name, &duration_time,
                              &from, &from_x, &from_y,
                              &safe_level,
                              &n_read ) == 7
                 && n_read == static_cast< int >( line.length() ) )
            {
                act.setHold( action_name, duration_time,
                             from, from_x, from_y,
                             safe_level );
                chain->add( act );
                continue;
            }

            n_read = 0;
            if ( std::sscanf( line.c_str(),
                              "__ %d: move (%255[0-9a-zA-Z_-]) t=%d"
                              " from[%d](%lf %lf)-to(%lf %lf), safe=%d%n",
                              &depth, action_name, &duration_time,
                              &from, &from_x, &from_y,
                              &to_x, &to_y, &safe_level,
                              &n_read ) == 9
                 && n_read == static_cast< int >( line.length() ) )
            {
                act.setMove( action_name, duration_time,
                             from, from_x, from_y,
                             to_x, to_y,
                             safe_level );
                chain->add( act );
                continue;
            }
        }

        std::cerr << __FILE__ << ": !! skip[" << line << "]" << std::endl;
    }

    return data;
}
