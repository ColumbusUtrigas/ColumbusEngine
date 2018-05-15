cmake_minimum_required(VERSION 2.8)
project(CLMBSEngine)

file(GLOB SRC_AUDIO ./Audio/*.cpp ./Audio/OpenAL/*.cpp)
file(GLOB SRC_COMMON ./Common/Image/*.cpp ./Common/Model/*.cpp ./Common/Sound/*.cpp ./Common/Compression/*.cpp)
file(GLOB SRC_CORE ./Core/Linux/*.cpp ./Core/Platform/*.cpp)
file(GLOB SRC_GRAPHICS ./Graphics/*.cpp ./Graphics/OpenGL/*.cpp ./Graphics/Particles/*.cpp)
file(GLOB SRC_INPUT ./Input/*.cpp)
file(GLOB SRC_PHYSICS ./Physics/*.cpp)
file(GLOB SRC_RENDER_API ./RenderAPI/*.cpp)
file(GLOB SRC_RENDER_API_OPENGL ./RenderAPIOpenGL/*.cpp)
file(GLOB SRC_SCENE ./Scene/*.cpp)
file(GLOB SRC_SYSTEM ./System/*.cpp)

set(SRC ${SRC_AUDIO} ${SRC_COMMON} ${SRC_CORE} ${SRC_GRAPHICS} ${SRC_INPUT} ${SRC_PHYSICS} ${SRC_RENDER_API} ${SRC_RENDER_API_OPENGL} ${SRC_SCENE} ${SRC_SYSTEM} main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -Wall -Wno-reorder -Wno-sign-compare -Wno-strict-aliasing")

set(TARGET ColumbusEngine)

include_directories(./)
include_directories(./Lib/JSON/)
include_directories(./Lib/STB/)
include_directories(./Lib/ZSTD/Include/)
include_directories(/usr/include/SDL2/)
include_directories(/usr/include/bullet/)
include_directories(/usr/include/AL/)

add_executable(${TARGET} ${SRC})

target_link_libraries(${TARGET} SDL2 SDL2main GL GLU GLEW tinyxml2 png tiff jpeg LinearMath BulletCollision BulletDynamics openal pthread)
target_link_libraries(${TARGET} ${CMAKE_SOURCE_DIR}/Lib/ZSTD/Lib/libZSTD.a)







