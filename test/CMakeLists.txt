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
