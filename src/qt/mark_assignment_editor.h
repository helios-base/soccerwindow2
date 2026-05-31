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

#include <rcsc/game_time.h>

#include <map>
#include <set>
#include <string>

class QAction;
class QTableView;
class QLabel;

class MarkAssignmentTableModel;
class MainData;
class MarkAssignment;

class MarkAssignmentEditor
    : public QMainWindow {

    Q_OBJECT

private:

    bool M_initial_auto_resize_done;

    MainData & M_main_data;

    QTableView * M_mark_assignment_view;
    MarkAssignmentTableModel * M_model;

    QAction * M_accept_group_act;

    QLabel * M_time_label;

    rcsc::GameTime M_current_time;
    std::set< rcsc::GameTime, rcsc::GameTime::Less > M_modified_times;

    // not used
    MarkAssignmentEditor() = delete;
    MarkAssignmentEditor( const MarkAssignmentEditor & ) = delete;
    MarkAssignmentEditor & operator=( const MarkAssignmentEditor & ) = delete;

public:

    MarkAssignmentEditor( MainData & main_data,
                          QWidget * parent );

    ~MarkAssignmentEditor();

    void clearAll();

    bool checkAndWarnUnsavedChanges();

private:

    void createView();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    void adjustWindowSizeToTable();


protected:
    void showEvent( QShowEvent * event ) override;
    void closeEvent( QCloseEvent * event ) override;

private:
    bool openMarkCostFeaturesLog( const QString & file_path );
    void saveChanges( const QString & file_path );

private slots:
    void openMarkCostFeaturesLog();
    void saveChanges();
    void saveChangesAs();

    void acceptGroup( bool checked );
    void applyChanges();

public slots:
    void syncTime();

signals:
    void assignmentsChanged();

};

#endif
