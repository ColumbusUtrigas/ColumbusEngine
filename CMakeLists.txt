cmake_minimum_required(VERSION 3.8)
project(ColumbusEngine)

function(group_files_by_directory)
	cmake_parse_arguments(GROUP_FILES "" "" "SRCS" ${ARGN})

	foreach(FILE ${GROUP_FILES_SRCS})
		# Get the directory of the source file
		get_filename_component(PARENT_DIR ${FILE} DIRECTORY)

		# Remove common directory prefix to make the group
		string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" GROUP ${PARENT_DIR})

		# Make sure we are using windows slashes
		string(REPLACE "/" "\\" GROUP ${GROUP})

		# Group into "Source Files" and "Header Files"
		#    if ("${FILE}" MATCHES ".*\\.cpp")
		#       set(GROUP "Source Files${GROUP}")
		#    elseif("${FILE}" MATCHES ".*\\.h")
		#       set(GROUP "Header Files${GROUP}")
		#    endif()

		source_group(${GROUP} FILES ${FILE})
	endforeach()
endfunction()

function(dir_to_var PATH SOURCES)
	file(GLOB TMP_SRC ${PATH}/*.cpp ${PATH}/*.c ${PATH}/*.hpp ${PATH}/*.h)
	group_files_by_directory(SRCS ${TMP_SRC})
	set(${SOURCES} ${TMP_SRC} PARENT_SCOPE)
endfunction()

function(dir_to_var_recurse PATH SOURCES)
	file(GLOB_RECURSE TMP_SRC ${PATH}/*.cpp ${PATH}/*.c ${PATH}/*.hpp ${PATH}/*.h)
	group_files_by_directory(SRCS ${TMP_SRC})
	set(${SOURCES} ${TMP_SRC} PARENT_SCOPE)
endfunction()

function(add_dir_to_target TARGET PATH)
	file(GLOB TMP_SRC ${PATH}/*.cpp ${PATH}/*.c ${PATH}/*.hpp ${PATH}/*.h)
	target_sources(${TARGET} PUBLIC ${TMP_SRC})
	group_files_by_directory(SRCS ${TMP_SRC})
endfunction()

function(add_dir_to_target_recurse TARGET PATH)
	file(GLOB_RECURSE TMP_SRC ${PATH}/*.cpp ${PATH}/*.c ${PATH}/*.hpp ${PATH}/*.h)
	target_sources(${TARGET} PUBLIC ${TMP_SRC})
	group_files_by_directory(SRCS ${TMP_SRC})
endfunction()

set(BINARIES_DIR ${PROJECT_SOURCE_DIR}/Binaries)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARIES_DIR}) # static libraries output
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARIES_DIR}) # dynamic libraries output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARIES_DIR}) # executables output

####################################################################

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(TARGET ColumbusEngine)

########## IMGUI OPENGL3 SDL2 DX12 ##########
file(GLOB IMGUI_SRC
	Lib/imgui/imgui.h
	./Lib/imgui/*.cpp
	./Lib/imgui/misc/cpp/*.cpp
	./Lib/imgui/examples/imgui_impl_sdl.cpp
	./Lib/imgui/examples/imgui_impl_win32.cpp
	./Lib/imgui/examples/imgui_impl_opengl3.cpp
	./Lib/imgui/examples/imgui_impl_dx12.cpp
	./Lib/ImGuizmo/*.cpp)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ./Lib/imgui)
if(LINUX)
	target_include_directories(imgui PUBLIC /usr/include/SDL2/)
elseif(WIN32)
	target_include_directories(imgui PUBLIC ./Lib/GLEW/include)
	target_include_directories(imgui PUBLIC ./Lib/SDL2/include)
endif()

########## BULLET PHYSICS #############
dir_to_var_recurse(Lib/bullet3/src/BulletCollision BULLET_COLLISION_SRC)
dir_to_var_recurse(Lib/bullet3/src/BulletDynamics BULLET_DYNAMICS_SRC)
dir_to_var_recurse(Lib/bullet3/src/LinearMath BULLET_MATH_SRC)
set(BULLET_SRC ${BULLET_COLLISION_SRC} ${BULLET_DYNAMICS_SRC} ${BULLET_MATH_SRC})
add_library(bullet STATIC ${BULLET_SRC})
target_include_directories(bullet PRIVATE Lib/bullet3/src)
set(BULLET_LIB bullet)

#set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "Bullet build 2 demos" FORCE)
#set(BUILD_BULLET3       OFF CACHE BOOL "Bullet build 3" FORCE)
#set(BUILD_CLSOCKET      OFF CACHE BOOL "Bullet build cl socket" FORCE)
#set(BUILD_CPU_DEMOS     OFF CACHE BOOL "Bullet build cpu demos" FORCE)
#set(BUILD_ENET          OFF CACHE BOOL "Bullet build  enet" FORCE)
#set(BUILD_EXTRAS        OFF CACHE BOOL "Bullet build extras" FORCE)
#set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "Bullet build opengl3 demos" FORCE)
#set(BUILD_UNIT_TESTS    OFF CACHE BOOL "Bullet build unit tests" FORCE)

#set(USE_DOUBLE_PRECISION                    OFF CACHE BOOL "Bullet use double precision" FORCE)
#set(USE_GRAPHICAL_BENCHMARK                 OFF CACHE BOOL "Bullet use Graphical Benchmark" FORCE)
#set(BUILD_SHARED_LIBS                       OFF CACHE BOOL "Bullet use shared libraries" FORCE)
#set(USE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD ON  CACHE BOOL "Bullet use btSoftMultiBodyDynamicsWorld" FORCE)
#set(BULLET2_MULTITHREADING                  ON  CACHE BOOL "Bullet 2 multithread" FORCE)
#set(USE_GLUT                                OFF CACHE BOOL "Bullet use glut" FORCE)
#set(INSTALL_CMAKE_FILES                     OFF CACHE BOOL "Bullet install cmake files" FORCE)
#set(USE_MSVC_RUNTIME_LIBRARY_DLL            OFF CACHE BOOL "Bullet msvc runtime library dll" FORCE)

#set(CMAKE_DEBUG_POSTFIX          "" CACHE STRING "Bullet Adds a postfix for debug-built libraries.")
#set(CMAKE_MINSIZEREL_POSTFIX     "" CACHE STRING "Bullet Adds a postfix for MinsizeRelease-built libraries.")
#set(CMAKE_RELWITHDEBINFO_POSTFIX "" CACHE STRING "Bullet Adds a postfix for ReleaseWithDebug-built libraries.")

#add_subdirectory(Lib/bullet3)
#set_target_properties(BulletCollision PROPERTIES FOLDER Bullet)
#set_target_properties(BulletDynamics  PROPERTIES FOLDER Bullet)
#set_target_properties(LinearMath      PROPERTIES FOLDER Bullet)
#set_target_properties(Bullet3Common      PROPERTIES FOLDER Bullet)
#set_target_properties(BulletInverseDynamics      PROPERTIES FOLDER Bullet)
#set_target_properties(BulletSoftBody      PROPERTIES FOLDER Bullet)

########## COLUMBUS ENGINE ############
add_executable(${TARGET} main.cpp)
set_target_properties(${TARGET} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET})
set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET}>"
                                           VS_DEBUGGER_COMMAND           "$<TARGET_FILE:${TARGET}>")

target_include_directories(${TARGET} PUBLIC ./)
target_include_directories(${TARGET} PUBLIC ./Lib/)
target_include_directories(${TARGET} PUBLIC ./Lib/json/single_include/nlohmann/)
target_include_directories(${TARGET} PUBLIC ./Lib/MiniMP3)
target_include_directories(${TARGET} PUBLIC SYSTEM ./Lib/STB/)
target_include_directories(${TARGET} PUBLIC ./Lib/ZSTD/Include/)

if(LINUX)
	target_include_directories(${TARGET} PUBLIC /usr/include/SDL2/)
	target_include_directories(${TARGET} PUBLIC /usr/include/bullet/)
elseif(WIN32)
	target_include_directories(${TARGET} PUBLIC ./Lib/SDL2/include)
	target_include_directories(${TARGET} PUBLIC ./Lib/bullet3/src)
	target_include_directories(${TARGET} PUBLIC ./Lib/GLEW/include)
	target_include_directories(${TARGET} PUBLIC ./Lib/LibTIFF/Include)
	target_include_directories(${TARGET} PUBLIC ./Lib/GLM)
	target_include_directories(${TARGET} PUBLIC ./Lib/WinPixEventRuntime/Include)

	target_include_directories(${TARGET} PUBLIC $ENV{VULKAN_SDK}/Include)
endif()

add_dir_to_target(${TARGET} Audio)
add_dir_to_target_recurse(${TARGET} Common)
add_dir_to_target(${TARGET} Core)
if(LINUX)
	add_dir_to_target(${TARGET} Core/Linux)
elseif(WIN32)
	add_dir_to_target(${TARGET} Core/Windows)
	add_dir_to_target(${TARGET} Graphics/DX12)
	add_dir_to_target(${TARGET} Graphics/Vulkan)
endif()
add_dir_to_target_recurse(${TARGET} Editor)
add_dir_to_target_recurse(${TARGET} Game)

#add_dir_to_target_recurse(${TARGET} Graphics) #OpenGL is included
add_dir_to_target(${TARGET} Graphics)
add_dir_to_target(${TARGET} Graphics/OpenGL)
add_dir_to_target_recurse(${TARGET} Graphics/Particles)
add_dir_to_target(${TARGET} Graphics/Postprocess)

add_dir_to_target(${TARGET} Input)
add_dir_to_target(${TARGET} Physics)
add_dir_to_target(${TARGET} Profiling)
add_dir_to_target(${TARGET} RenderAPIOpenGL)
add_dir_to_target(${TARGET} Resources)
add_dir_to_target(${TARGET} Scene)
add_dir_to_target(${TARGET} System)

#file(GLOB builtin_shaders_files Graphics/Shaders/*.csl)
#add_custom_target(builtin_shaders ${builtin_shaders_files})
#add_dependencies(${TARGET} builtin_shaders)

if (LINUX)
	target_link_libraries(${TARGET} SDL2 SDL2main GL GLEW png tiff jpeg
		LinearMath BulletCollision BulletDynamics pthread zstd vulkan)
elseif(WIN32)
	target_link_directories(${TARGET} PUBLIC
		./Lib/GLEW/lib/Release/x64
		./Lib/SDL2/lib/x64
		./Lib/ZSTD/lib
		./Lib/LibTIFF/lib/x64
		./Lib/WinPixEVentRuntime/bin/x64
		$ENV{VULKAN_SDK}/Lib
	)
	target_link_libraries(${TARGET} opengl32 SDL2 glew32 zstd tiff ${BULLET_LIB} vulkan-1 WinPixEventRuntime)
endif()

target_link_libraries(${TARGET} imgui)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Data" "$<TARGET_FILE_DIR:${TARGET}>/Data")

if (WIN32)
	# copy required DLLs to build
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/RequiredForBuild/" "$<TARGET_FILE_DIR:${TARGET}>")
endif()

