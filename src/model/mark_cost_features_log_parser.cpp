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

#include "mark_cost_features_log_parser.h"

#include <sstream>

using namespace rcsc;

namespace {
std::size_t
find_field_index( const std::vector< std::string > & fields,
                  const std::string & field_name )
{
    auto it = std::find( fields.begin(), fields.end(), field_name );
    if ( it == fields.end() )
    {
        return std::string::npos;
    }
    return std::distance( fields.begin(), it );
}
}

/*-------------------------------------------------------------------*/
bool
MarkCostFeaturesLogParser::parse( std::istream & is,
                                  MarkCostFeaturesLog & log )
{
    if ( ! parseHeader( is ) )
    {
        return false;
    }

    log.clearAll();
    log.setHeaderLine( M_header_line );
    log.setAcceptedFieldIndex( M_acccepted_field_index );
    log.setLabelFieldIndex( M_label_field_index );
    log.setSameInLastStepFieldIndex( M_same_in_last_step_field_index );

    while ( parseRecord( is, log ) )
    {
        // continue parsing until EOF or error
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
MarkCostFeaturesLogParser::parseHeader( std::istream & is )
{
    // parse csv header
    std::string line;
    if ( ! std::getline( is, line ) )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "failed to read the header line." << std::endl;
        return false;
    }

    M_header_line = line;

    // split the header line into fields
    std::string field;
    std::istringstream istr( line );
    while ( std::getline( istr, field, ',' ) )
    {
        M_header_fields.push_back( field );
    }

    // find the field indices

    M_acccepted_field_index = find_field_index( M_header_fields, "Accepted" );
    if ( M_acccepted_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'Accepted' is not found in the header." << std::endl;
        return false;
    }

    M_label_field_index = find_field_index( M_header_fields, "label" );
    if ( M_label_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'label' is not found in the header." << std::endl;
        return false;
    }

    M_group_id_field_index = find_field_index( M_header_fields, "group_id" );
    if ( M_group_id_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'group_id' is not found in the header." << std::endl;
        return false;
    }

    M_time_field_index = find_field_index( M_header_fields, "Time" );
    if ( M_time_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'Time' is not found in the header." << std::endl;
        return false;
    }

    M_ball_pos_x_field_index = find_field_index( M_header_fields, "BallPosX" );
    M_ball_pos_y_field_index = find_field_index( M_header_fields, "BallPosY" );
    if ( M_ball_pos_x_field_index == std::string::npos 
         || M_ball_pos_y_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'BallPosX' or 'BallPosY' is not found in the header." << std::endl;
    }

    M_marker_unum_field_index = find_field_index( M_header_fields, "MarkerUnum" );
    if ( M_marker_unum_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'MarkerUnum' is not found in the header." << std::endl;
        return false;
    }

    M_marker_pos_x_field_index = find_field_index( M_header_fields, "MarkerPosX" );
    if ( M_marker_pos_x_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'MarkerPosX' is not found in the header." << std::endl;
        return false;
    }

    M_marker_pos_y_field_index = find_field_index( M_header_fields, "MarkerPosY" );
    if ( M_marker_pos_y_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'MarkerPosY' is not found in the header." << std::endl;
        return false;
    }

    M_target_id_field_index = find_field_index( M_header_fields, "TargetId" );
    if ( M_target_id_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'TargetId' is not found in the header." << std::endl;
        return false;
    }

    M_target_unum_field_index = find_field_index( M_header_fields, "TargetUnum" );
    if ( M_target_unum_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'TargetUnum' is not found in the header." << std::endl;
        return false;
    }

    M_target_pos_x_field_index = find_field_index( M_header_fields, "TargetPosX" );
    if ( M_target_pos_x_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'TargetPosX' is not found in the header." << std::endl;
        return false;
    }

    M_target_pos_y_field_index = find_field_index( M_header_fields, "TargetPosY" );
    if ( M_target_pos_y_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'TargetPosY' is not found in the header." << std::endl;
        return false;
    }

    M_move_point_x_field_index = find_field_index( M_header_fields, "MovePointX" );
    if ( M_move_point_x_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'MovePointX' is not found in the header." << std::endl;
        return false;
    }

    M_move_point_y_field_index = find_field_index( M_header_fields, "MovePointY" );
    if ( M_move_point_y_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'MovePointY' is not found in the header." << std::endl;
        return false;
    }

    M_same_in_last_step_field_index = find_field_index( M_header_fields, "SameInLastStep" );
    // Note: the 'SameInLastStep' field is optional, so we do not return false if it's not found.
    //  We just set the index to npos and handle it accordingly in the rest of the code.
    if ( M_same_in_last_step_field_index == std::string::npos )
    {
        std::cerr << __FILE__ << ": (parseHeader) "
                  << "the field 'SameInLastStep' is not found in the header."
                  << " This field is optional, so we will proceed without it." << std::endl;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
MarkCostFeaturesLogParser::parseRecord( std::istream & is,
                                        MarkCostFeaturesLog & log )
{
    // parse csv record
    std::string line;
    if ( ! std::getline( is, line ) )
    {
        return false;
    }

    // split the record line into fields
    std::string field;
    std::istringstream istr( line );
    std::vector< std::string > fields;
    while ( std::getline( istr, field, ',' ) )
    {
        fields.push_back( field );
    }
    if ( fields.size() != M_header_fields.size() )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "the number of fields in the record does not match the header." << std::endl;
        return false;
    }

    // parse the fields
    int accepted_flag;
    int label;
    std::string group_id;
    rcsc::GameTime time;
    rcsc::Vector2D ball_pos = rcsc::Vector2D::INVALIDATED;
    int marker_unum;
    double marker_pos_x, marker_pos_y;
    char target_id;
    int target_unum;
    double target_pos_x, target_pos_y;
    double move_point_x, move_point_y;

    if ( std::sscanf( fields[M_acccepted_field_index].c_str(), "%d", &accepted_flag ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the Accepted field: " << fields[M_acccepted_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_label_field_index].c_str(), "%d", &label ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the Label field: " << fields[M_label_field_index] << std::endl;
        return false;
    }

    group_id = fields[M_group_id_field_index];
    // trim quotes if present
    if ( ! group_id.empty()
         && group_id.front() == '"' && group_id.back() == '"' )
    {
        group_id = group_id.substr( 1, group_id.size() - 2 );
    }

    {
        int cycle, stopped;
        if ( std::sscanf( fields[M_time_field_index].c_str(), "\"%d-%d\"", &cycle, &stopped ) == 2 )
        {
            time.assign( cycle, stopped );
        }
        else
        {
            std::cerr << __FILE__ << ": (parseRecord) "
                      << "failed to parse the Time field: " << fields[M_time_field_index] << std::endl;
            return false;
        }
    }

    if ( M_ball_pos_x_field_index != std::string::npos 
         && M_ball_pos_y_field_index != std::string::npos )
    {
        if ( std::sscanf( fields[M_ball_pos_x_field_index].c_str(), "%lf", &ball_pos.x ) != 1 )
        {
            std::cerr << __FILE__ << ": (parseRecord) "
                      << "failed to parse the BallPosX field: " << fields[M_ball_pos_x_field_index] << std::endl;
            return false;
        }
        if ( std::sscanf( fields[M_ball_pos_y_field_index].c_str(), "%lf", &ball_pos.y ) != 1 )
        {
            std::cerr << __FILE__ << ": (parseRecord) "
                      << "failed to parse the BallPosY field: " << fields[M_ball_pos_y_field_index] << std::endl;
            return false;
        }
    }

    if ( std::sscanf( fields[M_marker_unum_field_index].c_str(), "%d", &marker_unum ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the MarkerUnum field: " << fields[M_marker_unum_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_marker_pos_x_field_index].c_str(), "%lf", &marker_pos_x ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the MarkerPosX field: " << fields[M_marker_pos_x_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_marker_pos_y_field_index].c_str(), "%lf", &marker_pos_y ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the MarkerPosY field: " << fields[M_marker_pos_y_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_target_id_field_index].c_str(), "\"%c\"", &target_id ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the TargetId field: " << fields[M_target_id_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_target_unum_field_index].c_str(), "%d", &target_unum ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the TargetUnum field: " << fields[M_target_unum_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_target_pos_x_field_index].c_str(), "%lf", &target_pos_x ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the TargetPosX field: " << fields[M_target_pos_x_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_target_pos_y_field_index].c_str(), "%lf", &target_pos_y ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the TargetPosY field: " << fields[M_target_pos_y_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_move_point_x_field_index].c_str(), "%lf", &move_point_x ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the MovePointX field: " << fields[M_move_point_x_field_index] << std::endl;
        return false;
    }

    if ( std::sscanf( fields[M_move_point_y_field_index].c_str(), "%lf", &move_point_y ) != 1 )
    {
        std::cerr << __FILE__ << ": (parseRecord) "
                  << "failed to parse the MovePointY field: " << fields[M_move_point_y_field_index] << std::endl;
        return false;
    }

    const bool accepted = ( accepted_flag != 0 );
    const bool assigned = ( label != 0 );
    log.addAssignment( time, group_id, accepted, assigned,
                       ball_pos,
                       marker_unum, Vector2D( marker_pos_x, marker_pos_y ),
                       target_id, target_unum, Vector2D( target_pos_x, target_pos_y ),
                       Vector2D( move_point_x, move_point_y ),
                       std::move( fields ) );
    return true;
}
