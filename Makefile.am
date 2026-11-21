## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS=-I m4

SUBDIRS = src

# the application icon in the hicolor icon theme of freedesktop.org
hicolor16dir = $(datadir)/icons/hicolor/16x16/apps
hicolor16_DATA = icons/app/16/soccerwindow2.png
hicolor32dir = $(datadir)/icons/hicolor/32x32/apps
hicolor32_DATA = icons/app/32/soccerwindow2.png
hicolor48dir = $(datadir)/icons/hicolor/48x48/apps
hicolor48_DATA = icons/app/48/soccerwindow2.png
hicolor64dir = $(datadir)/icons/hicolor/64x64/apps
hicolor64_DATA = icons/app/64/soccerwindow2.png
hicolor128dir = $(datadir)/icons/hicolor/128x128/apps
hicolor128_DATA = icons/app/128/soccerwindow2.png
hicolor256dir = $(datadir)/icons/hicolor/256x256/apps
hicolor256_DATA = icons/app/256/soccerwindow2.png
hicolorscalabledir = $(datadir)/icons/hicolor/scalable/apps
hicolorscalable_DATA = icons/app/svg/soccerwindow2.svg

# icons embedded with the Qt resource system (see docs/icons.md)
# icons.am defines the lists of the files (ICONS_SVG_FILES, ICONS_PNG_FILES).
# src/qt/Makefile.am builds them into the program.
ICONS_DIR = icons
include $(srcdir)/icons/icons.am

# tests (run by "make check")
TESTS = test/smoke_help.sh test/check_icons.sh
TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL)
AM_TESTS_ENVIRONMENT = \
	SOCCERWINDOW2='$(abs_top_builddir)/src/qt/soccerwindow2'; export SOCCERWINDOW2; \
	LD_LIBRARY_PATH='$(LIBRCSCLIB)'$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}; export LD_LIBRARY_PATH;

EXTRA_DIST = \
	bootstrap \
	CMakeLists.txt \
	cmake-helper.sh \
	cmake-config.h.in \
	cmake/Dist.cmake \
	cmake/DistCheck.cmake \
	cmake/FindLibrcsc.cmake \
	cmake/Uninstall.cmake \
	RELEASE_NOTES.md \
	test/CMakeLists.txt \
	test/check_icons.sh \
	test/smoke_help.sh \
	test/test_gui_smoke.cpp \
	ChangeLog.qt \
	Doxyfile.in \
	docs/build.md \
	docs/icons.md \
	docs/icons.en.md \
	icons/app/make_soccerwindow2_svg.py \
	icons/generate.sh \
	icons/generated.sha256 \
	icons/icons.am \
	icons/icons.qrc \
	icons/palette.awk \
	icons/palette.txt \
	$(ICONS_SVG_FILES) \
	$(ICONS_PNG_FILES) \
	NEWS.en \
	m4/ax_boost_base.m4 \
	xpm/canvas_enlarge.xpm \
	xpm/canvas_player_number.xpm \
	xpm/canvas_player_type.xpm \
	xpm/chase.xpm \
	xpm/clear.xpm \
	xpm/data_auto_select.xpm \
	xpm/debug_log_view.xpm \
	xpm/debug_server_switch.xpm \
	xpm/debug_view_all.xpm \
	xpm/debug_view_ball.xpm \
	xpm/debug_view_comment.xpm \
	xpm/debug_view_line.xpm \
	xpm/debug_view_message.xpm \
	xpm/debug_view_opponent.xpm \
	xpm/debug_view_player.xpm \
	xpm/debug_view_self.xpm \
	xpm/debug_view_target.xpm \
	xpm/debug_view_teammate.xpm \
	xpm/delete.xpm \
	xpm/fedit2.xpm \
	xpm/hand.xpm \
	xpm/insert.xpm \
	xpm/logplayer_accelerate.xpm \
	xpm/logplayer_decelerate.xpm \
	xpm/logplayer_go_first.xpm \
	xpm/logplayer_go_last.xpm \
	xpm/logplayer_go_next_score.xpm \
	xpm/logplayer_go_prev_score.xpm \
	xpm/logplayer_live_mode.xpm \
	xpm/logplayer_one_step_back.xpm \
	xpm/logplayer_one_step_forward.xpm \
	xpm/logplayer_play_back.xpm \
	xpm/logplayer_play_forward.xpm \
	xpm/logplayer_play_or_stop.xpm \
	xpm/logplayer_shift_down.xpm \
	xpm/logplayer_shift_up.xpm \
	xpm/logplayer_slider.xpm \
	xpm/logplayer_stop.xpm \
	xpm/monitor_connect.xpm \
	xpm/monitor_discard_player.xpm \
	xpm/monitor_disconnect.xpm \
	xpm/monitor_kickoff.xpm \
	xpm/monitor_move_player.xpm \
	xpm/new.xpm \
	xpm/num_1.xpm \
	xpm/num_2.xpm \
	xpm/num_3.xpm \
	xpm/num_4.xpm \
	xpm/num_5.xpm \
	xpm/num_6.xpm \
	xpm/num_7.xpm \
	xpm/num_8.xpm \
	xpm/num_9.xpm \
	xpm/num_10.xpm \
	xpm/num_11.xpm \
	xpm/num_12.xpm \
	xpm/num_13.xpm \
	xpm/num_14.xpm \
	xpm/num_15.xpm \
	xpm/num_16.xpm \
	xpm/num_17.xpm \
	xpm/num_18.xpm \
	xpm/num_19.xpm \
	xpm/num_20.xpm \
	xpm/num_21.xpm \
	xpm/num_22.xpm \
	xpm/num_23.xpm \
	xpm/num_24.xpm \
	xpm/num_25.xpm \
	xpm/num_26.xpm \
	xpm/num_27.xpm \
	xpm/num_28.xpm \
	xpm/num_29.xpm \
	xpm/num_30.xpm \
	xpm/num_31.xpm \
	xpm/num_32.xpm \
	xpm/open.xpm \
	xpm/open_dir.xpm \
	xpm/open_rcg.xpm \
	xpm/record.xpm \
	xpm/refresh.xpm \
	xpm/replace.xpm \
	xpm/reverse.xpm \
	xpm/save.xpm \
	xpm/soccerwindow2.xpm \
	xpm/soccerwindow2-nostr.xpm \
	xpm/sync.xpm \
	xpm/sync-ball.xpm \
	xpm/symmetry.xpm \
	xpm/train.xpm

CLEANFILES = *~
