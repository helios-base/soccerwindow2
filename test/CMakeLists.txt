# Smoke test: start soccerwindow2 with --help on the offscreen platform.
add_test(NAME soccerwindow2_help
  COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/smoke_help.sh $<TARGET_FILE:soccerwindow2>
  )
set_tests_properties(soccerwindow2_help
  PROPERTIES
  TIMEOUT 60
  )

# Check the icon files (PNG files, lists, colors). Inkscape is not needed.
add_test(NAME check_icons
  COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/check_icons.sh ${PROJECT_SOURCE_DIR}/icons
  )

# Check the desktop entry (soccerwindow2.desktop.in). It is skipped when
# desktop-file-validate (desktop-file-utils) is not found.
add_test(NAME check_desktop_entry
  COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/check_desktop_entry.sh ${PROJECT_SOURCE_DIR}
  )
set_tests_properties(check_desktop_entry
  PROPERTIES
  SKIP_RETURN_CODE 77
  )

# GUI smoke test with Qt Test
find_package(Qt${QT_VERSION_MAJOR} ${SOCCERWINDOW2_QT_MIN_VERSION} REQUIRED COMPONENTS Test)

add_executable(test_gui_smoke
  test_gui_smoke.cpp
  )
set_target_properties(test_gui_smoke
  PROPERTIES
  AUTOMOC ON
  )
target_link_libraries(test_gui_smoke
  PRIVATE
  soccerwindow2_gui
  Qt::Test
  )
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(test_gui_smoke PRIVATE -W -Wall)
endif()

add_test(NAME gui_smoke
  COMMAND test_gui_smoke
  )
set_tests_properties(gui_smoke
  PROPERTIES
  TIMEOUT 120
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
  )

# Qt Test programs of the GUI library: test_NAME.cpp is built as test_NAME
# and registered as the test NAME.
function(soccerwindow2_add_qt_test name)
  add_executable(test_${name}
    test_${name}.cpp
    )
  set_target_properties(test_${name}
    PROPERTIES
    AUTOMOC ON
    )
  target_link_libraries(test_${name}
    PRIVATE
    soccerwindow2_gui
    Qt::Test
    )
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_${name} PRIVATE -W -Wall)
  endif()
  add_test(NAME ${name}
    COMMAND test_${name}
    )
endfunction()

# the transports of the monitor client and MonitorClient, with local fake
# servers, and the "Connect to ..." dialog
soccerwindow2_add_qt_test(message_splitter)
soccerwindow2_add_qt_test(monitor_transport)
soccerwindow2_add_qt_test(monitor_client)
soccerwindow2_add_qt_test(connect_dialog)
set_tests_properties(message_splitter monitor_transport monitor_client connect_dialog
  PROPERTIES
  TIMEOUT 60
  )

# MainData::openRCG() with the game logs in test/data (text v5/v6 and JSON,
# uncompressed and gzip)
soccerwindow2_add_qt_test(rcg_loading)
target_compile_definitions(test_rcg_loading
  PRIVATE
  SOCCERWINDOW2_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
  )
set_tests_properties(rcg_loading
  PROPERTIES
  TIMEOUT 60
  )

# FieldCanvas::renderFrame(), FrameScope and File > Save Image of This Frame
# (docs/plan/2026-11-28-00_frame-export.md 4.1), with test/data/head_v5.rcg
soccerwindow2_add_qt_test(field_canvas_render)
target_compile_definitions(test_field_canvas_render
  PRIVATE
  SOCCERWINDOW2_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
  )
set_tests_properties(field_canvas_render
  PROPERTIES
  TIMEOUT 60
  )

# MonitorClient with a real rcssserver. It runs only when the environment
# variable RCSSSERVER is set to the path of rcssserver, and is skipped
# (exit status 77) otherwise, e.g. in CI:
#   RCSSSERVER=/path/to/rcssserver ctest -R monitor_client_live --output-on-failure
soccerwindow2_add_qt_test(monitor_client_live)
set_tests_properties(monitor_client_live
  PROPERTIES
  TIMEOUT 120
  SKIP_RETURN_CODE 77
  )
