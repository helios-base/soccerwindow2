add_subdirectory(model)
add_subdirectory(qt)

configure_file(sswindow2.in sswindow2 @ONLY)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/sswindow2
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
