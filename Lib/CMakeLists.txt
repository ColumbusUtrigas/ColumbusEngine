# exported libraries:
# - bc7enc
# - CMP_Compressonator
# - spirv_reflect
# - imgui
# - implot
# - xatlas
# - nfd
# - bullet

########## SPIRV-REFLECT ############
file(GLOB SPIRV_REFLECT_SRC
	SPIRV-Reflect/spirv_reflect.h
	SPIRV-Reflect/spirv_reflect.c)

add_library(spirv_reflect STATIC ${SPIRV_REFLECT_SRC})
target_include_directories(spirv_reflect PUBLIC ./SPIRV-Reflect)
set_target_properties(spirv_reflect PROPERTIES FOLDER "Lib")

########## IMGUI OPENGL3 SDL2 VK ##########
file(GLOB IMGUI_SRC
	imgui/imgui.h
	imgui/*.cpp
	imgui/misc/cpp/*.cpp
	imgui/backends/imgui_impl_sdl2.cpp
	imgui/backends/imgui_impl_opengl3.cpp
	imgui/backends/imgui_impl_vulkan.cpp
	ImGuizmo/*.cpp)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC imgui)
set_target_properties(imgui PROPERTIES FOLDER "Lib")

if(LINUX)
	target_include_directories(imgui PUBLIC /usr/include/SDL2/)
elseif(WIN32)
	target_include_directories(imgui PUBLIC GLEW/include)
	target_include_directories(imgui PUBLIC SDL2/include)
	target_include_directories(imgui PUBLIC $ENV{VULKAN_SDK}/Include)
endif()

########## IMPLOT ##########
file(GLOB IMPLOT_SRC
	implot/implot.h
	implot/implot_internal.h
	implot/implot.cpp
	implot/implot_items.cpp
)
add_library(implot STATIC ${IMPLOT_SRC})
target_include_directories(implot PUBLIC imgui)
target_link_libraries(implot imgui)
set_target_properties(implot PROPERTIES FOLDER "Lib")

########## XATLAS ##########
file(GLOB XATLAS_SRC xatlas/xatlas.h xatlas/xatlas.cpp)
add_library(xatlas STATIC ${XATLAS_SRC})
set_target_properties(xatlas PROPERTIES FOLDER "Lib")

########## NFD ##########
if(LINUX)
	file(GLOB NFD_SRC
		nativefiledialog/src/nfd_gtk.c
		nativefiledialog/src/common.h
		nativefiledialog/src/nfd_common.h
		nativefiledialog/src/nfd_common.c)

	add_library(nfd STATIC ${NFD_SRC})
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

	target_include_directories(nfd PRIVATE nativefiledialog/src/include)
	target_include_directories(nfd PRIVATE ${GTK3_INCLUDE_DIRS})
	target_link_directories(nfd PRIVATE ${GTK3_LIBRARY_DIRS})
	target_link_libraries(nfd ${GTK3_LIBRARIES})
elseif(WIN32)
	file(GLOB NFD_SRC
		nativefiledialog/src/nfd_win.cpp
		nativefiledialog/src/common.h
		nativefiledialog/src/nfd_common.h
		nativefiledialog/src/nfd_common.c)
	add_library(nfd STATIC ${NFD_SRC})
	target_include_directories(nfd PRIVATE nativefiledialog/src/include)
	target_link_libraries(nfd comctl32)
endif()
set_target_properties(nfd PROPERTIES FOLDER "Lib")

########## BC7ENC ##########
file(GLOB BC7ENC_SRC
	bc7enc/bc7decomp.cpp
	bc7enc/bc7decomp.h
	bc7enc/bc7enc.c
	bc7enc/bc7enc.h
	bc7enc/dds_defs.h
	bc7enc/rgbcx.h
	bc7enc/rgbcx_table4.h
	bc7enc/rgbcx_impl.cpp
)
add_library(bc7enc STATIC ${BC7ENC_SRC})
set_target_properties(bc7enc PROPERTIES FOLDER "Lib")

########## BULLET PHYSICS #############
dir_to_var_recurse(bullet3/src/BulletCollision BULLET_COLLISION_SRC)
dir_to_var_recurse(bullet3/src/BulletDynamics BULLET_DYNAMICS_SRC)
dir_to_var_recurse(bullet3/src/LinearMath BULLET_MATH_SRC)
set(BULLET_SRC ${BULLET_COLLISION_SRC} ${BULLET_DYNAMICS_SRC} ${BULLET_MATH_SRC})
add_library(bullet STATIC ${BULLET_SRC})
target_include_directories(bullet PRIVATE bullet3/src)
target_compile_definitions(bullet PRIVATE BT_THREADSAFE)
set(BULLET_LIB bullet)
set_target_properties(bullet PROPERTIES FOLDER "Lib")

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

#add_subdirectory(bullet3)
#set_target_properties(BulletCollision PROPERTIES FOLDER Bullet)
#set_target_properties(BulletDynamics  PROPERTIES FOLDER Bullet)
#set_target_properties(LinearMath      PROPERTIES FOLDER Bullet)
#set_target_properties(Bullet3Common      PROPERTIES FOLDER Bullet)
#set_target_properties(BulletInverseDynamics      PROPERTIES FOLDER Bullet)
#set_target_properties(BulletSoftBody      PROPERTIES FOLDER Bullet)

### Compressonator
set(PROJECT_FOLDER_SDK_LIBS "Lib/Compressonator")
get_filename_component(PROJECT_SOURCE_DIR "./compressonator" ABSOLUTE)
add_subdirectory(compressonator/cmp_core/)
add_subdirectory(compressonator/cmp_compressonatorlib/)

### FidelityFX
set(FFX_API_BACKEND "VK_X64")
set(FFX_AUTO_COMPILE_SHADERS ON)
set(FFX_BLUR ON)
set(FFX_DOF ON)
add_subdirectory_with_folder("Lib/FidelityFX" FidelityFX-SDK/sdk/)
