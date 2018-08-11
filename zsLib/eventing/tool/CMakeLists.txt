cmake_minimum_required(VERSION 3.7.2)

# Our core file list
sw_load_sources(zsLibEventingToolCompilerDeps
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Declare our tool
add_executable(zsLib-Eventing.Tool.Compiler ${zsLibEventingToolCompilerDeps})

# Link to zslib-eventing
target_link_libraries(zsLib-Eventing.Tool.Compiler PRIVATE zsLib-Eventing)

# Include the bin dir for config discovery
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

# Setup precompiled header
sw_pch(zsLib-Eventing.Tool.Compiler "testing.h")
