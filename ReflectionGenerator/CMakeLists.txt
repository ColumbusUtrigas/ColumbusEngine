add_executable(ReflectionGenerator ReflectionGeneratorMain.cpp)

# target_include_directories(ReflectionGenerator PUBLIC ../) # shared includes with the engine
set_target_properties(ReflectionGenerator PROPERTIES FOLDER "Engine")

set_target_properties(ReflectionGenerator PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES)