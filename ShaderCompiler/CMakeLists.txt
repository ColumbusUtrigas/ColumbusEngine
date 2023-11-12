add_executable(ShaderCompiler ShaderCompilerMain.cpp)

if (LINUX)
	add_custom_target(copy_dxc
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Lib/dxc/linux/bin/dxc" "$<TARGET_FILE_DIR:${TARGET}>/"
	)
endif()

add_dependencies(ShaderCompiler copy_dxc)

target_include_directories(ShaderCompiler PUBLIC ../) # shared includes with the engine
target_link_libraries(ShaderCompiler ShaderBytecode)

set_target_properties(ShaderCompiler PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)