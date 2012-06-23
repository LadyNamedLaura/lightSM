
find_program(SYSTEMD_PATH systemd)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/daemon.h.in
		${CMAKE_CURRENT_BINARY_DIR}/daemon.h @ONLY)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lightSM main.c user.c server.c opt.c systemd.c)
target_link_libraries(lightSM pam pam_misc)

install(TARGETS lightSM DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES pamfile DESTINATION /etc/pam.d RENAME lightsm)
