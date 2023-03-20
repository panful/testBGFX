set(target_name bgfx_draw_cube)

add_executable(${target_name} main.cpp)
target_link_libraries(${target_name} glfw bgfxlib)

set_property(TARGET ${target_name} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

install(DIRECTORY shaders DESTINATION .)
install(TARGETS ${target_name} RUNTIME DESTINATION .)

add_custom_command(TARGET ${target_name} 
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${target_name}>/shaders
)
