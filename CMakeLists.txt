cmake_minimum_required(VERSION 3.8)
project(ColumbusEngine)

function(add_dir_to_target TARGET PATH)
	file(GLOB TMP_SRC ${PATH}/*.cpp ${PATH}/*.c)
	target_sources(${TARGET} PUBLIC ${TMP_SRC})
endfunction()

function(add_dir_to_target_recurse TARGET PATH)
	file(GLOB_RECURSE TMP_SRC ${PATH}/*.cpp ${PATH}/*.c)
	target_sources(${TARGET} PUBLIC ${TMP_SRC})
endfunction()

####################################################################

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(TARGET ColumbusEngine)

########## IMGUI OPENGL3 SDL2 ##########
file(GLOB IMGUI_SRC
	./Lib/imgui/*.cpp
	./Lib/imgui/misc/cpp/*.cpp
	./Lib/imgui/examples/imgui_impl_sdl.cpp
	./Lib/imgui/examples/imgui_impl_win32.cpp
	./Lib/imgui/examples/imgui_impl_opengl3.cpp
	./Lib/ImGuizmo/*.cpp)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ./Lib/imgui)
if(LINUX)
	target_include_directories(imgui PUBLIC /usr/include/SDL2/)
elseif(WIN32)
	target_include_directories(imgui PUBLIC ./Lib/GLEW/include)
	target_include_directories(imgui PUBLIC ./Lib/SDL2/include)
endif()

########################################

add_executable(${TARGET} main.cpp)
set_target_properties(${TARGET} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

target_include_directories(${TARGET} PUBLIC ./)
target_include_directories(${TARGET} PUBLIC ./Lib/)
target_include_directories(${TARGET} PUBLIC ./Lib/MiniMP3)
target_include_directories(${TARGET} PUBLIC SYSTEM ./Lib/STB/)
target_include_directories(${TARGET} PUBLIC ./Lib/ZSTD/Include/)

if(LINUX)
	target_include_directories(${TARGET} PUBLIC /usr/include/SDL2/)
	target_include_directories(${TARGET} PUBLIC /usr/include/bullet/)
elseif(WIN32)
	target_include_directories(${TARGET} PUBLIC ./Lib/SDL2/include)
	target_include_directories(${TARGET} PUBLIC ./Lib/Bullet/include)
	target_include_directories(${TARGET} PUBLIC ./Lib/GLEW/include)
	target_include_directories(${TARGET} PUBLIC ./Lib/LibTIFF/Include)
	target_include_directories(${TARGET} PUBLIC ./Lib/GLM)
endif()

add_dir_to_target(${TARGET} Audio)
add_dir_to_target_recurse(${TARGET} Common)
add_dir_to_target(${TARGET} Core)
if(LINUX)
	add_dir_to_target(${TARGET} Core/Linux)
elseif(WIN32)
	add_dir_to_target(${TARGET} Core/Windows)
endif()
add_dir_to_target_recurse(${TARGET} Editor)

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

file(GLOB builtin_shaders_files Graphics/Shaders/*.csl)
add_custom_target(builtin_shaders DEPENDS ${builtin_shaders_files})
add_dependencies(${TARGET} builtin_shaders)

if (LINUX)
	target_link_libraries(${TARGET} SDL2 SDL2main GL GLEW png tiff jpeg
		LinearMath BulletCollision BulletDynamics pthread zstd vulkan)
elseif(WIN32)
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		set(BULLET_LIB_PATH ./Lib/Bullet/lib/Debug)
	else()
		set(BULLET_LIB_PATH ./Lib/Bullet/lib/Release)
	endif()

	target_link_directories(${TARGET} PUBLIC
		${BULLET_LIB_PATH}
		./Lib/GLEW/lib/Release/x64
		./Lib/SDL2/lib/x64
		./Lib/ZSTD/lib
		./Lib/LibTIFF/lib/x64
	)
	target_link_libraries(${TARGET} opengl32 SDL2 glew32 glew32 zstd tiff
		BulletLinearMath BulletCollision BulletDynamics
	)
endif()

target_link_libraries(${TARGET} imgui)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Data" "${CMAKE_CURRENT_BINARY_DIR}/Data")

