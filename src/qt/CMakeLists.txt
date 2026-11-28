# GUI library: all the GUI sources except main.cpp.
# It is linked to the executable and to the tests.
add_library(soccerwindow2_gui STATIC
  action_sequence_option_window.cpp
  action_sequence_selector.cpp
  action_sequence_tree_view.cpp
  ball_painter.cpp
  ball_painter_rcss.cpp
  ball_trace_painter.cpp
  color_setting_dialog.cpp
  connect_dialog.cpp
  coordinate_delegate.cpp
  debug_log_dir_dialog.cpp
  debug_log_painter.cpp
  debug_message_window.cpp
  debug_painter.cpp
  debug_server.cpp
  detail_dialog.cpp
  dir_selector.cpp
  draw_config.cpp
  draw_data_painter.cpp
  features_log_painter.cpp
  field_canvas.cpp
  field_painter.cpp
  font_setting_dialog.cpp
  formation_data_view.cpp
  formation_editor_painter.cpp
  formation_editor_window.cpp
  frame_scope.cpp
  image_save_dialog.cpp
  label_editor_window.cpp
  launcher_dialog.cpp
  log_player.cpp
  log_player_tool_bar.cpp
  main_window.cpp
  mark_assignment_editor.cpp
  mark_assignment_painter.cpp
  mark_assignment_table_model.cpp
  message_splitter.cpp
  monitor_client.cpp
  monitor_transport.cpp
  offside_line_painter.cpp
  player_control_painter.cpp
  player_painter.cpp
  player_painter_rcss.cpp
  player_trace_painter.cpp
  player_type_dialog.cpp
  score_board_painter.cpp
  score_board_painter_rcss.cpp
  simple_label_selector.cpp
  shortcut_keys_dialog.cpp
  tcp_monitor_transport.cpp
  team_graphic_painter.cpp
  trainer_dialog.cpp
  udp_monitor_transport.cpp
  ui_icon.cpp
  ui_palette.cpp
  ui_style.cpp
  view_config_dialog.cpp
  voronoi_diagram_painter.cpp
  # icons embedded with the Qt resource system (see docs/icons.md)
  ${PROJECT_SOURCE_DIR}/icons/icons.qrc
  )

set_target_properties(soccerwindow2_gui
  PROPERTIES
  AUTOMOC ON
  AUTORCC ON
  )

# config.h is taken from the build directory (the include directories of
# model). The top source directory is not added: config.h made there by an
# in-tree Autotools build would be included instead.
target_include_directories(soccerwindow2_gui
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(soccerwindow2_gui
  PUBLIC
  model
  Qt::Core
  Qt::Gui
  Qt::Widgets
  Qt::Network
  )

if(ZLIB_FOUND)
  target_link_libraries(soccerwindow2_gui PUBLIC ZLIB::ZLIB)
endif()

# the WebSocket connection to rcssserver (Qt WebSockets is optional)
if(HAVE_QT_WEBSOCKETS)
  target_sources(soccerwindow2_gui PRIVATE ws_monitor_transport.cpp)
  target_link_libraries(soccerwindow2_gui PUBLIC Qt::WebSockets)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(soccerwindow2_gui PRIVATE -W -Wall)
endif()

# executable
add_executable(soccerwindow2
  main.cpp
  )

set_target_properties(soccerwindow2
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
  )

target_link_libraries(soccerwindow2
  PRIVATE
  soccerwindow2_gui
  )

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(soccerwindow2 PRIVATE -W -Wall)
endif()

install(TARGETS soccerwindow2
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
