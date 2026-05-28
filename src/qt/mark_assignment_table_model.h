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

#ifndef MARK_ASSIGNMENT_TABLE_MODEL_H
#define MARK_ASSIGNMENT_TABLE_MODEL_H

#include <QAbstractTableModel>

#include "mark_cost_features_log.h"
#include "mark_assignment_table_model.h"

#include <rcsc/geom/vector_2d.h>

#include <vector>
#include <map>
#include <memory>

/*-------------------------------------------------------------------*/

struct MarkTargetKey {
    int unum_;
    rcsc::Vector2D pos_;

    MarkTargetKey( const int unum,
                   const rcsc::Vector2D & pos )
        : unum_( unum ),
          pos_( pos )
    { }

    bool isSameTarget( const int other_unum,
                       const rcsc::Vector2D & other_pos ) const
    {
        if ( unum_ > 0 && other_unum > 0 )
        {
            return unum_ == other_unum;
        }

        return ( pos_ - other_pos ).r2() < std::pow( 0.001, 2 );
    }

};

/*-------------------------------------------------------------------*/

class MarkAssignmentTableModel
    : public QAbstractTableModel {

    Q_OBJECT

private:

    std::vector< int > M_marker_unums;
    std::vector< MarkTargetKey > M_targets;
    std::map< int, int > M_assignments; // marker_unum -> target index

public:

    MarkAssignmentTableModel( QObject * parent )
        : QAbstractTableModel( parent )
    { }

    ~MarkAssignmentTableModel()
    { }

    int rowCount( const QModelIndex & /* parent = QModelIndex() */ ) const override
    {
        return M_marker_unums.size();
    }

    int columnCount( const QModelIndex & /* parent = QModelIndex() */ ) const override
    {
        return M_targets.size();
    }

    QVariant data( const QModelIndex & index,
                   int role ) const override;
    QVariant headerData( int section,
                         Qt::Orientation orientation,
                         int role ) const override;
    bool setData( const QModelIndex & index,
                  const QVariant &, int role ) override;
    Qt::ItemFlags flags( const QModelIndex & index ) const override;

    //

    void setAssignments( const std::vector< MarkAssignment::Ptr > & assignments );
    std::vector< std::pair< int, MarkTargetKey > > getAssignments() const;

private:
    bool hasColumnConflict( int col ) const;

};

#endif

