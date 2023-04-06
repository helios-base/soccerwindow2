// -*-c++-*-

/*!
  \file edit_data.h
  \brief formation editor data class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#ifndef FORMATION_EDIT_DATA_H
#define FORMATION_EDIT_DATA_H

#include <string>

#include <rcsc/formation/formation_data.h>
#include <rcsc/formation/formation.h>
#include <rcsc/geom/triangulation.h>
#include <rcsc/geom/vector_2d.h>

class FormationEditData {
public:

    static const double MAX_X;
    static const double MAX_Y;

    enum SelectType {
        SELECT_BALL,
        SELECT_PLAYER,
        SELECT_SAMPLE,
        NO_SELECT,
    };

private:

    std::string M_filepath;
    bool M_modified;
    std::string M_saved_datetime; //!< YYYYmmdd-HHMMSS

    rcsc::FormationData::Data M_current_state; //!< current state on the edit canvas.

    rcsc::Formation::Ptr M_formation; //!< current trained formation
    rcsc::FormationData::Ptr M_formation_data; //!< training data
    rcsc::Triangulation M_triangulation; //!< delaunay triangulation constructed by the recoreded training data

    rcsc::Formation::Ptr M_background_formation;
    rcsc::Triangulation M_background_triangulation;

    int M_current_index;
    SelectType M_select_type;
    int M_select_index;

    // not used
    FormationEditData( const FormationEditData & ) = delete;
    FormationEditData & operator=( const FormationEditData & ) = delete;

public:
    FormationEditData();
    ~FormationEditData();

private:
    void init();

public:

    const std::string & filePath() const
      {
          return M_filepath;
      }

    bool isModified() const
      {
          return M_modified;
      }

    const rcsc::FormationData::Data & currentState() const
      {
          return M_current_state;
      }

    rcsc::Formation::ConstPtr formation() const
      {
          return M_formation;
      }

    rcsc::FormationData::ConstPtr formationData() const
      {
          return M_formation_data;
      }

    const rcsc::Triangulation & triangulation() const
      {
          return M_triangulation;
      }

    rcsc::Formation::ConstPtr backgroundFormation() const
      {
          return M_background_formation;
      }

    const rcsc::Triangulation & backgroundTriangulation() const
      {
          return M_background_triangulation;
      }

    int currentIndex() const
      {
          return M_current_index;
      }

    SelectType selectType() const
      {
          return M_select_type;
      }

    int selectIndex() const
      {
          return M_select_index;
      }

    bool newFormation( const std::string & type_name );
    bool openConf( const std::string & filepath );
    bool saveConf();
    bool saveConfAs( const std::string & filepath );

    bool openData( const std::string & filepath );
    bool saveDataAs( const std::string & filepath );

    bool openBackgroundConf( const std::string & filepath );

private:
    void updateTriangulation();
    void updatePlayerPosition();

public:

    void updateRoleData( const int num,
                         const int paired_unum,
                         const std::string & role_name );
    void updateRoleType( const int num,
                         const int type_index,
                         const int side_index );

    void moveBallTo( const double x,
                     const double y );
    void movePlayerTo( const int num,
                       const double x,
                       const double y );

    bool moveSelectObjectTo( const double x,
                             const double y );

    bool selectObject( const double x,
                       const double y );
    bool releaseObject();

    std::string addData();
    std::string insertData( const int idx );
    std::string replaceData( const int idx );
private:
    std::string replaceDataImpl( const int idx,
                                 const rcsc::FormationData::Data & data );
public:
    std::string replaceBall( const int idx,
                             const double x,
                             const double y );
    std::string replacePlayer( const int idx,
                               const int unum,
                               const double x,
                               const double y );
    std::string deleteData( const int idx );
    std::string changeDataIndex( const int old_idx,
                                 const int new_idx );

    bool setCurrentIndex( const int idx );
    void reverseY();
private:
    void reverseYImpl( std::vector< rcsc::Vector2D > * players );
public:
    void fitModel();
};

#endif
