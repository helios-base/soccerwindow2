## Process this file with automake to produce Makefile.in

if BUILD_QT
BUILD_PROGS = soccerwindow2
endif

bin_PROGRAMS = $(BUILD_PROGS)

soccerwindow2_SOURCES = \
	action_sequence_option_window.cpp \
	action_sequence_selector.cpp \
	action_sequence_tree_view.cpp \
	ball_painter.cpp \
	ball_painter_rcss.cpp \
	ball_trace_painter.cpp \
	color_setting_dialog.cpp \
	coordinate_delegate.cpp \
	debug_log_dir_dialog.cpp \
	debug_log_painter.cpp \
	debug_message_window.cpp \
	debug_painter.cpp \
	debug_server.cpp \
	detail_dialog.cpp \
	dir_selector.cpp \
	draw_config.cpp \
	draw_data_painter.cpp \
	features_log_painter.cpp \
	field_canvas.cpp \
	field_painter.cpp \
	font_setting_dialog.cpp \
	formation_data_view.cpp \
	formation_editor_painter.cpp \
	formation_editor_window.cpp \
	image_save_dialog.cpp \
	label_editor_window.cpp \
	launcher_dialog.cpp \
	log_player.cpp \
	log_player_tool_bar.cpp \
	main.cpp \
	main_window.cpp \
	monitor_client.cpp \
	offside_line_painter.cpp \
	player_control_painter.cpp \
	player_painter.cpp \
	player_painter_rcss.cpp \
	player_trace_painter.cpp \
	player_type_dialog.cpp \
	score_board_painter.cpp \
	score_board_painter_rcss.cpp \
	simple_label_selector.cpp \
	shortcut_keys_dialog.cpp \
	team_graphic_painter.cpp \
	trainer_dialog.cpp \
	view_config_dialog.cpp \
	voronoi_diagram_painter.cpp

nodist_soccerwindow2_SOURCES = \
	moc_action_sequence_option_window.cpp \
	moc_action_sequence_selector.cpp \
	moc_action_sequence_tree_view.cpp \
	moc_color_setting_dialog.cpp \
	moc_coordinate_delegate.cpp \
	moc_debug_log_dir_dialog.cpp \
	moc_debug_message_window.cpp \
	moc_debug_server.cpp \
	moc_detail_dialog.cpp \
	moc_dir_selector.cpp \
	moc_field_canvas.cpp \
	moc_font_setting_dialog.cpp \
	moc_formation_data_view.cpp \
	moc_formation_editor_window.cpp \
	moc_image_save_dialog.cpp \
	moc_label_editor_window.cpp \
	moc_launcher_dialog.cpp \
	moc_log_player.cpp \
	moc_log_player_tool_bar.cpp \
	moc_main_window.cpp \
	moc_monitor_client.cpp \
	moc_player_type_dialog.cpp \
	moc_shortcut_keys_dialog.cpp \
	moc_simple_label_selector.cpp \
	moc_trainer_dialog.cpp \
	moc_view_config_dialog.cpp

noinst_HEADERS = \
	action_sequence_option_window.h \
	action_sequence_selector.h \
	action_sequence_tree_view.h \
	ball_painter.h \
	ball_painter_rcss.h \
	ball_trace_painter.h \
	color_setting_dialog.h \
	coordinate_delegate.h \
	debug_log_dir_dialog.h \
	debug_log_painter.h \
	debug_message_window.h \
	debug_painter.h \
	debug_server.h \
	detail_dialog.h \
	dir_selector.h \
	draw_config.h \
	draw_data_painter.h \
	features_log_painter.h \
	field_canvas.h \
	field_painter.h \
	font_setting_dialog.h \
	formation_data_view.h \
	formation_editor_painter.h \
	formation_editor_window.h \
	image_save_dialog.h \
	label_editor_window.h \
	launcher_dialog.h \
	log_player.h \
	log_player_tool_bar.h \
	main_window.h \
	monitor_client.h \
	mouse_state.h \
	offside_line_painter.h \
	painter_interface.h \
	player_control_painter.h \
	player_painter.h \
	player_painter_rcss.h \
	player_trace_painter.h \
	player_type_dialog.h \
	score_board_painter.h \
	score_board_painter_rcss.h \
	shortcut_keys_dialog.h \
	simple_label_selector.h \
	team_graphic_painter.h \
	trainer_dialog.h \
	view_config_dialog.h \
	voronoi_diagram_painter.h


soccerwindow2_CPPFLAGS = -I$(top_srcdir)/src/model $(QT_CPPFLAGS)
soccerwindow2_CXXFLAGS = $(QT_CXXFLAGS) -Wall -W -Wno-deprecated-copy
soccerwindow2_LDFLAGS = $(QT_LDFLAGS)
soccerwindow2_LDADD = $(top_builddir)/src/model/libsoccerwindow2_model.a $(QT_LIBS)
#soccerwindow2_LIBADD = ../model/libsoccerwindow2_model.a

# source files from headers generated by Meta Object Compiler
moc_%.cpp: %.h
	$(QT_MOC) $< -o $@

# rule to generate tranlation files
##%.qm: %.ts
##	$(QT_LRELEASE) $< -qm $@

AM_CPPFLAGS =
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

EXTRA_DIST = \
	qt.pro \
	soccerwindow2.rc \
	soccerwindow2.ico

CLEANFILES = $(nodist_soccerwindow2_SOURCES) *~
