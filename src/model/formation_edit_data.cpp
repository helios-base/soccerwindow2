// -*-c++-*-

/*!
  \file formation_edit_data.cpp
  \brief formation editor data class Source File.
*/

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

#include "formation_edit_data.h"

#include "options.h"

#include <rcsc/formation/formation_parser.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/math_util.h>

//#include <chronoe>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ctime>

using namespace rcsc;

const double FormationEditData::MAX_X = ServerParam::DEFAULT_PITCH_LENGTH * 0.5 + 2.0;
const double FormationEditData::MAX_Y = ServerParam::DEFAULT_PITCH_WIDTH * 0.5 + 2.0;

namespace {

inline
double
round_x( const double x )
{
    return rint( bound( -FormationEditData::MAX_X, x, FormationEditData::MAX_X ) / FormationData::PRECISION ) * FormationData::PRECISION;
}

inline
double
round_y( const double y )
{
    return rint( bound( -FormationEditData::MAX_Y, y, FormationEditData::MAX_Y ) / FormationData::PRECISION ) * FormationData::PRECISION;
}

inline
Vector2D
round_coordinates( const double x,
                   const double y )
{
    return Vector2D( round_x( x ), round_y( y ) );
}

std::string
make_current_datetime_string()
{
    // std::chrono::system_clock::time_point n = std::chrono::system_clock::now();
    std::time_t t = std::time( nullptr );
    std::tm * lt = std::localtime( &t );

    char time_str[128];
    std::strftime( time_str, sizeof( time_str ), "%Y%m%d-%H%M%S", lt );

    return time_str;
}

}


/*-------------------------------------------------------------------*/
FormationEditData::FormationEditData()
    : M_filepath(),
      M_modified( false ),
      M_saved_datetime(), // == duration::zero()
      M_current_index( -1 ),
      M_select_type( NO_SELECT ),
      M_select_index( 0 )
{
    init();
}

/*-------------------------------------------------------------------*/
FormationEditData::~FormationEditData()
{
    //std::cerr << "delete EditData" << std::endl;
}

/*-------------------------------------------------------------------*/
void
FormationEditData::init()
{
    M_modified = false;
    M_formation_data.reset();

    M_current_state.ball_.assign( 0.0, 0.0 );
    for ( size_t i = 0; i < M_current_state.players_.size(); ++i )
    {
        M_current_state.players_[i].assign( -3.0 * i + 3.0, -37.0 );
    }

    //M_select_type = NO_SELECT;
    M_select_index = 0;

    M_triangulation.clear();
}

