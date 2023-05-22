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

//#include <variant>
#include <string>
#include <unordered_map>
#include <memory>

class FeaturesLog {
public:

    using Ptr = std::shared_ptr< FeaturesLog >;

private:
    //std::variant< int, double, std::string > M_label;
    //std::string M_label;

    int M_index;
    rcsc::Vector2D M_pos; //!< coordinates value for visualizing purpose
    std::string M_description;

    double M_value;
    std::vector< double > M_float_features;
    std::vector< std::string > M_cat_features;

public:

    FeaturesLog()
        : M_index( -1 ),
          M_pos( rcsc::Vector2D::INVALIDATED )
      { }

    FeaturesLog( const size_t float_count,
                 const size_t cat_count )
        : M_index( -1 ),
          M_pos( rcsc::Vector2D::INVALIDATED )
      {
          M_float_features.reserve( float_count );
          M_cat_features.reserve( cat_count );
      }

    void setIndex( const int idx )
      {
          M_index = idx;
      }

    void setPos( const double x,
                 const double y )
      {
          M_pos.assign( x, y );
      }

    void setDescription( const std::string & description )
      {
          M_description = description;
      }

    void setValue( const double v )
      {
          M_value = v;
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


    int index() const
      {
          return M_index;
      }

    const rcsc::Vector2D & pos() const
      {
          return M_pos;
      }

    const std::string & description() const
      {
          return M_description;
      }

    double value() const
      {
          return M_value;
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

class GroupedFeaturesLog {
public:

    //using Ptr = std::shared_ptr< GroupedFeaturesLog >;

private:
    std::string M_task_name;
    int M_group_id;
    std::vector< std::string > M_header;
    std::vector< FeaturesLog::Ptr > M_features_list;

public:

    GroupedFeaturesLog()
        : M_group_id( -1 )
      { }

    explicit
    GroupedFeaturesLog( const std::string & task_name )
        : M_task_name( task_name ),
          M_group_id( -1 )
      { }

    void setTaskName( const std::string & task_name )
      {
          M_task_name = task_name;
      }

    void setGroupId( const int id )
      {
          M_group_id = id;
      }

    void setHeader( const std::vector< std::string > & header )
      {
          M_header = header;
      }

    void addFeaturesLog( FeaturesLog::Ptr ptr )
      {
          M_features_list.push_back( ptr );
      }

    const std::string & taskName() const
      {
          return M_task_name;
      }

    int groupId() const
      {
          return M_group_id;
      }

    const std::vector< std::string > & header() const
      {
          return M_header;
      }

    const std::vector< FeaturesLog::Ptr > & featuresList() const
      {
          return M_features_list;
      }
};

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

class FeaturesLogHolder {
public:
    // key: task name, value: grouped features log
    using Map = std::unordered_map< std::string, GroupedFeaturesLog >;

private:
    Map M_groups;

public:

    bool addFeaturesLog( const std::string & task_name,
                         FeaturesLog::Ptr ptr )
      {
          if ( task_name.empty() )
          {
              return false;
          }

          // if ( M_groups.find( task_name ) == M_groups.end() )
          // {
          //     M_groups.insert( std::make_pair( task_name, GroupedFeaturesLog( task_name ) ) );

          // }
          // M_groups[task_name].addFeaturesLog( ptr );

          GroupedFeaturesLog & group = M_groups[task_name];
          group.setTaskName( task_name );
          group.addFeaturesLog( ptr );
          return true;
      }

    bool setHeader( const std::string & task_name,
                    const std::vector< std::string > & header )
      {
          if ( task_name.empty() )
          {
              return false;
          }

          M_groups[task_name].setHeader( header );
          return true;
      }

    const Map & groups() const
      {
          return M_groups;
      }
};

#endif
