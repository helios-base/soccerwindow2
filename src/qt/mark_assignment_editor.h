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

#ifndef MARK_ASSIGNMENT_EDITOR_H
#define MARK_ASSIGNMENT_EDITOR_H

#include <QMainWindow>

// class QAction;
class QTableView;

class MarkAssignmentTableModel;
class MainData;

class MarkAssignmentEditor
    : public QMainWindow {

    Q_OBJECT

private:

    MainData & M_main_data;

    QTableView * M_mark_assignment_view;
    MarkAssignmentTableModel * M_model;

    // not used
    MarkAssignmentEditor() = delete;
    MarkAssignmentEditor( const MarkAssignmentEditor & ) = delete;
    MarkAssignmentEditor & operator=( const MarkAssignmentEditor & ) = delete;

public:

    MarkAssignmentEditor( MainData & main_data,
                          QWidget * parent );

    ~MarkAssignmentEditor();

    void clearAll();

private:

    void createView();
    void createActions();
    void createMenus();
    void createToolBars();

    bool openMarkCostFeaturesLog( const QString & file_path );

private slots:
    void openMarkCostFeaturesLog();

public slots:
    void syncCycle();

signals:
    void assignmentsChanged();

};


#endif
