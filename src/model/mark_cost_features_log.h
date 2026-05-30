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

    struct Less {
        bool operator()( const Marker & lhs,
                         const Marker & rhs ) const
          {
              return ( lhs.unum_ < rhs.unum_ );
          }
    };
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

    struct Less {
        bool operator()( const MarkTargetKey & lhs,
                         const MarkTargetKey & rhs ) const
          {
              return ( lhs.id_ < rhs.id_ );
          }
    };
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
    std::vector< std::string > raw_fields_; //!< all original CSV fields; empty if not loaded from file

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

struct MarkAssignmentGroup {
    std::string group_id_;
    std::vector< MarkAssignment > assignments_;

    explicit
    MarkAssignmentGroup( const std::string & group_id )
        : group_id_( group_id )
    { }

    void addAssignment( const bool assigned,
                        const int marker_unum,
                        const rcsc::Vector2D & marker_pos,
                        const char target_id,
                        const int target_unum,
                        const rcsc::Vector2D & target_pos,
                        std::vector< std::string > raw_fields = {} )
    {
        assignments_.emplace_back( assigned, marker_unum, marker_pos, target_id, target_unum, target_pos );
        assignments_.back().raw_fields_ = std::move( raw_fields );
    }
};

/*-------------------------------------------------------------------*/

class MarkCostFeaturesLog {
private:
    std::string M_file_path;
    std::string M_header_line;          //!< original CSV header line
    std::size_t M_label_field_index = std::string::npos; //!< index of the 'label' column
    std::map< rcsc::GameTime, MarkAssignmentGroup, rcsc::GameTime::Less > M_groups;

public:

    void clearAll()
    {
        M_file_path.clear();
        M_header_line.clear();
        M_label_field_index = std::string::npos;
        M_groups.clear();
    }

    const std::map< rcsc::GameTime, MarkAssignmentGroup, rcsc::GameTime::Less > & groups() const
    {
        return M_groups;
    }

    const MarkAssignmentGroup & getAssignmentGroupAt( const rcsc::GameTime & time ) const;

    void setFilePath( const std::string & file_path )
    {
        M_file_path = file_path;
    }
    const std::string & filePath() const
    {
        return M_file_path;
    }

    const std::string & headerLine() const { return M_header_line; }
    std::size_t labelFieldIndex() const { return M_label_field_index; }

    void setHeaderLine( const std::string & line ) { M_header_line = line; }
    void setLabelFieldIndex( std::size_t idx ) { M_label_field_index = idx; }

    void addAssignment( const rcsc::GameTime & time,
                        const std::string & group_id,
                        const bool assigned,
                        const int marker_unum,
                        const rcsc::Vector2D & marker_pos,
                        const char target_id,
                        const int target_unum,
                        const rcsc::Vector2D & target_pos,
                        std::vector< std::string > raw_fields = {} );

    void updateAssignmentGroup( const rcsc::GameTime & time,
                                const MarkAssignmentGroup & new_group );

    std::ostream & print( std::ostream & os ) const;
};

#endif
