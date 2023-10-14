add_library(ShaderBytecode STATIC ShaderBytecode.h ShaderBytecode.cpp)

target_include_directories(ShaderBytecode PUBLIC ../) # shared includes with the engine
target_link_libraries(ShaderBytecode spirv_reflect)
target_include_directories(ShaderBytecode PUBLIC $ENV{VULKAN_SDK}/Include)

set_target_properties(ShaderBytecode PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)
