add_library(DDGI
	DDGI.cpp
)

target_include_directories(DDGI PRIVATE $ENV{VULKAN_SDK}/Include)
target_include_directories(DDGI PRIVATE SDK/include)
target_include_directories(DDGI PRIVATE ../)

set_target_properties(DDGI PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)