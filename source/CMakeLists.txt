add_executable(hello_bgfx main.cpp)
target_link_libraries(hello_bgfx glfw bgfxlib)

set_property(TARGET hello_bgfx PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

install(DIRECTORY shaders DESTINATION .)
install(TARGETS hello_bgfx RUNTIME DESTINATION .)
