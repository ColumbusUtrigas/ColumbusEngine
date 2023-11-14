add_executable(ShaderCompiler ShaderCompilerMain.cpp)

target_include_directories(ShaderCompiler PUBLIC ../) # shared includes with the engine
target_link_libraries(ShaderCompiler ShaderBytecode)

set_target_properties(ShaderCompiler PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)