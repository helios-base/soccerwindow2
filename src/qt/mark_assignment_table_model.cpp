// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGlobal>

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) )
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "mark_assignment_table_model.h"

/*-------------------------------------------------------------------*/
QVariant
MarkAssignmentTableModel::data( const QModelIndex & index,
                                int role ) const
{
    if ( ! index.isValid() )
    {
        return QVariant();
    }

    const int marker_index = index.row();
    const int target_index = index.column();

    // const int marker_unum = M_markers[marker_index].unum_;
    // const MarkTargetKey & target = M_targets[target_index];

    const bool is_assigned = ( 0 <= marker_index
                               && marker_index < static_cast< int >( M_assignments.size() )
                               && M_assignments[marker_index] == target_index );

    switch ( role )
    {
    case Qt::CheckStateRole:
        return is_assigned ? Qt::Checked : Qt::Unchecked;

    case Qt::BackgroundRole:
        // If the target is assigned to another marker, show the cell in red.
        if ( is_assigned
             && hasColumnConflict( target_index ) )
        {
            return QColor( 255, 100, 100 );
        }
        return QVariant();

    default:
        break;
    }

    return QVariant();
}

/*-------------------------------------------------------------------*/
QVariant
MarkAssignmentTableModel::headerData( int section,
                                      Qt::Orientation orientation,
                                      int role ) const
{
    if ( role != Qt::DisplayRole )
    {
        return QVariant();
    }

    if ( orientation == Qt::Vertical )
    {
        if ( 0 <= section
             && section < static_cast< int >( M_markers.size() ) )
        {
            return QString::number( M_markers[section].unum_ );
        }
        else
        {
            return QVariant();
        }
    }

    if ( orientation == Qt::Horizontal )
    {
        if ( 0 <= section
             && section < static_cast< int >( M_targets.size() ) )
        {
            const MarkTargetKey & target = M_targets[section];
            if ( target.unum_ > 0 )
            {
                return QString::number( target.unum_ );
            }
            else
            {
                return QString( "(%1, %2)" )
                    .arg( target.pos_.x, 0, 'f', 1 )
                    .arg( target.pos_.y, 0, 'f', 1 );
            }
        }
        else
        {
            return QVariant();
        }
    }

    return QVariant();
}

/*-------------------------------------------------------------------*/
bool
MarkAssignmentTableModel::setData( const QModelIndex & index,
                                   const QVariant & value,
                                   int role )
{
    if ( ! index.isValid() ) return false;
    if ( role != Qt::CheckStateRole ) return false;

    const int marker_index = index.row();
    // const int marker_unum = M_marker_unums[marker_index];
    const int target_index = index.column();
    // const MarkTargetKey & target = M_targets[target_index];

    if ( value.toInt() == Qt::Checked )
    {
        // If the target index is same as the current assignment, do nothing.
        if ( 0 <= marker_index
             && marker_index < static_cast< int >( M_assignments.size() )
             && M_assignments[marker_index] == target_index )
        {
            return false;
        }

        const int old_target_index = ( ( 0 <= marker_index
                                         && marker_index < static_cast< int >( M_assignments.size() ) )
                                           ? M_assignments[marker_index]
                                           : -1 );

        // If the user checks the checkbox, assign the marker to the new target.
        M_assignments[marker_index] = target_index;

        // If the anther target is already assigned,
        // emit dataChanged for the old target column to update the background color.
        if ( old_target_index >= 0
             && old_target_index != target_index )
        {
            emit dataChanged( createIndex( 0, old_target_index ),
                              createIndex( rowCount( QModelIndex() ) - 1, old_target_index ),
                              { Qt::CheckStateRole, Qt::BackgroundRole } );
        }

        emit dataChanged( createIndex( 0, target_index ),
                          createIndex( rowCount( QModelIndex() ) - 1, target_index ),
                          { Qt::CheckStateRole, Qt::BackgroundRole } );
    }
    else
    {
        // If the user unchecks the checkbox, remove the assignment.
        if ( 0 <= marker_index
             && marker_index < static_cast< int >( M_assignments.size() ) )
        {
            M_assignments[marker_index] = -1;
        }

        emit dataChanged( createIndex( 0, target_index ),
                          createIndex( rowCount( QModelIndex() ) - 1, target_index ),
                          { Qt::CheckStateRole, Qt::BackgroundRole } );
    }

    return true;
}

