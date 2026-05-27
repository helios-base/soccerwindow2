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

struct MarkAssignment {
    std::uint8_t label_;
    int marker_unum_;
    int target_unum_;
    rcsc::Vector2D target_pos_;
    rcsc::Vector2D move_point_;

    MarkAssignment( const std::uint8_t label,
                    const int marker_unum,
                    const int target_unum,
                    const rcsc::Vector2D & target_pos,
                    const rcsc::Vector2D & move_point )
        : label_( label ),
          marker_unum_( marker_unum ),
          target_unum_( target_unum ),
          target_pos_( target_pos ),
          move_point_( move_point )
    { }

};


/*-------------------------------------------------------------------*/

class MarkCostFeaturesLog {
public:
    using Ptr = std::shared_ptr< MarkCostFeaturesLog >;

private:
    std::string M_game_id;
    std::map< rcsc::GameTime, std::vector< MarkAssignment >, rcsc::GameTime::Less > M_data;

public:

    const std::vector< MarkAssignment > & getAssignmentsAt( const rcsc::GameTime & time ) const;

    void setGameID( const std::string & game_id )
    {
        M_game_id = game_id;
    }
    const std::string & gameID() const
    {
        return M_game_id;
    }

    void addAssignment( const rcsc::GameTime & time,
                        const MarkAssignment & assignment )
    {
        M_data[time].push_back( assignment );
    }
    void addAssignment( const rcsc::GameTime & time,
                        const std::uint8_t label,
                        const int marker_unum,
                        const int target_unum,
                        const rcsc::Vector2D & target_pos,
                        const rcsc::Vector2D & move_point )
    {
        M_data[time].emplace_back( label, marker_unum, target_unum, target_pos, move_point );
    }
};

#endif
