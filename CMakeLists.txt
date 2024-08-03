cmake_minimum_required(VERSION 3.8)
project(ColumbusEngine)

# TODO: engine as a library, game/app/editor as a target

# TODO: project-related stuff here
if (NOT DEFINED BINARIES_DIR)
    set(BINARIES_DIR ${PROJECT_SOURCE_DIR}/Binaries)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARIES_DIR}) # static libraries output
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARIES_DIR}) # dynamic libraries output
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARIES_DIR}) # executables output

    message("Setting new BINARIES_DIR, it is ${BINARIES_DIR}")
else()
    message("Grabbing BINARIES_DIR from the upper project, it is ${BINARIES_DIR}")
endif()

message("Some debug info for CMake")
message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_ARCHIVE_OUTPUT_DIRECTORY = ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

include(EngineLib.cmake)

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Data" "$<TARGET_FILE_DIR:${TARGET}>/Data")
# add_custom_command(TARGET ${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:${TARGET}>/Data/Shaders")

########## TEST APP ############
add_executable(TestApp Core/Windows/WindowsMain2.cpp)
target_link_libraries(TestApp ColumbusEngine)
target_include_engine_directories(TestApp)

set_target_properties(TestApp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# target_compile_options(TestApp PRIVATE -fsanitize=address)
# target_link_options(TestApp PRIVATE -fsanitize=address)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TestApp)
set_target_properties(TestApp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:TestApp>"
                                         VS_DEBUGGER_COMMAND           "$<TARGET_FILE:TestApp>")
################################

ADD_CUSTOM_TARGET(do_always1 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Data" "$<TARGET_FILE_DIR:${TARGET}>/Data")
ADD_CUSTOM_TARGET(do_always2 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:${TARGET}>/Data/Shaders")

if (WIN32)
	# copy required DLLs to build
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/RequiredForBuild/" "$<TARGET_FILE_DIR:${TARGET}>")
endif()
