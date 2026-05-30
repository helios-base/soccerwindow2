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

class MarkAssignmentTableModel
    : public QAbstractTableModel {

    Q_OBJECT

private:

    std::string M_current_group_id;
    std::vector< Marker > M_markers;
    std::vector< MarkTargetKey > M_targets; 

    // size = M_markers.size()
    // M_assignments[marker_index]=target_index or -1 if no assignment
    std::vector< int > M_assignments; 
public:

    MarkAssignmentTableModel( QObject * parent )
        : QAbstractTableModel( parent )
    { }

    ~MarkAssignmentTableModel()
    { }

    int rowCount( const QModelIndex & /* parent = QModelIndex() */ ) const override
    {
        return M_markers.size();
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

    void setAssignmentGroup( const MarkAssignmentGroup & group );

    MarkAssignmentGroup getAssignmentGroup() const;

private:
    // target_index == column index
    bool hasColumnConflict( int target_index ) const;

};

#endif

