cmake_minimum_required(VERSION 2.8)
project(CLMBSEngine)

file(GLOB SRC_AUDIO ./Audio/*.cpp)
file(GLOB SRC_COMMON_IMAGE ./Common/Image/*.cpp ./Common/Image/*/*.cpp)
file(GLOB SRC_COMMON_SOUND ./Common/Sound/*.cpp ./Common/Sound/*/*.cpp)

file(GLOB SRC_COMMON ${SRC_COMMON_IMAGE} ./Common/Model/*.cpp ./Common/Model/*/*.cpp ${SRC_COMMON_SOUND} ./Common/*/*.cpp)

file(GLOB SRC_CORE ./Core/*.cpp ./Core/Linux/*.cpp ./Core/Platform/*.cpp)
file(GLOB SRC_GRAPHICS ./Graphics/*.cpp ./Graphics/OpenGL/*.cpp ./Graphics/Particles/*.cpp ./Graphics/Particles/*/*.cpp)
file(GLOB SRC_INPUT ./Input/*.cpp)
file(GLOB SRC_PHYSICS ./Physics/*.cpp)
file(GLOB SRC_RENDER_API ./RenderAPI/*.cpp)
file(GLOB SRC_RENDER_API_OPENGL ./RenderAPIOpenGL/*.cpp)
file(GLOB SRC_SCENE ./Scene/*.cpp)
file(GLOB SRC_SYSTEM ./System/*.cpp)
file(GLOB SRC_EDITOR ./Editor/*.cpp ./Editor/*/*.cpp)
file(GLOB SRC_PROFILING ./Profiling/*.cpp)

file(GLOB IMGUI ./Lib/imgui/*.cpp ./Lib/imgui/misc/cpp/*.cpp ./Lib/imgui/examples/imgui_impl_sdl.cpp ./Lib/imgui/examples/imgui_impl_opengl3.cpp)

set(SRC ${SRC_AUDIO} ${SRC_COMMON} ${SRC_CORE} ${SRC_GRAPHICS} ${SRC_INPUT} ${SRC_PHYSICS} ${SRC_RENDER_API} ${SRC_RENDER_API_OPENGL} ${SRC_SCENE} ${SRC_SYSTEM} ${IMGUI} ${SRC_EDITOR} ${SRC_PROFILING} main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -Wall -Wextra -Wno-unused-parameter")

set(TARGET ColumbusEngine)

include_directories(./)
include_directories(./Lib/MiniMP3)
include_directories(SYSTEM ./Lib/STB/)
include_directories(./Lib/ZSTD/Include/)
include_directories(/usr/include/SDL2/)
include_directories(/usr/include/bullet/)

add_executable(${TARGET} ${SRC})

target_link_libraries(${TARGET} SDL2 SDL2main GL GLEW tinyxml2 png tiff jpeg LinearMath BulletCollision BulletDynamics pthread zstd assimp)




