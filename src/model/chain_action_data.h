// -*-c++-*-

/*!
  \file chain_action_data.h
  \brief data contains chain actions Header File.
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

#ifndef SOCCERWINDOW2_CHAIN_ACTION_DATA_H
#define SOCCERWINDOW2_CHAIN_ACTION_DATA_H

#include <rcsc/geom/vector_2d.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <iosfwd>


class ActionDescription {
public:
    enum Category { HOLD, PASS, DRIBBLE, SHOOT, MOVE };

private:
    Category M_action_category;

    std::string M_action_name;
    int M_action_number;

    long M_spend_time;

    int M_from_unum;
    rcsc::Vector2D M_from_pos;

    int M_to_unum;
    rcsc::Vector2D M_to_pos;

    int M_safe_level;

    std::string M_description;

public:
    ActionDescription()
        : M_action_category( HOLD ),
          M_action_name(),
          M_action_number( -1 ),
          M_spend_time( -1 ),
          M_from_unum( -1 ),
          M_from_pos(),
          M_to_unum( -1 ),
          M_to_pos(),
          M_safe_level( 0 ),
          M_description()
      { }

    void setActionCategory( Category category )
      {
          M_action_category = category;
      }

    void setActionName( const std::string & action_name )
      {
          M_action_name = action_name;
      }

    void setActionNumber( int action_number )
      {
          M_action_number = action_number;
      }

    void setSpendTime( int spend_time )
      {
          M_spend_time = spend_time;
      }

    void setFrom( const int unum,
                  const rcsc::Vector2D & pos )
      {
          M_from_unum = unum;
          M_from_pos = pos;
      }

    void setTo( const int unum,
                const rcsc::Vector2D & pos )
      {
          M_to_unum = unum;
          M_to_pos = pos;
      }

    void setSafeLevel( const int level )
      {
          M_safe_level = level;
      }

    void setDescription( const std::string & description )
      {
          M_description = description;
      }


    Category actionCategory() const
      {
          return M_action_category;
      }

    const std::string & actionName() const
      {
          return M_action_name;
      }

    int actionNumber() const
      {
          return M_action_number;
      }

    long spendTime() const
      {
          return M_spend_time;
      }

    int fromUnum() const
      {
          return M_from_unum;
      }

    const rcsc::Vector2D & fromPos() const
      {
          return M_from_pos;
      }

    int toUnum() const
      {
          return M_to_unum;
      }

    const rcsc::Vector2D & toPos() const
      {
          return M_to_pos;
      }

    int safeLevel() const
      {
          return M_safe_level;
      }


    const std::string & description() const
      {
          return M_description;
      }

    static
    void debugPrint( const ActionDescription & action,
                     const int index,
                     std::ostream & out );
};

class ChainDescription {
public:
    typedef std::vector< ActionDescription > ActionSequenceType;

private:
    int M_chain_id;
    ActionSequenceType M_actions;
    std::vector< std::string > M_evaluation_description;

public:
    explicit
    ChainDescription( int chain_id )
        : M_chain_id( chain_id )
      {
      }

    int chainID() const
      {
          return M_chain_id;
      }

    const ActionSequenceType & actions() const
      {
          return M_actions;
      }

    const std::vector< std::string > & evaluationDescription() const
      {
          return M_evaluation_description;
      }

    void add( const ActionDescription & act )
      {
          M_actions.push_back( act );
      }

    void setEvaluationDescription( const std::vector< std::string > & evaluation_description )
      {
          M_evaluation_description = evaluation_description;
      }


public:
    static
    void debugPrint( const ChainDescription & chain,
                     const double evaluation,
                     std::ostream & out );
};


class ChainDescriptionSet {
public:
    typedef std::multimap< double,
                           boost::shared_ptr< const ChainDescription >,
                           std::greater< double > > MapType;

private:
    MapType M_chains;

public:
    const MapType & getMap() const
      {
          return M_chains;
      }

    void insert( const boost::shared_ptr< const ChainDescription > & chain,
                 const double evaluation )
      {
          M_chains.insert( std::make_pair( evaluation, chain ) );
      }
};

#endif
