# Smoke test: start soccerwindow2 with --help on the offscreen platform.
add_test(NAME soccerwindow2_help
  COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/smoke_help.sh $<TARGET_FILE:soccerwindow2>
  )
set_tests_properties(soccerwindow2_help
  PROPERTIES
  TIMEOUT 60
  )
