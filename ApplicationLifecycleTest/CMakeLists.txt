cmake_minimum_required(VERSION 3.7)
cmake_policy(SET CMP0015 NEW)

project(lifecycle)

include_directories(../../glesgae)
link_directories(../../glesgae/lib)

set(CMAKE_C_FLAGS "-Wall -Werror -Wpedantic -Wno-variadic-macros -Wcast-align -Wredundant-decls -Wextra -std=gnu89 -pedantic-errors")

# Compile Options
option(USE_SDL2 "Use SDL2 Bindings" OFF)
option(USE_OGL "Use OpenGL Bindings" OFF)
option(USE_SDL2GL "Use SDL2 with platform GL" OFF)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

if (UNIX)
	add_definitions(-DLINUX)
endif (UNIX)

# SDL2 specifics
if (USE_SDL2)
	add_definitions(-DSDL2)
	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIRS})
endif (USE_SDL2)

# OGL specifics
if (USE_OGL)
	remove_definitions(-DSDL2)
	if (UNIX)
		add_definitions(-DGLX)
	elseif (WIN32)
		add_definitions(-DWGL)
	elseif (APPLE)
		add_definitions(-DAGL)
	endif (UNIX)
endif (USE_OGL)

# SDL2 with GL Specifics
if (USE_SDL2GL)
endif (USE_SDL2GL)

add_executable(lifecycle main.c)

if (USE_SDL2)
	target_link_libraries(lifecycle glesgae ${SDL2_LIBRARIES})
endif (USE_SDL2)

if (USE_OGL)
	if (UNIX)
		target_link_libraries(lifecycle glesgae m X11 GL)
	endif (UNIX)
endif (USE_OGL)

