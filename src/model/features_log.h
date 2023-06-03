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

#include "draw_types.h"

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>

//#include <variant>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>

class Features {
public:

    using Ptr = std::shared_ptr< Features >;
    using ConstPtr = std::shared_ptr< const Features >;

private:
    rcsc::GameTime M_time;
    int M_index;
    int M_editable_label; //!< rank value or category label
    double M_value; //!< score or label
    std::vector< double > M_float_features;
    std::vector< std::string > M_cat_features;

    std::shared_ptr< const DrawGroup > M_draw_data;

public:

    Features()
        : M_time( -1, 0 ),
          M_index( -1 ),
          M_editable_label( 0 ),
          M_value( 0.0 )
      { }

    Features( const size_t float_count,
                 const size_t cat_count )
        : M_time( -1, 0 ),
          M_index( -1 ),
          M_editable_label( 0 ),
          M_value( 0.0 )
      {
          M_float_features.reserve( float_count );
          M_cat_features.reserve( cat_count );
      }

    void setTime( const rcsc::GameTime & time )
      {
          M_time = time;
      }

    void setIndex( const int idx )
      {
          M_index = idx;
      }

    void setDrawData( std::shared_ptr< DrawGroup > draw_data )
      {
          M_draw_data = draw_data;
      }

    void setEditableLabel( const int v )
      {
          M_editable_label = v;
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

    const rcsc::GameTime & time() const
      {
          return M_time;
      }


    int index() const
      {
          return M_index;
      }

    int editableLabel() const
      {
          return M_editable_label;
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


    const std::shared_ptr< const DrawGroup > & drawData() const
      {
          return M_draw_data;
      };


    std::ostream & printCSV( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*!
  \brief time grouped features log (features list for the same situation)
*/

class FeaturesGroup {
public:

    using Ptr = std::shared_ptr< FeaturesGroup >;
    using ConstPtr = std::shared_ptr< const FeaturesGroup >;

private:
    rcsc::GameTime M_time;
    std::vector< Features::Ptr > M_features_list;

public:

    FeaturesGroup()
        : M_time( -1, 0 )
      { }

    void setTime( const rcsc::GameTime & time )
      {
          M_time = time;
      }

    void addFeatures( Features::Ptr ptr )
      {
          M_features_list.push_back( ptr );
      }

    const rcsc::GameTime & time() const
      {
          return M_time;
      }

    const std::vector< Features::Ptr > & featuresList() const
      {
          return M_features_list;
      }

    Features::ConstPtr findFeaturesLog( const int index ) const;

    std::ostream & printCSV( std::ostream & os ) const;
};

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*!
  \brief assumed to store all feature logs for a task written in a file recoreded by the player.
 */
class FeaturesLog {
public:

    using Ptr = std::shared_ptr< FeaturesLog >;
    using ConstPtr = std::shared_ptr< const FeaturesLog >;
    using Map = std::map< rcsc::GameTime, FeaturesGroup::Ptr, rcsc::GameTime::Less >;

private:
    int M_unum;
    std::string M_task_name;
    size_t M_float_features_size;
    size_t M_cat_features_size;
    std::vector< std::string > M_feature_names;
    Map M_timed_map;

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

    void setFeatureNames( const std::vector< std::string > & names )
      {
          M_feature_names = names;
      }

    void addFeatures( const rcsc::GameTime & time,
                      Features::Ptr ptr )
      {
          if ( ! ptr ) return;

          if ( ! M_timed_map[time] )
          {
              M_timed_map[time] = FeaturesGroup::Ptr( new FeaturesGroup() );
          }
          M_timed_map[time]->addFeatures( ptr );
      }

    void addFeaturesGroup( const rcsc::GameTime & time,
                           FeaturesGroup::Ptr ptr )
      {
          if ( ptr )
          {
              M_timed_map[time] = ptr;
          }
      }

    int unum() const
      {
          return M_unum;
      }

    const std::string & taskName() const
      {
          return M_task_name;
      }

    const std::vector< std::string > & featureNames() const
      {
          return M_feature_names;
      }

    size_t floatFeaturesSize() const
      {
          return M_float_features_size;
      }

    size_t catFeaturesSize() const
      {
          return M_cat_features_size;
      }

    const Map & timedMap() const
      {
          return M_timed_map;
      }

    FeaturesGroup::ConstPtr findGroup( const rcsc::GameTime & time ) const;

    std::ostream & printCSV( std::ostream & os ) const;
};

#endif
