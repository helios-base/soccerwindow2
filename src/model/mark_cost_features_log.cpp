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
const std::vector< MarkAssignment::Ptr > &
MarkCostFeaturesLog::getAssignmentsAt( const rcsc::GameTime & time ) const
{
    decltype( M_data )::const_iterator it = M_data.find( time );
    if ( it != M_data.end() )
    {
        return it->second;
    }

    static const std::vector< MarkAssignment::Ptr > empty_assignments;
    return empty_assignments;
}

/*-------------------------------------------------------------------*/
void
MarkCostFeaturesLog::updateAssignments( const rcsc::GameTime & time,
                                        const std::vector< std::pair< int, MarkTargetKey > > & assignments )
{
    std::vector< MarkAssignment::Ptr > & current_assignments = M_data[time];

    for ( const auto & [marker_unum, target_key] : assignments )
    {
        auto it = std::find_if( current_assignments.begin(), current_assignments.end(),
                                [marker_unum]( const MarkAssignment::Ptr & a )
                                {
                                    return a->marker_unum_ == marker_unum;
                                } );
        if ( it != current_assignments.end() )
        {
            (*it)->target_ = target_key;
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
        const std::vector< MarkAssignment::Ptr > & assignments = entry.second;

        os << time << ":\n";
        for ( const MarkAssignment::Ptr & a : assignments )
        {
            os << "  ";
            os << "assignment: " << std::boolalpha << a->assigned_
               << ", " << "marker_unum: " << a->marker_unum_
               << ", " << "target: " << a->target_.id_ << ' ' << a->target_.unum_ << " at " << a->target_.pos_;
            os << "\n";
        }
    }
    return os;
}
