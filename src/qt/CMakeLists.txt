
add_executable(soccerwindow2
  action_sequence_option_window.cpp
  action_sequence_selector.cpp
  action_sequence_tree_view.cpp
  ball_painter.cpp
  ball_painter_rcss.cpp
  ball_trace_painter.cpp
  color_setting_dialog.cpp
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
  image_save_dialog.cpp
  label_editor_window.cpp
  launcher_dialog.cpp
  log_player.cpp
  log_player_tool_bar.cpp
  main.cpp
  main_window.cpp
  mark_assignment_editor.cpp
  mark_assignment_painter.cpp
  mark_assignment_table_model.cpp
  monitor_client.cpp
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
  team_graphic_painter.cpp
  trainer_dialog.cpp
  view_config_dialog.cpp
  voronoi_diagram_painter.cpp
)

set_target_properties(soccerwindow2
  PROPERTIES
  AUTOMOC ON
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
  )

target_include_directories(soccerwindow2
  PRIVATE
  ${PROJECT_SOURCE_DIR}
  )

target_link_libraries(soccerwindow2
  PRIVATE
  model
  Qt::Core
  Qt::Gui
  Qt::Widgets
  Qt::Network
  )

if(ZLIB_FOUND)
  target_link_libraries(soccerwindow2 PRIVATE ZLIB::ZLIB)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(soccerwindow2 PRIVATE -W -Wall)
endif()

install(TARGETS soccerwindow2
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
