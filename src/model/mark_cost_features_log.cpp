// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mark_cost_features_log.h"

/*-------------------------------------------------------------------*/
void
MarkCostFeaturesLog::addAssignment( const rcsc::GameTime & time,
                                    const std::string & group_id,
                                    const bool assigned,
                                    const int marker_unum,
                                    const rcsc::Vector2D & marker_pos,
                                    const char target_id,
                                    const int target_unum,
                                    const rcsc::Vector2D & target_pos )
{
#if 0
    if ( M_groups.find( time ) == M_groups.end() )
    {
        M_groups.emplace( time, MarkAssignmentGroup( group_id ) );
    }

    decltype( M_groups )::iterator it = M_groups.find( time );
    if ( it == M_groups.end() )
    {
        std::cerr << "Error: Failed to create assignment group at time " << time << std::endl;
        return;
    }
    
    MarkAssignmentGroup & group = it->second;

    if ( group.group_id_ != group_id )
    {
        std::cerr << "Warning: Group ID mismatch at time " << time
                  << ": existing group ID = " << group.group_id_
                  << ", new group ID = " << group_id << std::endl;
    }
#else
    auto [it, inserted] = M_groups.try_emplace( time, group_id );
    MarkAssignmentGroup & group = it->second;
    if ( ! inserted
         && group.group_id_ != group_id )
    {
        std::cerr << "Warning: Group ID mismatch at time " << time
                  << ": existing group ID = " << group.group_id_
                  << ", new group ID = " << group_id << std::endl;
    }
#endif
    group.addAssignment( assigned, marker_unum, marker_pos, target_id, target_unum, target_pos );
}

/*-------------------------------------------------------------------*/
const MarkAssignmentGroup &
MarkCostFeaturesLog::getAssignmentGroupAt( const rcsc::GameTime & time ) const
{
    decltype( M_groups )::const_iterator it = M_groups.find( time );
    if ( it != M_groups.end() )
    {
        return it->second;
    }

    static const MarkAssignmentGroup empty_group( "" );
    return empty_group;
}

/*-------------------------------------------------------------------*/
void
MarkCostFeaturesLog::updateAssignmentGroup( const rcsc::GameTime & time,
                                            const MarkAssignmentGroup & new_group )
{
    auto old_group = M_groups.find( time );
    if ( old_group == M_groups.end() )
    {
        std::cerr << "Warning: No existing assignments at time " << time << std::endl;
        return;
    }

    std::vector< MarkAssignment > & old_assignments = old_group->second.assignments_;

    // reset all assignments to unassigned
    for ( MarkAssignment & a : old_assignments )
    {
        a.assigned_ = false;
    }

    // update assignment flags based on new_assignments
    for ( const MarkAssignment & new_a : new_group.assignments_ )
    {
        // if ( ! new_a.assigned_ )
        // {
        //     std::cerr << "(MarkCostFeaturesLog) Warning:"
        //               << " Skipping unassigned marker " << new_a.marker_.unum_ << std::endl;
        //     continue;
        // }
        auto old_a = std::find_if( old_assignments.begin(), old_assignments.end(),
                                   [&new_a]( const MarkAssignment & a )
                                   {
                                       return ( a.marker_.unum_ == new_a.marker_.unum_ 
                                                && a.target_.id_ == new_a.target_.id_
                                                && a.target_.unum_ == new_a.target_.unum_ );
                                    } );
        if ( old_a != old_assignments.end() )
        {
            old_a->assigned_ = new_a.assigned_;
        }
        else
        {
            std::cerr << "(MarkCostFeaturesLog) Warning:"
                      << " No existing assignment found for marker " << new_a.marker_.unum_
                      << " and target " << new_a.target_.id_ << ' ' << new_a.target_.unum_ << std::endl;
        }
    }
}

/*-------------------------------------------------------------------*/
std::ostream &
MarkCostFeaturesLog::print( std::ostream & os ) const
{
    for ( const auto & entry : M_groups )
    {
        const rcsc::GameTime & time = entry.first;
        const MarkAssignmentGroup & group = entry.second;

        os << time << ":\n";
        os << "group_id: " << group.group_id_ << "\n";
        for ( const MarkAssignment & a : group.assignments_ )
        {
            os << "  ";
            os << "assignment: " << std::boolalpha << a.assigned_
               << ", " << "marker_unum: " << a.marker_.unum_
               << ", " << "target: " << a.target_.id_ << ' ' << a.target_.unum_ << " at " << a.target_.pos_;
            os << "\n";
        }
    }
    return os;
}
