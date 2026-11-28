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

# the desktop entry, with which the desktop environments can find and start
# soccerwindow2. The directory of the executable is embedded in Exec.
applicationsdir = $(datadir)/applications
applications_DATA = soccerwindow2.desktop
soccerwindow2.desktop: $(srcdir)/soccerwindow2.desktop.in Makefile
	sed -e 's|@SOCCERWINDOW2_BINDIR@|$(bindir)|g' $(srcdir)/soccerwindow2.desktop.in > $@

# icons embedded with the Qt resource system (see docs/icons.md)
# icons.am defines the lists of the files (ICONS_SVG_FILES, ICONS_PNG_FILES).
# src/qt/Makefile.am builds them into the program.
ICONS_DIR = icons
include $(srcdir)/icons/icons.am

# tests (run by "make check")
TESTS = test/smoke_help.sh test/smoke_export.sh test/check_icons.sh test/check_desktop_entry.sh
TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL)
if USE_RPATH
# The library path of librcsc is embedded in the executable (RPATH), so it is
# not added to LD_LIBRARY_PATH: the smoke test checks that soccerwindow2
# starts without it.
TESTS_LIBRARY_PATH =
else
TESTS_LIBRARY_PATH = LD_LIBRARY_PATH='$(LIBRCSCLIB)'$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}; export LD_LIBRARY_PATH;
endif
AM_TESTS_ENVIRONMENT = \
	SOCCERWINDOW2='$(abs_top_builddir)/src/qt/soccerwindow2'; export SOCCERWINDOW2; \
	$(TESTS_LIBRARY_PATH)

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
	soccerwindow2.desktop.in \
	test/CMakeLists.txt \
	test/check_desktop_entry.sh \
	test/check_icons.sh \
	test/check_websocket.sh \
	test/data/head_json.rcg \
	test/data/head_v5.rcg \
	test/data/head_v6.rcg \
	test/smoke_export.sh \
	test/smoke_help.sh \
	test/test_connect_dialog.cpp \
	test/test_environment.h \
	test/test_export_batch.cpp \
	test/test_export_dialog.cpp \
	test/test_export_range.cpp \
	test/test_export_range_list.cpp \
	test/test_export_timing.cpp \
	test/test_ffmpeg_sink.cpp \
	test/test_field_canvas_render.cpp \
	test/test_gui_smoke.cpp \
	test/test_image_sequence_sink.cpp \
	test/test_log_player_slider.cpp \
	test/test_message_splitter.cpp \
	test/test_monitor_client.cpp \
	test/test_monitor_client_live.cpp \
	test/test_monitor_transport.cpp \
	test/test_rcg_loading.cpp \
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
	NEWS.en

CLEANFILES = *~ soccerwindow2.desktop
