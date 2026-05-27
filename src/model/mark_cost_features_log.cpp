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
