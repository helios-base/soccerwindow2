// -*-c++-*-

/*!
  \file features_log.h
  \brief features log classes Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
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

#ifndef SOCCERWINDOW2_MODEL_FEATURES_LOG_H
#define SOCCERWINDOW2_MODEL_FEATURES_LOG_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>

//#include <variant>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>

class FeaturesLog {
public:

    using Ptr = std::shared_ptr< FeaturesLog >;

private:
    rcsc::GameTime M_time;
    //std::variant< int, double, std::string > M_label;
    double M_label;
    std::vector< double > M_float_features;
    std::vector< std::string > M_cat_features;

    std::string M_description;

public:

    FeaturesLog()
        : M_time( -1, 0 )
      { }

    FeaturesLog( const size_t float_count,
                 const size_t cat_count )
        : M_time( -1, 0 )
      {
          M_float_features.reserve( float_count );
          M_cat_features.reserve( cat_count );
      }

    void setTime( const rcsc::GameTime & time )
      {
          M_time = time;
      }

    void setDescription( const std::string & description )
      {
          M_description = description;
      }

    void setLabel( const double v )
      {
          M_label = v;
      }

    void addFeature( const double v )
      {
          M_float_features.push_back( v );
      }

    void addFeature( const std::string & v )
      {
          M_cat_features.push_back( v );
      }

    void setFeatures( const std::vector< double > & float_features,
                      const std::vector< std::string > & cat_features )
      {
          M_float_features = float_features;
          M_cat_features = cat_features;
      }

    void setFeatures( const std::vector< double > & float_features )
      {
          M_float_features = float_features;
      }

    void setFeatures( const std::vector< std::string > & cat_features )
      {
          M_cat_features = cat_features;
      }

    const rcsc::GameTime & time() const
      {
          return M_time;
      }

    const std::string & description() const
      {
          return M_description;
      }

    double label() const
      {
          return M_label;
      }

    const std::vector< double > & floatFeatures() const
      {
          return M_float_features;
      }

    const std::vector< std::string > & catFeatures() const
      {
          return M_cat_features;
      }

};

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*!
  \brief time grouped features log
*/

class GroupedFeaturesLog {
public:

    using Ptr = std::shared_ptr< GroupedFeaturesLog >;

private:
    rcsc::GameTime M_time;
    std::vector< FeaturesLog::Ptr > M_features_list;

public:

    GroupedFeaturesLog()
        : M_time( -1, 0 )
      { }

    void setTime( const rcsc::GameTime & time )
      {
          M_time = time;
      }

    void addFeaturesLog( FeaturesLog::Ptr ptr )
      {
          M_features_list.push_back( ptr );
      }

    const rcsc::GameTime & time() const
      {
          return M_time;
      }

    const std::vector< FeaturesLog::Ptr > & featuresList() const
      {
          return M_features_list;
      }
};

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*!
  \brief assumed to store all feature logs for a task written in a file recoreded by the player.
 */
class WholeFeaturesLog {
public:

    using Ptr = std::shared_ptr< WholeFeaturesLog >;
    using Map = std::map< rcsc::GameTime, GroupedFeaturesLog::Ptr, rcsc::GameTime::Less >;

private:
    int M_unum;
    std::string M_task_name;
    size_t M_float_features_size;
    size_t M_cat_features_size;
    std::vector< std::string > M_header;
    Map M_group_map;

public:

    void setUnum( const int unum )
      {
          M_unum = unum;
      }

    void setTaskName( const std::string & task_name )
      {
          M_task_name = task_name;
      }

    void setFeaturesCount( const size_t float_size,
                           const size_t cat_size )
      {
          M_float_features_size = float_size;
          M_cat_features_size = cat_size;
      }

    void setHeader( const std::vector< std::string > & header )
      {
          M_header = header;
      }

    void addFeaturesLog( FeaturesLog::Ptr ptr )
      {
          if ( ! ptr ) return;

          if ( ! M_group_map[ptr->time()] )
          {
              M_group_map[ptr->time()] = GroupedFeaturesLog::Ptr( new GroupedFeaturesLog() );
          }
          M_group_map[ptr->time()]->addFeaturesLog( ptr );
      }

    void addGroupedFeaturesLog( GroupedFeaturesLog::Ptr ptr )
      {
          if ( ptr )
          {
              M_group_map[ptr->time()] = ptr;
          }
      }

    size_t floatFeaturesSize() const
      {
          return M_float_features_size;
      }

    size_t catFeaturesSize() const
      {
          return M_cat_features_size;
      }

    const Map & groupMap() const
      {
          return M_group_map;
      }
};

#endif
