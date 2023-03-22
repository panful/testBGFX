set(target_name "bgfx_example")

add_executable(${target_name} "main.cpp" "stb_image_write.h" "stb_image.h")
target_link_libraries(${target_name} glfw bgfxlib)

set_property(TARGET ${target_name} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

install(TARGETS ${target_name} RUNTIME DESTINATION .)

# 拷贝shader文件到安装目录
install(DIRECTORY shaders DESTINATION .)
# 拷贝shader文件到生成目录
add_custom_command(TARGET ${target_name} 
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${target_name}>/shaders
)
