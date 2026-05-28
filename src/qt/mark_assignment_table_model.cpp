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

    const int col = index.column();

    const int marker_unum = M_marker_unums[index.row()];
    // const MarkTargetKey & target = M_targets[index.column()];
    const std::map< int, int >::const_iterator it = M_assignments.find( marker_unum );
    const bool is_assigned = ( it != M_assignments.end() && it->second == index.column() );

    switch ( role )
    {
    case Qt::CheckStateRole:
        return is_assigned ? Qt::Checked : Qt::Unchecked;

    case Qt::BackgroundRole:
        // If the target is assigned to another marker, show the cell in red.
        if ( is_assigned
             && hasColumnConflict( col ) )
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
             && section < static_cast< int >( M_marker_unums.size() ) )
        {
            return QString::number( M_marker_unums[section] );
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

    const int marker_unum = M_marker_unums[index.row()];
    const int target_index = index.column();

    if ( value.toInt() == Qt::Checked )
    {
        const std::map< int, int >::const_iterator it = M_assignments.find( marker_unum );

        // If the target index is same as the current assignment, do nothing.
        if ( it != M_assignments.end()
             && it->second == target_index )
        {
            return false;
        }

        const int old_col = ( it != M_assignments.end()
                                  ? it->second
                                  : -1 );

        // If the user checks the checkbox, assign the marker to the new target.
        M_assignments[marker_unum] = target_index;

        // If the anther target is already assigned, unassign it.
        if ( old_col >= 0
             && old_col != target_index )
        {
            emit dataChanged( createIndex( 0, old_col ),
                              createIndex( rowCount( QModelIndex() ) - 1, old_col ),
                              { Qt::CheckStateRole, Qt::BackgroundRole } );
        }

        emit dataChanged( createIndex( 0, target_index ),
                          createIndex( rowCount( QModelIndex() ) - 1, target_index ),
                          { Qt::CheckStateRole, Qt::BackgroundRole } );
    }
    else
    {
        // If the user unchecks the checkbox, remove the assignment.
        M_assignments.erase( marker_unum );

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
MarkAssignmentTableModel::setAssignments( const std::vector< MarkAssignment::Ptr > & assignments )
{
    this->beginResetModel();

    M_marker_unums.clear();
    M_targets.clear();
    M_assignments.clear();

    for ( const MarkAssignment::Ptr & a : assignments )
    {
        M_marker_unums.push_back( a->marker_unum_ );
        M_targets.emplace_back( a->target_unum_, a->target_pos_ );
        M_assignments[a->marker_unum_] = M_targets.size() - 1;
    }

    std::sort( M_marker_unums.begin(), M_marker_unums.end() );

    // Build a sorted permutation of target indices.
    // Targets with a valid unum are sorted by unum; others are sorted by x position.
    const int n_targets = static_cast< int >( M_targets.size() );
    std::vector< int > perm( n_targets );
    for ( int i = 0; i < n_targets; ++i )
    {
        perm[i] = i;
    }
    std::sort( perm.begin(), perm.end(),
               [&]( int a, int b )
               {
                   const MarkTargetKey & ta = M_targets[a];
                   const MarkTargetKey & tb = M_targets[b];
                   //    if ( ta.unum_ > 0 && tb.unum_ > 0 )
                   //    {
                   //        return ta.unum_ < tb.unum_;
                   //    }
                   return ta.pos_.x < tb.pos_.x;
               } );

    // Build inverse permutation: old_index -> new_index
    std::vector< int > inv_perm( n_targets );
    for ( int i = 0; i < n_targets; ++i )
    {
        inv_perm[perm[i]] = i;
    }

    // Apply permutation to M_targets
    std::vector< MarkTargetKey > sorted_targets;
    sorted_targets.reserve( n_targets );
    for ( int idx : perm )
    {
        sorted_targets.push_back( M_targets[idx] );
    }
    M_targets = std::move( sorted_targets );

    // Update M_assignments to use the new (post-sort) target indices
    for ( auto & [unum, target_idx] : M_assignments )
    {
        target_idx = inv_perm[target_idx];
    }

    this->endResetModel();
}

/*-------------------------------------------------------------------*/
std::vector< std::pair< int, MarkTargetKey > >
MarkAssignmentTableModel::getAssignments() const
{
    std::vector< std::pair< int, MarkTargetKey > > result;
    result.reserve( M_assignments.size() );

    for ( const auto & [marker_unum, target_index] : M_assignments )
    {
        if ( target_index < 0
             || target_index >= static_cast< int >( M_targets.size() ) )
        {
            continue;
        }

        result.emplace_back( marker_unum, M_targets[target_index] );
    }

    return result;
}

/*-------------------------------------------------------------------*/
bool
MarkAssignmentTableModel::hasColumnConflict( int col ) const
{
    int count = 0;
    for ( const auto & [unum, index] : M_assignments )
    {
        if ( index == col ) ++count;
        if ( count > 1 ) return true;
    }
    return false;
}
