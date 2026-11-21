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
	m4/ax_boost_base.m4

CLEANFILES = *~
