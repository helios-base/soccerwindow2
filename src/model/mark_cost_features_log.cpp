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
const std::vector< MarkAssignment > &
MarkCostFeaturesLog::getAssignmentsAt( const rcsc::GameTime & time ) const
{
    decltype( M_data )::const_iterator it = M_data.find( time );
    if ( it != M_data.end() )
    {
        return it->second;
    }

    static const std::vector< MarkAssignment > empty_assignments;
    return empty_assignments;
}

/*-------------------------------------------------------------------*/
void
MarkCostFeaturesLog::updateAssignments( const rcsc::GameTime & time,
                                        const std::vector< MarkAssignment > & new_assignments )
{
    auto assignments_it = M_data.find( time );
    if ( assignments_it == M_data.end() )
    {
        std::cerr << "Warning: No existing assignments at time " << time << std::endl;
        return;
    }

    std::vector< MarkAssignment > & current_assignments = assignments_it->second;

    // reset all assignments to unassigned
    for ( MarkAssignment & a : current_assignments )
    {
        a.assigned_ = false;
    }

    // update assignment flags based on new_assignments
    for ( const MarkAssignment & new_a : new_assignments )
    {
        if ( ! new_a.assigned_ )
        {
            std::cerr << "(MarkCostFeaturesLog) Warning:"
                      << " Skipping unassigned marker " << new_a.marker_.unum_ << std::endl;
            continue;
        }
        auto old_a = std::find_if( current_assignments.begin(), current_assignments.end(),
                                   [&new_a]( const MarkAssignment & a )
                                   {
                                       return ( a.marker_.unum_ == new_a.marker_.unum_ 
                                                && a.target_.id_ == new_a.target_.id_
                                                && a.target_.unum_ == new_a.target_.unum_ );
                                    } );
        if ( old_a != current_assignments.end() )
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
    for ( const auto & entry : M_data )
    { 
        const rcsc::GameTime & time = entry.first;
        const std::vector< MarkAssignment > & assignments = entry.second;

        os << time << ":\n";
        for ( const MarkAssignment & a : assignments )
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
