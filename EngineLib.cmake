# define shader-related variables
if (LINUX)
	set(DXC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Lib/dxc/linux/bin/dxc")
elseif (WIN32)
	set(DXC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Lib/dxc/windows/bin/x64/dxc.exe")
endif()
set(SPIRV_OUTPUT_DIR "${BINARIES_DIR}/PrecompiledShaders/")

include(CommonFunctions.cmake)

# platfrom stuff
if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(TARGET ColumbusEngine)

########## COLUMBUS ENGINE ############
add_compile_options(/MP) # enables multithreaded compilation
add_library(${TARGET} STATIC Core/Engine.cpp)
set_target_properties(${TARGET} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)
set_target_properties(ColumbusEngine PROPERTIES FOLDER "Engine")

function(target_include_engine_directories TO_TARGET)
    target_include_directories(${TO_TARGET} PUBLIC ./)
    target_include_directories(${TO_TARGET} PUBLIC ./Lib/)
    target_include_directories(${TO_TARGET} PUBLIC ./Lib/json/single_include/nlohmann/)
    target_include_directories(${TO_TARGET} PUBLIC ./Lib/MiniMP3)
    target_include_directories(${TO_TARGET} PUBLIC SYSTEM ./Lib/STB/)
    target_include_directories(${TO_TARGET} PUBLIC ./Lib/ZSTD/Include/)
    target_include_directories(${TO_TARGET} PUBLIC ./DDGI/SDK/include/) # RTXGI SDK
    target_include_directories(${TO_TARGET} PUBLIC ./Lib/bullet3/src)
    target_include_directories(${TO_TARGET} PUBLIC ./Lib/GLM)

    if(LINUX)
        target_include_directories(${TO_TARGET} PUBLIC /usr/include/SDL2/)
    elseif(WIN32)
        target_include_directories(${TO_TARGET} PUBLIC ./Lib/SDL2/include)
        target_include_directories(${TO_TARGET} PUBLIC ./Lib/LibTIFF/Include)
        target_include_directories(${TO_TARGET} PUBLIC ./Lib/WinPixEventRuntime/Include)

        target_include_directories(${TO_TARGET} PUBLIC $ENV{VULKAN_SDK}/Include)
    endif()
endfunction()

target_include_engine_directories(${TARGET})
add_dir_to_target(${TARGET} Audio)
add_dir_to_target_recurse(${TARGET} Common)
add_dir_to_target(${TARGET} Core)
if(LINUX)
	add_dir_to_target(${TARGET} Core/Linux)
elseif(WIN32)
	add_dir_to_target(${TARGET} Core/Windows)
endif()
add_dir_to_target_recurse(${TARGET} Editor)
add_dir_to_target_recurse(${TARGET} Game)

#add_dir_to_target_recurse(${TARGET} Graphics) #OpenGL is included
add_dir_to_target(${TARGET} Graphics)
# add_dir_to_target(${TARGET} Graphics/OpenGL) # goodbye my friend
add_dir_to_target(${TARGET} Graphics/Vulkan)
add_dir_to_target_recurse(${TARGET} Graphics/Particles)
add_dir_to_target_recurse(${TARGET} Graphics/RenderPasses)
add_dir_to_target(${TARGET} Graphics/Core)

add_dir_to_target(${TARGET} Math)
add_dir_to_target(${TARGET} Input)
add_dir_to_target(${TARGET} Physics)
add_dir_to_target(${TARGET} Profiling)
add_dir_to_target(${TARGET} Scene)
add_dir_to_target(${TARGET} System)

if (LINUX)
	target_compile_options(${TARGET} PUBLIC -O0 -g)

	# target_compile_options(${TARGET} PUBLIC -fsanitize=address)
	# target_link_libraries(${TARGET} asan)

	target_link_libraries(${TARGET} SDL2 SDL2main GL GLEW png tiff jpeg
		${BULLET_LIB} pthread zstd vulkan shaderc_shared)
	target_link_libraries(${TARGET} unwind dw)
elseif(WIN32)
	target_link_directories(${TARGET} PUBLIC
		./Lib/GLEW/lib/Release/x64
		./Lib/SDL2/lib/x64
		./Lib/ZSTD/lib
		./Lib/LibTIFF/lib/x64
		./Lib/WinPixEVentRuntime/bin/x64
		$ENV{VULKAN_SDK}/Lib
	)
	target_link_libraries(${TARGET} opengl32 SDL2 glew32 zstd tiff bullet WinPixEventRuntime)
	target_link_libraries(${TARGET} vulkan-1 shaderc_shared)
endif()

target_link_libraries(${TARGET} bc7enc)
target_link_libraries(${TARGET} CMP_Compressonator)
target_link_libraries(${TARGET} nfd)
target_link_libraries(${TARGET} imgui)
target_link_libraries(${TARGET} implot)
target_link_libraries(${TARGET} xatlas)
target_link_libraries(${TARGET} spirv_reflect)
target_link_libraries(${TARGET} ShaderBytecode)
target_link_libraries(${TARGET} DDGI)

target_link_libraries(${TARGET} ffx_backend_vk_x64)
target_link_libraries(${TARGET} ffx_dof_x64)
target_link_libraries(${TARGET} ffx_blur_x64)
target_link_libraries(${TARGET} ffx_denoiser_x64)

add_subdirectory(Lib)
add_subdirectory(ShaderBytecode)
add_subdirectory(ShaderCompiler)
add_subdirectory(DDGI)

add_subdirectory(Shaders)
add_dependencies(Shaders ShaderCompiler)