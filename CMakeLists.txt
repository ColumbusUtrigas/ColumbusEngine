cmake_minimum_required(VERSION 3.8)
project(ColumbusEngine)

# TODO: engine as a library, game/app/editor as a target

# TODO: project-related stuff here
if (NOT DEFINED BINARIES_DIR)
    set(BINARIES_DIR ${PROJECT_SOURCE_DIR}/Binaries)
    set(SYMBOLS_DIR ${PROJECT_SOURCE_DIR}/Binaries)
    set(LIBS_DIR ${PROJECT_SOURCE_DIR}/Binaries/Libs)

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBS_DIR}) # static libraries output
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARIES_DIR}) # dynamic libraries output
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARIES_DIR}) # executables output
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${SYMBOLS_DIR}) # symbols output
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${SYMBOLS_DIR}) # symbols output

    message("Setting new BINARIES_DIR, it is ${BINARIES_DIR}")
    message("Setting new SYMBOLS_DIR, it is ${SYMBOLS_DIR}")
    message("Setting new LIBS_DIR, it is ${LIBS_DIR}")
else()
    message("Grabbing BINARIES_DIR from the upper project, it is ${BINARIES_DIR}")
    message("Grabbing SYMBOLS_DIR from the upper project, it is ${SYMBOLS_DIR}")
    message("Grabbing LIBS_DIR from the upper project, it is ${LIBS_DIR}")
endif()

message("Some debug info for CMake")
message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_ARCHIVE_OUTPUT_DIRECTORY = ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

include(EngineLib.cmake)

if (WIN32)
	# copy required DLLs to build
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/RequiredForBuild/" "${BINARIES_DIR}/$<CONFIG>")
endif()
