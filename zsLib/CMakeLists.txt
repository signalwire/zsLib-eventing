cmake_minimum_required(VERSION 3.7.2)

option(ZSLIB_STATIC "Build sw lib as a static library" ON)

# Our core file list
sw_load_sources(zsLib-EventingDeps
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Declare sw lib
if (ZSLIB_STATIC)
	add_library(zsLib-Eventing STATIC ${zsLib-EventingDeps})
	target_compile_definitions(zsLib PUBLIC -DZSLIB_STATIC=1)
else()
	add_library(zsLib-Eventing SHARED ${zsLib-EventingDeps})
endif()

# Setup a useful export file for inclusion in other projects
export(TARGETS zsLib-Eventing
	FILE zsLibEventingConfig.cmake
	EXPORT_LINK_INTERFACE_LIBRARIES
)

if (SW_PLAT_WIN)
	target_compile_definitions(zsLib-Eventing PUBLIC -DWIN32_LEAN_AND_MEAN=1)
endif()

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(zsLib-Eventing PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

if (NOT TARGET cryptopp-static)
	message(FATAL_ERROR "Where is cryptopp?")
endif()

# Link to zslib and cryptopp
target_link_libraries(zsLib-Eventing PUBLIC zsLib cryptopp-static)

# Include the bin dir for config discovery
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

# Setup precompiled header
sw_pch(zsLib-Eventing "eventing.h")
