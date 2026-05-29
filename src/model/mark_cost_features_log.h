// -*-c++-*-

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

#ifndef SOCCERWINDOW2_MODEL_MARK_COST_FEATURES_LOG_H
#define SOCCERWINDOW2_MODEL_MARK_COST_FEATURES_LOG_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

/*-------------------------------------------------------------------*/

struct Marker {
    int unum_;
    rcsc::Vector2D pos_;

    Marker( const int unum,
            const rcsc::Vector2D & pos )
        : unum_( unum ),
          pos_( pos )
    { }
};

/*-------------------------------------------------------------------*/
inline
bool operator==( const Marker & lhs,
                 const Marker & rhs )
{
    return lhs.unum_ == rhs.unum_;
}

/*-------------------------------------------------------------------*/

struct MarkTargetKey {
    char id_;
    int unum_;
    rcsc::Vector2D pos_;

    MarkTargetKey( const char id,
                   const int unum,
                   const rcsc::Vector2D & pos )
        : id_( id ),
          unum_( unum ),
          pos_( pos )
    { }
};

/*-------------------------------------------------------------------*/
inline bool
operator==( const MarkTargetKey & lhs,
            const MarkTargetKey & rhs )
{
    return lhs.id_ == rhs.id_;
}

/*-------------------------------------------------------------------*/

struct MarkAssignment {
    bool assigned_;
    Marker marker_;
    MarkTargetKey target_;

    MarkAssignment( const bool assigned,
                    const int marker_unum,
                    const rcsc::Vector2D & marker_pos,
                    const char target_id,
                    const int target_unum,
                    const rcsc::Vector2D & target_pos )
        : assigned_( assigned ),
          marker_( marker_unum, marker_pos ),
          target_( target_id, target_unum, target_pos )
    { }
};

/*-------------------------------------------------------------------*/

class MarkCostFeaturesLog {
private:
    std::string M_file_path;
    std::map< rcsc::GameTime, std::vector< MarkAssignment >, rcsc::GameTime::Less > M_data;

public:

    void clearAll()
    {
        M_file_path.clear();
        M_data.clear();
    }

    const std::vector< MarkAssignment > & getAssignmentsAt( const rcsc::GameTime & time ) const;

    void setFilePath( const std::string & file_path )
    {
        M_file_path = file_path;
    }
    const std::string & filePath() const
    {
        return M_file_path;
    }

    void addAssignment( const rcsc::GameTime & time,
                        const bool assigned,
                        const int marker_unum,
                        const rcsc::Vector2D & marker_pos,
                        const char target_id,
                        const int target_unum,
                        const rcsc::Vector2D & target_pos )
    {
        M_data[time].emplace_back( assigned,
                                   marker_unum, marker_pos,
                                   target_id, target_unum, target_pos );
    }

    void updateAssignments( const rcsc::GameTime & time,
                            const std::vector< MarkAssignment > & assignments );

    std::ostream & print( std::ostream & os ) const;
};

#endif
