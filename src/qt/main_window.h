// -*-c++-*-

/*!
  \file main_window.h
  \brief main application window class Header File.
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

#ifndef SOCCERWINDOW2_QT_MAIN_WINDOW_H
#define SOCCERWINDOW2_QT_MAIN_WINDOW_H

#include <QMainWindow>
#include <QPalette>

#include "main_data.h"

#include <memory>

class QAction;
class QActionGroup;
class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QImage;
class QLabel;
class QMenu;
class QPoint;
class QResizeEvent;
class QString;

class DebugMessageWindow;
class DebugServer;
class DetailDialog;
class FieldCanvas;
class FormationEditorWindow;
class LabelEditorWindow;
class LauncherDialog;
class LogPlayer;
class LogPlayerToolBar;
class MarkAssignmentEditor;
class MonitorClient;
class SimpleLabelSelector;
class TrainerDialog;
class PlayerTypeDialog;
class ViewConfigDialog;

class MainWindow
    : public QMainWindow {

    Q_OBJECT

private:

    MainData M_main_data;

    FieldCanvas * M_field_canvas;

    LogPlayer * M_log_player;

    LogPlayerToolBar * M_log_player_tool_bar;
    //QToolBar * M_monitor_tool_bar;

    QLabel * M_position_label;

    DetailDialog * M_detail_dialog;
    PlayerTypeDialog * M_player_type_dialog;
    TrainerDialog * M_trainer_dialog;
    ViewConfigDialog * M_view_config_dialog;
    LauncherDialog * M_launcher_dialog;

    FormationEditorWindow * M_formation_editor_window;
    LabelEditorWindow * M_label_editor_window;
    SimpleLabelSelector * M_simple_label_selector;
    MarkAssignmentEditor * M_mark_assignment_editor;

    DebugMessageWindow * M_debug_message_window;

    MonitorClient * M_monitor_client;
    DebugServer * M_debug_server;

    QString M_last_gamelog_dir;
    QString M_last_connected_host;

    //! the file name of the game log shown (empty for the frames received from a server)
    QString M_game_log_name;

    QString M_server_command;
    QStringList M_server_args;

    //! the palette of the desktop, saved at startup for the "system" color scheme
    QPalette M_system_palette;

    // file actions
    QAction * M_open_rcg_act;
    QAction * M_save_rcg_act;
    QAction * M_open_debug_view_act;
    QAction * M_save_debug_view_act;
    QAction * M_open_draw_data_act;
    QAction * M_save_image_act; //!< File > Save Image of This Frame
    QAction * M_exit_act;

    // monitor actions
    QAction * M_kick_off_act;
    QAction * M_set_live_mode_act;
    QAction * M_connect_monitor_act;
    QAction * M_connect_monitor_to_act;
    QAction * M_disconnect_monitor_act;
    QAction * M_kill_server_act;
    QAction * M_restart_server_act;
    QAction * M_show_trainer_dialog_act;
    QAction * M_show_launcher_dialog_act;
    QActionGroup * M_playmode_change_act_group;

    // view actions
    QAction * M_toggle_menu_bar_act;
    QAction * M_toggle_tool_bar_act;
    QAction * M_toggle_status_bar_act;
    QAction * M_full_screen_act;
    QAction * M_show_player_type_dialog_act;
    QAction * M_show_detail_dialog_act;
    QActionGroup * M_style_act_group;
    QActionGroup * M_ui_color_scheme_act_group;
    QAction * M_show_color_setting_dialog_act;
    QAction * M_show_font_setting_dialog_act;
    QAction * M_show_view_config_dialog_act;

    // view config actions
    QAction * M_zoom_in_act;
    QAction * M_zoom_out_act;
    QAction * M_fit_to_screen_act;
    QAction * M_toggle_enlarge_act;
    QAction * M_toggle_reverse_side_act;
    QAction * M_toggle_player_reverse_draw_act;
    QAction * M_toggle_show_player_number_act;
    QAction * M_toggle_show_player_type_act;
    QAction * M_toggle_show_stamina_act;
    QAction * M_toggle_show_view_area_act;
    QAction * M_toggle_show_focus_point_act;
    QAction * M_toggle_show_body_shadow_act;
    QAction * M_toggle_show_catchable_area_act;
    QAction * M_toggle_show_tackle_area_act;
    QAction * M_toggle_show_kick_accel_area_act;
    QAction * M_toggle_show_pointto_act;
    QAction * M_toggle_show_attentionto_act;
    QAction * M_toggle_show_score_board_act;
    QAction * M_toggle_show_team_graphic_act;
    QAction * M_toggle_show_ball_act;
    QAction * M_toggle_show_players_act;
    QAction * M_toggle_show_flags_act;
    QAction * M_toggle_show_offside_line_act;
    QAction * M_select_left_agent_act[12];
    QAction * M_select_right_agent_act[12];
    QAction * M_toggle_focus_ball_act;
    QAction * M_toggle_focus_player_act;
    QAction * M_toggle_select_auto_all_act;
    QAction * M_toggle_select_auto_left_act;
    QAction * M_toggle_select_auto_right_act;
    QAction * M_unselect_act;

    // log player actions
    QAction * M_log_player_go_first_act;
    QAction * M_log_player_go_prev_score_act;
    QAction * M_log_player_one_step_back_act;
    QAction * M_log_player_play_or_stop_act;
    QAction * M_log_player_one_step_forward_act;
    QAction * M_log_player_go_next_score_act;
    QAction * M_log_player_go_last_act;
    QAction * M_log_player_shift_down_act;
    QAction * M_log_player_shift_up_act;
    QAction * M_log_player_stop_act;
    QAction * M_log_player_play_back_act;
    QAction * M_log_player_play_forward_act;

    // editor actions
    QAction * M_show_formation_editor_window_act;
    QAction * M_show_label_editor_window_act;
    QAction * M_show_simple_label_selector_act;
    QAction * M_show_mark_assignment_editor_act;

    // tool actions
    QAction * M_show_debug_message_window_act;
    QAction * M_toggle_debug_server_act;
    QAction * M_show_image_save_dialog_act;

    // help actions
    QAction * M_about_act;
    QAction * M_shortcut_keys_act;

    // not used
    MainWindow( const MainWindow & );
    const MainWindow & operator=( const MainWindow & );

public:

    MainWindow();
    ~MainWindow();

    //! initialize application settings using command line options.
    void init();

    //! open a game log file (File > Open rcg file...)
    void openRCG( const QString & file_path );

    /*!
      \brief the frame shown, drawn into an image as large as the view (with
      its device pixel ratio). docs/plan/2026-11-28-00_frame-export.md 4.1.
      \return the image, or a null image if there is no frame
    */
    QImage renderImage() const;

    /*!
      \brief the default file name of Save Image of This Frame: the name of
      the game log without .rcg(.gz) (or the program's), and the cycle of the
      frame shown (1234, or 1234.3 when stopped), e.g. "head_v5-1.png"
    */
    QString defaultImageName() const;

    /*!
      \brief save the frame shown to an image file (Save Image of This Frame
      without the file dialog). The format is given by the extension (PNG if
      there is none).
      \param path the file path
      \param error the reason of a failure, if not null
      \return true if saved
    */
    bool saveImage( const QString & path,
                    QString * error ) const;