/*-------------------------------------------------------------------*/
Qt::ItemFlags
MarkAssignmentTableModel::flags( const QModelIndex & index ) const
{
    if ( ! index.isValid() )
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

/*-------------------------------------------------------------------*/
void
MarkAssignmentTableModel::setAssignmentGroup( const MarkAssignmentGroup & group )
{
    this->beginResetModel();

    M_current_group_id.clear();
    M_markers.clear();
    M_targets.clear();
    M_assignments.clear();
    M_move_points.clear();

    M_current_group_id = group.group_id_;

    for ( const MarkAssignment & a : group.assignments_ )
    {
        if ( std::find( M_markers.begin(), M_markers.end(), a.marker_ ) == M_markers.end() )
        {
            M_markers.push_back( a.marker_ );
        }
        if ( std::find( M_targets.begin(), M_targets.end(), a.target_ ) == M_targets.end() )
        {
            M_targets.push_back( a.target_ );
        }
    }

    std::sort( M_markers.begin(), M_markers.end(),
               []( const Marker & a, const Marker & b )
               {
                   return a.unum_ < b.unum_;
               } );
    std::sort( M_targets.begin(), M_targets.end(),
               []( const MarkTargetKey & a, const MarkTargetKey & b )
               {
                   //    return a.pos_.x < b.pos_.x;
                   if ( a.unum_ > 0 && b.unum_ > 0 )
                   {
                       return a.unum_ < b.unum_;
                   }
                   else if ( a.unum_ > 0 )
                   {
                       return true;
                   }
                   else if ( b.unum_ > 0 )
                   {
                       return false;
                   }
                   else
                   {
                       return a.pos_.x < b.pos_.x;
                   }
               } );

    M_assignments.resize( M_markers.size(), -1 );
    M_move_points.assign( M_markers.size(),
                          std::vector< rcsc::Vector2D >( M_targets.size(), rcsc::Vector2D::INVALIDATED ) );
    for ( const MarkAssignment & a : group.assignments_ )
    {
        const int marker_index = std::find( M_markers.begin(), M_markers.end(), a.marker_ ) - M_markers.begin();
        const int target_index = std::find( M_targets.begin(), M_targets.end(), a.target_ ) - M_targets.begin();
        if ( 0 <= marker_index && marker_index < static_cast< int >( M_markers.size() )
             && 0 <= target_index && target_index < static_cast< int >( M_targets.size() ) )
        {
            M_move_points[marker_index][target_index] = a.move_point_;
        }
        if ( ! a.assigned_ ) continue;
        M_assignments[marker_index] = target_index;
    }

    this->endResetModel();
}

/*-------------------------------------------------------------------*/
MarkAssignmentGroup
MarkAssignmentTableModel::getAssignmentGroup() const
{
    for ( size_t i = 0; i < M_assignments.size(); ++i )
    {
        if ( hasColumnConflict( M_assignments[i] ) )
        {
            std::cerr << "Warning: Column conflict detected for target index " << M_assignments[i] << std::endl;
            return MarkAssignmentGroup( "" );
        }
    }

    MarkAssignmentGroup result( M_current_group_id );
    result.assignments_.reserve( M_assignments.size() );

    for ( size_t i = 0; i < M_assignments.size(); ++i )
    {
        const int target_index = M_assignments[i];
        if ( 0 <= target_index && target_index < static_cast< int >( M_targets.size() ) )
        {
            result.assignments_.emplace_back( true,
                                              M_markers[i].unum_, M_markers[i].pos_,
                                              M_targets[target_index].id_, M_targets[target_index].unum_,
                                              M_targets[target_index].pos_,
                                              M_move_points[i][target_index] );
        }
    }

    return result;
}

/*-------------------------------------------------------------------*/
bool
MarkAssignmentTableModel::hasColumnConflict( int target_index ) const
{
    int count = 0;
    for ( size_t i = 0; i < M_assignments.size(); ++i )
    {
        if ( M_assignments[i] == target_index ) ++count;
        if ( count > 1 ) return true;
    }
    return false;
}