/*-------------------------------------------------------------------*/
bool
FormationEditData::openConf( const std::string & filepath )
{
    M_formation = FormationParser::parse( filepath );
    if ( ! M_formation )
    {
        std::cerr << "(FormationEditData::openConf) Failed to read a formation [" << filepath << "]" << std::endl;
        return false;
    }

    init();

    M_formation_data = M_formation->toData();
    if ( ! M_formation_data )
    {
        std::cerr << "(FormationEditData::openConf) Failed to create a formation data [" << filepath << "]" << std::endl;
        return false;
    }

    M_filepath = filepath;

    updateTriangulation();
    updatePlayerPosition();
    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationEditData::saveConf()
{
    if ( M_filepath.empty() )
    {
        return false;
    }

    return saveConfAs( M_filepath );
}


/*-------------------------------------------------------------------*/
bool
FormationEditData::saveConfAs( const std::string & filepath )
{
    if ( ! M_formation )
    {
        return false;
    }

    if ( Options::instance().feditAutoBackup() )
    {
        std::filesystem::path backup_filepath = filepath;
        backup_filepath += "_";
        if ( ! M_formation->version().empty() )
        {
            backup_filepath += M_formation->version();
        }
        else if ( ! M_saved_datetime.empty() )
        {
            backup_filepath += M_saved_datetime;
        }
        else
        {
            backup_filepath += make_current_datetime_string();
        }

        std::error_code ec;
        std::filesystem::copy( filepath, backup_filepath,
                               std::filesystem::copy_options::none,
                               ec );
        if ( ec )
        {
            std::cerr << "Failed to backup the file [" << filepath  << "]"
                      // << "  category : " << ec.category().name() << '\n'
                      // << "  value : " << ec.value() << '\n'
                      << " " << ec.message() << std::endl;
        }
        else
        {
            std::cerr << "Copied to [" << backup_filepath << "]" << std::endl;
        }
    }

    M_saved_datetime = make_current_datetime_string();
    M_formation->setVersion( M_saved_datetime );

    std::ofstream fout( filepath );
    if ( ! fout.is_open() )
    {
        fout.close();
        return false;
    }

    if ( ! M_formation->print( fout ) )
    {
        return false;
    }

    fout.flush();
    fout.close();

    std::cerr << "Saved to  [" << filepath << "]" << std::endl;

    M_modified = false;
    M_filepath = filepath;
    return true;
}


/*-------------------------------------------------------------------*/
bool
FormationEditData::openData( const std::string & filepath )
{
    if ( ! M_formation )
    {
        std::cerr << "(FormationEditData::openData) No formation! Create a new one or open an exsiting one." << std::endl;
        return false;
    }

    M_triangulation.clear();

    std::ifstream fin( filepath );
    if ( ! fin )
    {
        std::cerr << "(FormationEditData::openData) Could not open the file " << filepath << std::endl;
        return false;
    }

    M_formation_data = FormationData::Ptr( new FormationData() );
    if ( ! M_formation_data->read( fin ) )
    {
        std::cerr << "(FormationEditData::openData) Could not read the file " << filepath << std::endl;
        return false;
    }

    M_current_index = -1;
    fitModel();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationEditData::saveDataAs( const std::string & filepath )
{
    if ( ! M_formation_data )
    {
        return false;
    }

    std::ofstream fout( filepath );
    if ( ! fout.is_open() )
    {
        fout.close();
        return false;
    }

    M_formation_data->print( fout );

    fout.flush();
    fout.close();
    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationEditData::openBackgroundConf( const std::string & filepath )
{
    M_background_formation = FormationParser::parse( filepath );
    if ( ! M_background_formation )
    {
        std::cerr << "(FormationEditData::openBackgroundConf) Failed to create a background formation. [" << filepath << "]" << std::endl;
        return false;
    }

    FormationData::Ptr background_data = M_background_formation->toData();
    if ( background_data )
    {
        M_background_triangulation.clear();

        for ( const FormationData::Data & d : background_data->dataCont() )
        {
            M_background_triangulation.addPoint( d.ball_ );
        }

        M_background_triangulation.compute();
    }

    return true;
}

/*-------------------------------------------------------------------*/
void
FormationEditData::updatePlayerPosition()
{
    if ( ! Options::instance().feditPlayerAutoMove() )
    {
        return;
    }

    if ( ! M_formation )
    {
        return;
    }

    M_formation->getPositions( M_current_state.ball_, M_current_state.players_ );

    for ( Vector2D & pos : M_current_state.players_ )
    {
        pos = round_coordinates( pos.x, pos.y );
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditData::updateTriangulation()
{
    if ( ! M_formation_data )
    {
        return;
    }

    M_triangulation.clear();

    for ( const FormationData::Data & d : M_formation_data->dataCont() )
    {
        M_triangulation.addPoint( d.ball_ );
    }

    M_triangulation.compute();

    std::cerr << "updateTriangulation"
              << "\n  edges=" << M_triangulation.edges().size()
              << "\n  triangles=" << M_triangulation.triangles().size()
              << std::endl;
}

/*-------------------------------------------------------------------*/
bool
FormationEditData::newFormation( const std::string & method_name )
{
    init();

    M_formation = Formation::create( method_name );

    if ( ! M_formation )
    {
        std::cerr << "(FormationEditData::createFormation) Failed to create a formation. method=" << method_name << std::endl;
        return false;
    }

    M_formation->setRole( 1, "Goalie", RoleType( RoleType::Goalie, RoleType::Center ), 0 );
    M_formation->setRole( 2, "CenterBack", RoleType( RoleType::Defender, RoleType::Left ), 3 );
    M_formation->setRole( 3, "CenterBack", RoleType( RoleType::Defender, RoleType::Right ), 2 );
    M_formation->setRole( 4, "SideBack", RoleType( RoleType::Defender, RoleType::Left ), 5 );
    M_formation->setRole( 5, "SideBack", RoleType( RoleType::Defender, RoleType::Right ), 4 );
    M_formation->setRole( 6, "DefensiveHalf", RoleType( RoleType::MidFielder, RoleType::Center ), 0 );
    M_formation->setRole( 7, "OffensiveHalf", RoleType( RoleType::MidFielder, RoleType::Left ), 8 );
    M_formation->setRole( 8, "OffensiveHalf", RoleType( RoleType::MidFielder, RoleType::Right ), 7 );
    M_formation->setRole( 9, "SideForward", RoleType( RoleType::Forward, RoleType::Left ), 10 );
    M_formation->setRole( 10, "SideForward", RoleType( RoleType::Forward, RoleType::Right ), 9 );
    M_formation->setRole( 11, "CenterForward", RoleType( RoleType::Forward, RoleType::Center ), 0 );

    M_formation_data = FormationData::Ptr( new FormationData() );
    if ( ! M_formation_data )
    {
        std::cerr << "(FormationEditData::createFormation) Failed to create a formation data." << std::endl;
        return false;
    }

    FormationData::Data data;
    data.ball_.assign( 0.0, 0.0 );
    data.players_.emplace_back( -50.0, 0.0 );
    data.players_.emplace_back( -20.0, -8.0 );
    data.players_.emplace_back( -20.0, 8.0 );
    data.players_.emplace_back( -18.0, -18.0 );
    data.players_.emplace_back( -18.0, 18.0 );
    data.players_.emplace_back( -15.0, 0.0 );
    data.players_.emplace_back( 0.0, -12.0 );
    data.players_.emplace_back( 0.0, 12.0 );
    data.players_.emplace_back( 10.0, -22.0 );
    data.players_.emplace_back( 10.0, 22.0 );
    data.players_.emplace_back( 10.0, 0.0 );

    M_formation_data->addData( data );

    fitModel();

    return true;
}

/*-------------------------------------------------------------------*/
void
FormationEditData::updateRoleData( const int num,
                                   const int paired_num,
                                   const std::string & role_name )
{
    if ( ! M_formation )
    {
        return;
    }

    M_formation->setRoleName( num, role_name );
    M_formation->setPositionPair( num, paired_num );

    M_modified = true;
}

/*-------------------------------------------------------------------*/
void
FormationEditData::updateRoleType( const int num,
                                   const int type_index,
                                   const int side_index )
{
    if ( ! M_formation )
    {
        return;
    }

    RoleType role_type = M_formation->roleType( num );
    role_type.setType( static_cast< RoleType::Type >( type_index ) );
    role_type.setSide( side_index == 1 ? RoleType::Left :
                       side_index == 2 ? RoleType::Right :
                       RoleType::Center );

    M_formation->setRoleType( num, role_type );

    M_modified = true;
}

/*-------------------------------------------------------------------*/
void
FormationEditData::moveBallTo( const double x,
                               const double y )
{
    Vector2D pos = round_coordinates( x, y );

    M_current_state.ball_ = pos;

    if ( Options::instance().feditDataAutoSelect() )
    {
        if ( pos.absY() < 1.0 )
        {
            M_current_state.ball_.y = 0.0;
        }

        if ( M_formation_data )
        {
            const int idx = M_formation_data->nearestDataIndex( pos, 1.0 );
            const FormationData::Data * data = M_formation_data->data( idx );
            if ( data )
            {
                M_current_index = idx;
                M_current_state.ball_ = data->ball_;
            }
        }
    }

    updatePlayerPosition();
}


/*-------------------------------------------------------------------*/
void
FormationEditData::movePlayerTo( const int num,
                                 const double x,
                                 const double y )
{
    if ( num < 1 || 11 < num )
    {
        std::cerr << "(FormationEditData::movePlayerTo) Illegal number " << num << std::endl;
        return;
    }

    try
    {
        Vector2D pos = round_coordinates( x, y );

        M_current_state.players_.at( num - 1 ) = pos;

        if ( Options::instance().feditPairMode()
             && M_current_state.ball_.absY() < 0.5
             && M_formation )
        {
            const int pair = M_formation->pairedNumber( num );
            if ( 1 <= pair && pair <= 11 )
            {
                M_current_state.players_.at( pair - 1 ).assign( x, -y );
            }
            else
            {
                for ( int n = 1; n <= 11; ++n )
                {
                    if ( n == num ) continue;
                    if ( M_formation->pairedNumber( n ) == num )
                    {
                        M_current_state.players_.at( n - 1 ).assign( x, -y );
                    }
                }
            }
        }
    }
    catch ( std::exception & e )
    {
        std::cerr << "(FormationEditData::movePlayerTo) exception caught! " << e.what() << std::endl;
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationEditData::moveSelectObjectTo( const double x,
                                       const double y )
{
    switch ( selectType() ) {
    case NO_SELECT:
        return false;
    case SELECT_BALL:
        moveBallTo( x, y );
        return true;
    case SELECT_PLAYER:
        movePlayerTo( selectIndex() + 1, x, y );
        return true;
    case SELECT_SAMPLE:
        // setConstraintTerminal( x, y );
        return true;
    default:
        break;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationEditData::selectObject( const double x,
                                 const double y )
{
    const Vector2D pos( x, y );
    const double dist2_thr = 1.5 * 1.5;

    double mindist2 = 200.0 * 200.0;

    //
    // ball
    //
    {
        double d2 = M_current_state.ball_.dist2( pos );
        if ( d2 < dist2_thr )
        {
            //std::cerr << "selection update ball" << std::endl;
            M_select_type = SELECT_BALL;
            mindist2 = d2;
        }
    }

    //
    // players
    //
    size_t index = 0;
    for ( std::vector< Vector2D >::iterator it = M_current_state.players_.begin();
          it != M_current_state.players_.end();
          ++it, ++index )
    {
        double d2 = it->dist2( pos );
        if ( d2 < dist2_thr
             && d2 < mindist2 )
        {
            M_select_type = SELECT_PLAYER;
            M_select_index = index; //std::distance( M_current_state.players_.begin(), it );
            mindist2 = d2;
        }
    }

    //
    // update the current state according to the given position
    //
    if ( M_select_type == SELECT_BALL )
    {
        M_current_state.ball_ = pos;
    }
    else if ( M_select_type == SELECT_PLAYER )
    {
        M_current_state.players_[M_select_index] = pos;
    }

    return ( M_select_type != NO_SELECT );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationEditData::releaseObject()
{
    if ( M_select_type != NO_SELECT )
    {
        M_select_type = NO_SELECT;
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::addData()
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    // add data
    std::string err = M_formation_data->addData( M_current_state );
    if ( ! err.empty() )
    {

    }

    // add paired data
    if ( Options::instance().feditPairMode()
         && M_current_state.ball_.absY() >= 0.5 )
    {
        FormationData::Data reversed = M_current_state;
        reversed.ball_.y *= -1.0;
        reverseYImpl( &reversed.players_ );
        err = M_formation_data->addData( reversed );
        if ( ! err.empty() )
        {

        }
    }

    M_current_state = M_formation_data->dataCont().back();
    M_current_index = M_formation_data->dataCont().size() - 1;

    fitModel();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::insertData( const int idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    if ( idx < 0 )
    {
        return std::string( "Invalid index" );
    }

    std::string err = M_formation_data->insertData( static_cast< size_t >( idx ), M_current_state );
    if ( ! err.empty() )
    {
        return err;
    }

    if ( Options::instance().feditPairMode() )
    {
        FormationData::Data reversed_data = M_current_state;
        reversed_data.ball_.y *= -1.0;
        reverseYImpl( &reversed_data.players_ );

        err = M_formation_data->insertData( static_cast< size_t >( idx + 1 ), reversed_data );
        if ( ! err.empty() )
        {
            return err;
        }
    }


    M_current_index = idx;

    fitModel();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::replaceData( const int idx )
{
    return replaceDataImpl( idx, M_current_state );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::replaceDataImpl( const int idx,
                                    const FormationData::Data & data )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }


    const FormationData::Data * original_data = M_formation_data->data( static_cast< size_t >( idx ) );
    if ( ! original_data )
    {
        return std::string( "Invalid index" );
    }
    const Vector2D original_ball = original_data->ball_;

    // replace data
    {
        std::string err = M_formation_data->replaceData( static_cast< size_t >( idx ), data );

        if ( ! err.empty() )
        {
            return err;
        }
    }

    // replace the paired data
    if ( Options::instance().feditPairMode()
         && data.ball_.absY() >= 0.5 )
    {
        size_t reversed_idx = size_t( -1 );

        for ( size_t i = 0; i < M_formation_data->dataCont().size(); ++i )
        {
            const FormationData::Data * r = M_formation_data->data( i );
            if ( r
                 && std::fabs( r->ball_.x - original_ball.x ) < 1.0e-5
                 && std::fabs( r->ball_.y + original_ball.y ) < 1.0e-5 )
            {
                reversed_idx = i;
                break;
            }
        }

        FormationData::Data reversed_data = data;
        reversed_data.ball_.y *= -1.0;
        reverseYImpl( &reversed_data.players_ );

        if ( reversed_idx != size_t( -1 ) )
        {
            std::string err = M_formation_data->replaceData( reversed_idx, reversed_data );
            if ( ! err.empty() )
            {
                return err;
            }
        }
        else
        {
            std::string err = M_formation_data->addData( reversed_data );
            if ( ! err.empty() )
            {
                std::cerr << "(EditData::replaceDataImpl) ERROR?" << std::endl;
            }
        }
    }

    fitModel();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::replaceBall( const int idx,
                                const double x,
                                const double y )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    const FormationData::Data * d = M_formation_data->data( static_cast< size_t >( idx ) );

    if ( ! d )
    {
        return std::string( "Invalid index" );
    }

    FormationData::Data tmp = *d;
    tmp.ball_.assign( x, y );

    return replaceDataImpl( idx, tmp );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::replacePlayer( const int idx,
                                  const int num,
                                  const double x,
                                  const double y )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    const FormationData::Data * d = M_formation_data->data( static_cast< size_t >( idx ) );

    if ( ! d )
    {
        return std::string( "Invalid index" );
    }

    FormationData::Data tmp = *d;
    try
    {
        tmp.players_.at( num - 1 ).assign( x, y );
    }
    catch ( std::exception & e )
    {
        std::cerr << e.what()
                  << ": EditData::replacePlayer() illegal player number. "
                  << num << std::endl;
        return std::string( "Illegal player number" );
    }

    return replaceDataImpl( idx, tmp );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::deleteData( const int idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    std::string err = M_formation_data->removeData( static_cast< size_t >( idx ) );

    if ( ! err.empty() )
    {
        return err;
    }

    M_current_index = -1;

    fitModel();

    return std::string();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
FormationEditData::changeDataIndex( const int old_idx,
                           const int new_idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return std::string( "No formation" );
    }

    size_t old_index = static_cast< size_t >( old_idx );
    size_t new_index = static_cast< size_t >( new_idx );

    if ( new_idx > old_idx )
    {
        new_index += 1;
    }

    std::string err = M_formation_data->changeDataIndex( old_index, new_index );

    if ( ! err.empty() )
    {
        return err;
    }

    std::cerr << "move data from " << old_idx << " to " << new_idx << std::endl;

    M_current_index = new_idx;

    fitModel();

    return std::string();
}

/*-------------------------------------------------------------------*/
bool
FormationEditData::setCurrentIndex( const int idx )
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return false;
    }

    if ( idx == -1 )
    {
        M_current_index = idx;
        return true;
    }

    const FormationData::Data * d = M_formation_data->data( idx );
    if ( ! d )
    {
        std::cerr << "(FormationEditData::setCurrentIndex) "
                  << "Index range over: " << idx
                  << std::endl;
        return false;
    }

    M_current_index = idx;

    if ( Options::instance().feditPlayerAutoMove() )
    {
        M_current_state = *d;
    }
    else
    {
        M_current_state.ball_ = d->ball_;
    }

    return true;
}

/*-------------------------------------------------------------------*/
void
FormationEditData::reverseY()
{
    if ( ! M_formation )
    {
        std::cerr << "***ERROR*** (FormationEditData) No formation!" << std::endl;
        return;
    }

    M_current_state.ball_.y *= -1.0;

    reverseYImpl( &M_current_state.players_ );
}

/*-------------------------------------------------------------------*/
void
FormationEditData::reverseYImpl( std::vector< Vector2D > * players )
{
    if ( ! M_formation )
    {
        std::cerr << "***ERROR*** (FormationEditData::reverseY) No formation." << std::endl;
        return;
    }

    std::vector< Vector2D > old_players = *players;

    int num = 0;
    for ( Vector2D & p : *players )
    {
        ++num;

        const int pair = M_formation->pairedNumber( num );
        if ( pair == 0 )
        {
            p.y *= -1.0;
        }
        else if ( 1 <= pair && pair <= 11 )
        {
            p.x = old_players[pair - 1].x;
            p.y = old_players[pair - 1].y * -1.0;
        }
        else
        {
            for ( int n = 1; n <= 11; ++n )
            {
                if ( M_formation->pairedNumber( n ) == num )
                {
                    p.x = old_players[n - 1].x;
                    p.y = old_players[n - 1].y * -1.0;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------*/
void
FormationEditData::fitModel()
{
    if ( ! M_formation
         || ! M_formation_data )
    {
        return;
    }

    M_formation->train( *M_formation_data );
    M_modified = true;
    updatePlayerPosition();
    updateTriangulation();
}
