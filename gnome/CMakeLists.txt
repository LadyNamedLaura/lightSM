find_package(DBUS REQUIRED)

include_directories(${DBUS_INCLUDE_DIRS})
link_directories(${DBUS_LIBRARY_DIRS})

add_executable(lightSM-gnome main.c)
target_link_libraries(lightSM-gnome ${DBUS_LIBRARIES})

install(TARGETS lightSM-gnome COMPONENT gnome DESTINATION libexec)