private:

    void readSettings();
    void saveSettings();
    void readShortcutKeysSettings();
    void saveShortcutKeysSettings();

    void createActions();
    void createActionsFile();
    void createActionsMonitor();
    void createActionsView();
    void createActionsViewConfig();
    void createActionsLogPlayer();
    void createActionsEditor();
    void createActionsDebug();
    void createActionsHelp();

    void createMenus();
    void createMenuFile();
    void createMenuMonitor();
    void createMenuLogPlayer();
    void createMenuView();
    void createMenuEditor();
    void createMenuDebug();
    void createMenuHelp();

    void createToolBars();
    void createStatusBar();

    void createFieldCanvas();
    QMenu * createNormalPopupMenu();
    QMenu * createSystemPopupMenu();
    QMenu * createMonitorPopupMenu();
    QMenu * createOfflinePopupMenu();

    void createViewConfigDialog();

    //! apply the color scheme of the widgets given by Options.
    void applyUIColorScheme();

protected:

    // overrided method (virtual in super)
    void closeEvent( QCloseEvent * event );
    void resizeEvent( QResizeEvent * event );
    void wheelEvent( QWheelEvent * event );

    void dragEnterEvent( QDragEnterEvent * event );
    void dropEvent( QDropEvent * event );

private:

    void connectMonitorTo( const char * hostname );
    void showConnectionFailure( const QString & reason );

    void openDrawData( const QString & file_path );

private slots:

    // file menu actions slots
    void openRCG();
    void saveRCG();
    void openDebugView();
    void saveDebugView();
    void openDrawData();
    void saveImage();
    // QWidget::close() is used as a quit action's slot.

    // monitor menu actions slots
    void kickOff();
    void setLiveMode();
    void connectMonitor();
    void connectMonitorTo();
    void disconnectMonitor();
    void showMonitorError( const QString & msg );
    void killServer();
    void startServer();
    void restartServer();
    void restartServer( const QString & command,
                        const QStringList & args );
    void showLauncherDialog();
    void changePlayMode( int mode,
                         const QPoint & point );

    // view menu actions slots
    void toggleMenuBar();
    void toggleToolBar();
    void toggleStatusBar();
    void toggleFullScreen();
    void showPlayerTypeDialog();
    void showDetailDialog();
    void changeStyle( bool checked );
    void changeUIColorScheme( QAction * action );
    void showColorSettingDialog();
    void showFontSettingDialog();
    void showTrainerDialog();
    void showViewConfigDialog();

    // editor menu actions slots
    void showFormationEditorWindow();

    // tool menu actions slots
    void showDebugMessageWindow();
    void toggleDebugServer( bool on );
    void startDebugServer();
    void stopDebugServer();
    void showImageSaveDialog();

    // help menu actions slots
    void about();
    void showShortcutKeys();

    void resizeCanvas( const QSize & size );

    void saveImageAndQuit();

public slots:

    void receiveMonitorPacket();

    void updatePositionLabel( const QPoint & point );

    void dropBallThere();
    void dropBall( const QPoint & pos );
    void freeKickLeft( const QPoint & pos );
    void freeKickRight( const QPoint & pos );

    void moveObjects();

    void yellowCard();
    void redCard();

signals:

    void viewUpdated();

};

#endif
