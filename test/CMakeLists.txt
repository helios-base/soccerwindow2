# Smoke test: start soccerwindow2 with --help on the offscreen platform.
add_test(NAME soccerwindow2_help
  COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/smoke_help.sh $<TARGET_FILE:soccerwindow2>
  )
set_tests_properties(soccerwindow2_help
  PROPERTIES
  TIMEOUT 60
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
