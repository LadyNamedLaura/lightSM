project(lightSM)

set(VERSION "0.1")
include(GNUInstallDirs)
OPTION(GNOME "Build and install gnome-specific code" OFF)
OPTION(MISC_UNIT "Install misc systemd units" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall -pedantic --std=c99 -O3 -flto -fuse-linker-plugin")

if(GNOME)
	ADD_SUBDIRECTORY(gnome)
endif(GNOME)
ADD_SUBDIRECTORY(x11)
ADD_SUBDIRECTORY(daemon)
ADD_SUBDIRECTORY(unit)
