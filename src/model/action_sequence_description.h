// -*-c++-*-

/*!
  \file action_sequence_description.h
  \brief definition of action sequence data Header File.
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

#ifndef SOCCERWINDOW2_ACTION_SEQUENCE_DESCRIPTION_H
#define SOCCERWINDOW2_ACTION_SEQUENCE_DESCRIPTION_H

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
    Category M_category;

    std::string M_name;
    int M_number;

    int M_duration_time;

    int M_from_unum;
    rcsc::Vector2D M_from_pos;

    int M_to_unum;
    rcsc::Vector2D M_to_pos;

    int M_safe_level;

    std::string M_description;

public:
    ActionDescription()
        : M_category( HOLD ),
          M_name(),
          M_number( -1 ),
          M_duration_time( -1 ),
          M_from_unum( -1 ),
          M_from_pos(),
          M_to_unum( -1 ),
          M_to_pos(),
          M_safe_level( 0 ),
          M_description()
      { }

    void setPass( const char * name,
                  const int number,
                  const int duration_time,
                  const int from_unum,
                  const double from_x,
                  const double from_y,
                  const int to_unum,
                  const double to_x,
                  const double to_y,
                  const int safe_level )
      {
          M_category = PASS;
          M_name = name;
          M_number = number;
          M_duration_time = duration_time;
          M_from_unum = from_unum;
          M_from_pos.assign( from_x, from_y );
          M_to_unum = to_unum;
          M_to_pos.assign( to_x, to_y );
          M_safe_level = safe_level;
      }

    void setDribble( const char * name,
                     const int number,
                     const int duration_time,
                     const int unum,
                     const double from_x,
                     const double from_y,
                     const double to_x,
                     const double to_y,
                     const int safe_level )
      {
          M_category = DRIBBLE;
          M_name = name;
          M_number = number;
          M_duration_time = duration_time;
          M_from_unum = unum;
          M_from_pos.assign( from_x, from_y );
          M_to_unum = unum;
          M_to_pos.assign( to_x, to_y );
          M_safe_level = safe_level;
      }

    void setShoot( const char * name,
                   const int duration_time,
                   const int unum,
                   const double from_x,
                   const double from_y,
                   const double to_x,
                   const double to_y,
                   const int safe_level )
      {
          M_category = SHOOT;
          M_name = name;
          M_duration_time = duration_time;
          M_from_unum = unum;
          M_from_pos.assign( from_x, from_y );
          M_to_unum = unum;
          M_to_pos.assign( to_x, to_y );
          M_safe_level = safe_level;
      }

    void setHold( const char * name,
                  const int duration_time,
                  const int unum,
                  const double x,
                  const double y,
                  const int safe_level )
      {
          M_category = HOLD;
          M_name = name;
          M_duration_time = duration_time;
          M_from_unum = unum;
          M_from_pos.assign( x, y );
          M_to_unum = unum;
          M_to_pos.assign( x, y );
          M_safe_level = safe_level;
      }

    void setMove( const char * name,
                  const int duration_time,
                  const int unum,
                  const double from_x,
                  const double from_y,
                  const double to_x,
                  const double to_y,
                  const int safe_level )
      {
          M_category = MOVE;
          M_name = name;
          M_duration_time = duration_time;
          M_from_unum = unum;
          M_from_pos.assign( from_x, from_y );
          M_to_unum = unum;
          M_to_pos.assign( to_x, to_y );
          M_safe_level = safe_level;
      }

    void setDescription( const std::string & description )
      {
          M_description = description;
      }


    Category category() const
      {
          return M_category;
      }

    const std::string & name() const
      {
          return M_name;
      }

    int number() const
      {
          return M_number;
      }

    int durationTime() const
      {
          return M_duration_time;
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

    std::ostream & print( std::ostream & os,
                          const int count ) const;
};

class ActionSequenceDescription {
private:
    int M_id;
    double M_value;
    std::vector< ActionDescription > M_actions;
    std::vector< std::string > M_evaluation_description;

public:
    explicit
    ActionSequenceDescription( int id )
        : M_id( id ),
          M_value( 0.0 )
      { }

    int id() const
      {
          return M_id;
      }

    double value() const
      {
          return M_value;
      }

    const std::vector< ActionDescription > & actions() const
      {
          return M_actions;
      }

    const std::vector< std::string > & evaluationDescription() const
      {
          return M_evaluation_description;
      }

    void setValue( const double value )
      {
          M_value = value;
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

    std::ostream & print( std::ostream & os ) const;

};


class ActionSequenceDescriptionSet {
public:
    typedef std::multimap< double,
                           boost::shared_ptr< const ActionSequenceDescription >,
                           std::greater< double > > MapType;

private:
    MapType M_sequence_map;

public:
    const MapType & getMap() const
      {
          return M_sequence_map;
      }

    void insert( const boost::shared_ptr< const ActionSequenceDescription > & seq )
      {
          M_sequence_map.insert( std::make_pair( seq->value(), seq ) );
      }
};

#endif
