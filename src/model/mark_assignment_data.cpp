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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mark_assignment_data.h"

#include "mark_cost_features_log_parser.h"

#include <fstream>
#include <iostream>

/*-------------------------------------------------------------------*/
bool
MarkAssignmentData::open( const std::string & filepath )
{
    std::ifstream fin( filepath );
    if ( ! fin.is_open() )
    {
        return false;
    }

    MarkCostFeaturesLogParser parser;
    if ( ! parser.parse( fin, M_log ) )
    {
        std::cerr << "(MarkAssignmentData::open) failed to parse " << filepath << std::endl;
        return false;
    }

    M_log.setFilePath( filepath );
    std::cerr << "(MarkAssignmentData::open) opened " << filepath << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentData::clear()
{
    M_log.clearAll();
    M_highlighted_assignments.clear();
}
