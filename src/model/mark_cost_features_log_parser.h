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

#ifndef SOCCERWINDOW2_MODEL_MARK_COST_FEATURES_LOG_PARSER_H
#define SOCCERWINDOW2_MODEL_MARK_COST_FEATURES_LOG_PARSER_H

#include "mark_cost_features_log.h"

#include <istream>
#include <vector>
#include <string>
#include <memory>

class MarkCostFeaturesLogParser {
private:
    std::vector< std::string > M_header_fields;
    std::size_t M_label_field_index;
    std::size_t M_group_id_field_index;
    std::size_t M_time_field_index;
    std::size_t M_marker_unum_field_index;
    std::size_t M_target_id_field_index;
    std::size_t M_target_unum_field_index;
    std::size_t M_target_pos_x_field_index;
    std::size_t M_target_pos_y_field_index;
    std::size_t M_move_point_x_field_index;
    std::size_t M_move_point_y_field_index;
public:

    bool parse( std::istream & is,
                MarkCostFeaturesLog & log );

private:

    bool parseHeader( std::istream & is );
    bool parseRecord( std::istream & is,
                      MarkCostFeaturesLog & log );
};

#endif
