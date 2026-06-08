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

#ifndef SOCCERWINDOW2_MODEL_MARK_ASSIGNMENT_DATA_H
#define SOCCERWINDOW2_MODEL_MARK_ASSIGNMENT_DATA_H

#include "mark_cost_features_log.h"

#include <set>
#include <string>
#include <utility>

/*!
  \brief Aggregates the mark cost features log and its associated
         UI highlight state (which assignments are selected in the editor).
*/
class MarkAssignmentData {
private:

    MarkCostFeaturesLog M_log;

    //! highlighted assignments (marker_unum, target_id) selected in the editor table
    std::set< std::pair< int, char > > M_highlighted_assignments;

    // not used
    MarkAssignmentData( const MarkAssignmentData & ) = delete;
    MarkAssignmentData & operator=( const MarkAssignmentData & ) = delete;

public:

    MarkAssignmentData() = default;
    ~MarkAssignmentData() = default;

    bool open( const std::string & filepath );
    void clear();

    MarkCostFeaturesLog & log()
      {
          return M_log;
      }
    const MarkCostFeaturesLog & log() const
      {
          return M_log;
      }

    void setHighlightedAssignments( const std::set< std::pair< int, char > > & assignments )
      {
          M_highlighted_assignments = assignments;
      }
    void clearHighlightedAssignments()
      {
          M_highlighted_assignments.clear();
      }
    const std::set< std::pair< int, char > > & highlightedAssignments() const
      {
          return M_highlighted_assignments;
      }
};

#endif
