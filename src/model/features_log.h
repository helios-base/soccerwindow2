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
    int M_group_id;
    //std::variant< int, double, std::string > M_label;
    double M_label;
    std::vector< double > M_float_features;
    std::vector< std::string > M_cat_features;

    std::string M_description;

public:

    FeaturesLog()
        : M_group_id( -1 )
      { }

    FeaturesLog( const size_t float_count,
                 const size_t cat_count )
        : M_group_id( -1 )
      {
          M_float_features.reserve( float_count );
          M_cat_features.reserve( cat_count );
      }

    void setGroupId( const int id )
      {
          M_group_id = id;
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

    int groupId() const
      {
          return M_group_id;
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

class GroupedFeaturesLog {
public:

    using Ptr = std::shared_ptr< GroupedFeaturesLog >;

private:
    int M_group_id;
    std::vector< FeaturesLog::Ptr > M_features_list;

public:

    GroupedFeaturesLog()
        : M_group_id( -1 )
      { }

    void setGroupId( const int id )
      {
          M_group_id = id;
      }

    void addFeaturesLog( FeaturesLog::Ptr ptr )
      {
          M_features_list.push_back( ptr );
      }

    int groupId() const
      {
          return M_group_id;
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

    using Ptr = std::shared_ptr< FeaturesLogHolder >;

private:
    std::string M_task_name;
    size_t M_float_features_size;
    size_t M_cat_features_size;
    std::vector< std::string > M_header;
    std::unordered_map< int, GroupedFeaturesLog::Ptr > M_groups;

public:

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

          if ( ! M_groups[ptr->groupId()] )
          {
              M_groups[ptr->groupId()] = GroupedFeaturesLog::Ptr( new GroupedFeaturesLog() );
          }
          M_groups[ptr->groupId()]->addFeaturesLog( ptr );
      }

    void addGroupedFeaturesLog( GroupedFeaturesLog::Ptr ptr )
      {
          if ( ptr )
          {
              M_groups[ptr->groupId()] = ptr;
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

    const std::unordered_map< int, GroupedFeaturesLog::Ptr > & groups() const
      {
          return M_groups;
      }
};

#endif
