// -*-c++-*-

/*!
  \file formation_data_view.h
  \brief formation data view class Header File.
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

#ifndef FORMAITON_DATA_VIEW_H
#define FORMAITON_DATA_VIEW_H

#include <QTreeWidget>

#include <memory>

class MainData;

class FormationDataView
    : public QTreeWidget {

    Q_OBJECT

private:


    const MainData & M_main_data;


    FormationDataView() = delete;
public:

    explicit
    FormationDataView( const MainData & main_data,
                       QWidget * parent = 0 );
    ~FormationDataView();

    void updateData();

    void selectData( int idx );
    void unselectData();

protected:

    void showEvent( QShowEvent * event );

    void contextMenuEvent( QContextMenuEvent * event );
    void dropEvent( QDropEvent * event );

private slots:

    void setCurrentData( QTreeWidgetItem * current );
    void menuChangeDataIndex();
    void menuDeleteData();
    void changeCoordinateValue( const QModelIndex & index,
                                double value );

signals:

    void dataSelected( int idx );
    void indexChangeRequested( int old_shown_index,
                               int new_shown_index );
    void deleteRequested( int idx );

    void ballReplaced( int idx, double x, double y );
    void playerReplaced( int idx, int unum, double x, double y );

};

#endif
