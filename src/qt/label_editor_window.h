// -*-c++-*-

/*!
  \file label_editor_window.h
  \brief label editor window Header File.
*/

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

#ifndef LABEL_EDITOR_WINDOW_H
#define LABEL_EDITOR_WINDOW_H

#include <QMainWindow>

#include "features_log.h"

class QAction;
//class QStandardItemModel;
//class QTableView;
//class QTreeWidget;
class QTableWidget;

class MainData;


class LabelEditorWindow
    : public QMainWindow {

    Q_OBJECT

private:

    MainData & M_main_data;

    QToolBar * M_tool_bar;

    //QStandardItemModel * M_model;
    //QTableView * M_item_view;
    //QTreeWidget * M_tree_view;
    QTableWidget * M_table_view;

    QAction * M_open_act;
    QAction * M_save_act;


    WholeFeaturesLog::Ptr M_features_log;


    // not used
    LabelEditorWindow() = delete;
    LabelEditorWindow( const LabelEditorWindow & ) = delete;
    LabelEditorWindow & operator=( const LabelEditorWindow & ) = delete;

public:

    LabelEditorWindow( MainData & main_data,
                       QWidget * parent );

    ~LabelEditorWindow();

private:

    void createView();
    void createActions();
    void createMenus();
    void createMenuFile();
    void createToolBars();

public:
    bool saveChanges();

private:

    bool openFile( const QString & filepath );

private slots:

    void openFile();
    void saveData();

    void clearTable();

    void initTable();
    void updateTableContents();


};

#endif
