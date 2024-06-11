/*
 * 1. BGFX绘制一个立方体
 * 2. 将BGFX绘制的结果保存为图片
 * 3. 更新vertexBuffer，修改立方体颜色
 * 4. 使用 Vulkan 无头渲染(Headless)
 * 5. 修改窗口大小
 */

#define TEST5

#ifdef TEST1

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"

#include <string>

const int WNDW_WIDTH  = 800;
const int WNDW_HEIGHT = 600;

bgfx::ShaderHandle loadShader(const char* FILENAME)
{
    std::string shaderPath = "???";

    switch (bgfx::getRendererType())
    {
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Vulkan:
            shaderPath = "shaders/spirv/";
            break;
        default:
            shaderPath = "???";
    }

    shaderPath += FILENAME;

    FILE* file = fopen(shaderPath.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory* mem = bgfx::alloc(fileSize + 1);
    fread(mem->data, 1, fileSize, file);
    mem->data[mem->size - 1] = '\0';
    fclose(file);

    return bgfx::createShader(mem);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "GLFW_BGFX", nullptr, nullptr);

    // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
    // Most graphics APIs must be used on the same thread that created the window.
    bgfx::renderFrame();

    bgfx::Init bgfxInit;
    bgfxInit.platformData.nwh = glfwGetWin32Window(window);
    bgfxInit.type             = bgfx::RendererType::Count; // Automatically choose a renderer.
    // bgfxInit.type = bgfx::RendererType::Vulkan;
    // bgfxInit.type = bgfx::RendererType::OpenGL;
    // bgfxInit.type = bgfx::RendererType::Direct3D12;
    bgfxInit.resolution.width  = WNDW_WIDTH;
    bgfxInit.resolution.height = WNDW_HEIGHT;
    bgfxInit.resolution.reset  = BGFX_RESET_VSYNC;
    bgfx::init(bgfxInit);

    struct PosColorVertex
    {
        float x;
        float y;
        float z;
        uint32_t abgr;
    };

    // 顶点数据 立方体共8个顶点
    // clang-format off
    static PosColorVertex cubeVertices[] = {
            {-1.0f,  1.0f,  1.0f,  0xff000000},
            { 1.0f,  1.0f,  1.0f,  0xff0000ff},
            {-1.0f, -1.0f,  1.0f,  0xff00ff00},
            { 1.0f, -1.0f,  1.0f,  0xff00ffff},
            {-1.0f,  1.0f, -1.0f,  0xffff0000},
            { 1.0f,  1.0f, -1.0f,  0xffff00ff},
            {-1.0f, -1.0f, -1.0f,  0xffffff00},
            { 1.0f, -1.0f, -1.0f,  0xffffffff},
        };
    // clang-format on

    // 索引数据 立方体共6个面，每个面2个三角形
    // clang-format off
    static const uint16_t cubeTriList[] = {
            0, 1, 2, 1, 3, 2,
            4, 6, 5, 5, 6, 7,
            0, 2, 4, 4, 2, 6,
            1, 5, 3, 5, 7, 3,
            0, 4, 1, 4, 5, 1,
            2, 3, 6, 6, 3, 7,
        };
    // clang-format on

    // 数据填充
    // VBO EBO
    bgfx::VertexLayout pcvDecl;
    pcvDecl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true).end();
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::makeRef(cubeVertices, sizeof(cubeVertices)), pcvDecl);
    bgfx::IndexBufferHandle ibh  = bgfx::createIndexBuffer(bgfx::makeRef(cubeTriList, sizeof(cubeTriList)));

    // 着色器程序
    // shaderProgram
    bgfx::ShaderHandle vsh      = loadShader("vs_cubes.bin");
    bgfx::ShaderHandle fsh      = loadShader("fs_cubes.bin");
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

    // Rendering Loop
    unsigned int counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, WNDW_WIDTH, WNDW_HEIGHT);

        // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        const bx::Vec3 at  = {0.0f, 0.0f, 0.0f};
        const bx::Vec3 eye = {0.0f, 0.0f, -5.0f};
        float view[16];
        bx::mtxLookAt(view, eye, at);
        float proj[16];
        bx::mtxProj(proj, 60.0f, float(WNDW_WIDTH) / float(WNDW_HEIGHT), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view, proj);
        float mtx[16];
        bx::mtxRotateXY(mtx, counter * 0.01f, counter * 0.01f);
        bgfx::setTransform(mtx);

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        // submit的第一个参数表示viewid
        bgfx::submit(0, program);
        bgfx::frame();

        counter++;
    }

    bgfx::shutdown();
    glfwTerminate();
    return EXIT_SUCCESS;
}

#endif // TEST1

#ifdef TEST2

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"

#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const int WNDW_WIDTH  = 800;
const int WNDW_HEIGHT = 600;

bgfx::ShaderHandle loadShader(const char* FILENAME)
{
    std::string shaderPath = "???";

    switch (bgfx::getRendererType())
    {
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Vulkan:
            shaderPath = "shaders/spirv/";
            break;
        default:
            shaderPath = "???";
    }

    shaderPath += FILENAME;

    FILE* file = fopen(shaderPath.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory* mem = bgfx::alloc(fileSize + 1);
    fread(mem->data, 1, fileSize, file);
    mem->data[mem->size - 1] = '\0';
    fclose(file);

    return bgfx::createShader(mem);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "GLFW_BGFX", nullptr, nullptr);

    // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
    // Most graphics APIs must be used on the same thread that created the window.
    bgfx::renderFrame();

    bgfx::Init bgfxInit;
    bgfxInit.platformData.nwh = glfwGetWin32Window(window);
    bgfxInit.type             = bgfx::RendererType::Count; // Automatically choose a renderer.
    // bgfxInit.type = bgfx::RendererType::Vulkan;
    // bgfxInit.type = bgfx::RendererType::OpenGL;
    // bgfxInit.type = bgfx::RendererType::Direct3D12;
    bgfxInit.resolution.width  = WNDW_WIDTH;
    bgfxInit.resolution.height = WNDW_HEIGHT;
    bgfxInit.resolution.reset  = BGFX_RESET_VSYNC;
    bgfx::init(bgfxInit);

    struct PosColorVertex
    {
        float x;
        float y;
        float z;
        uint32_t abgr;
    };

    // 顶点数据 立方体共8个顶点
    // clang-format off
    static PosColorVertex cubeVertices[] = {
            {-1.0f,  1.0f,  1.0f,  0xff000000},
            { 1.0f,  1.0f,  1.0f,  0xff0000ff},
            {-1.0f, -1.0f,  1.0f,  0xff00ff00},
            { 1.0f, -1.0f,  1.0f,  0xff00ffff},
            {-1.0f,  1.0f, -1.0f,  0xffff0000},
            { 1.0f,  1.0f, -1.0f,  0xffff00ff},
            {-1.0f, -1.0f, -1.0f,  0xffffff00},
            { 1.0f, -1.0f, -1.0f,  0xffffffff},
        };
    // clang-format on

    // 索引数据 立方体共6个面，每个面2个三角形
    // clang-format off
    static const uint16_t cubeTriList[] = {
            0, 1, 2, 1, 3, 2,
            4, 6, 5, 5, 6, 7,
            0, 2, 4, 4, 2, 6,
            1, 5, 3, 5, 7, 3,
            0, 4, 1, 4, 5, 1,
            2, 3, 6, 6, 3, 7,
        };
    // clang-format on

    // 数据填充
    // VBO EBO
    bgfx::VertexLayout pcvDecl;
    pcvDecl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true).end();
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::makeRef(cubeVertices, sizeof(cubeVertices)), pcvDecl);
    bgfx::IndexBufferHandle ibh  = bgfx::createIndexBuffer(bgfx::makeRef(cubeTriList, sizeof(cubeTriList)));

    // 着色器程序
    // shaderProgram
    bgfx::ShaderHandle vsh      = loadShader("vs_cubes.bin");
    bgfx::ShaderHandle fsh      = loadShader("fs_cubes.bin");
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

    // storage pixels
    // 使用new char[]，delete[] 时会崩溃，std::array也是如此
    std::vector<uint8_t> data(WNDW_WIDTH * WNDW_HEIGHT * 4);

    // Rendering Loop
    unsigned int counter = 0;
    while (!glfwWindowShouldClose(window) && counter < 10)
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        // Create a texture with the BGFX_TEXTURE_RT flag to indicate that it is a render target texture.
        auto colorTexture = bgfx::createTexture2D(
            WNDW_WIDTH,
            WNDW_HEIGHT,
            false,
            1,
            bgfx::TextureFormat::RGBA8,
            0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT_MSAA_X16
        );

        // Create a frame buffer object with the texture as its color attachment.
        bgfx::FrameBufferHandle frameBuffer = bgfx::createFrameBuffer(1, &colorTexture, false);

        // Set the current view's frame buffer to the frame buffer object.
        bgfx::setViewFrameBuffer(0, frameBuffer);

        // Render to the frame buffer object.
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFF0000FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, WNDW_WIDTH, WNDW_HEIGHT);

        // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        const bx::Vec3 at  = {0.0f, 0.0f, 0.0f};
        const bx::Vec3 eye = {0.0f, 0.0f, -5.0f};
        float view[16];
        bx::mtxLookAt(view, eye, at);
        float proj[16];
        bx::mtxProj(proj, 60.0f, float(WNDW_WIDTH) / float(WNDW_HEIGHT), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view, proj);
        float mtx[16];
        bx::mtxRotateXY(mtx, counter * 0.01f, counter * 0.01f);
        bgfx::setTransform(mtx);

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        bgfx::submit(0, program);
        bgfx::frame();

        // Create a texture with the BGFX_TEXTURE_READ_BACK flag to indicate that it can be read back from the GPU.
        auto textureHandle = bgfx::createTexture2D(
            WNDW_WIDTH,
            WNDW_HEIGHT,
            false,
            1,
            bgfx::TextureFormat::RGBA8,
            0 | BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
        );

        // Blit the color attachment of the frame buffer object to the read-back texture.
        bgfx::blit(0, textureHandle, 0, 0, colorTexture);

        // Read the texture data using bgfx::readTexture().
        bgfx::readTexture(textureHandle, data.data());

        // Save the texture data to an image file using a library such as stb_image_write.
        // 第0帧即第一次调用bgfx::frame()之后，图像数据全为0，第1帧即第二次调用bgfx::frame()之后，图像数据不为0
        auto fileName = "output_" + std::to_string(counter++) + ".png";
        stbi_write_png(fileName.c_str(), WNDW_WIDTH, WNDW_HEIGHT, 4, data.data(), WNDW_WIDTH * 4);

        // Destroy the texture and frame buffer object.
        bgfx::destroy(colorTexture);
        bgfx::destroy(frameBuffer);
        bgfx::destroy(textureHandle);
    }

    std::cout << "save " << counter << " images\n";

    bgfx::shutdown();
    glfwTerminate();
    return EXIT_SUCCESS;
}

#endif // TEST2

#ifdef TEST3

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"

#include <string>

const int WNDW_WIDTH  = 800;
const int WNDW_HEIGHT = 600;

bgfx::ShaderHandle loadShader(const char* FILENAME)
{
    std::string shaderPath = "???";

    switch (bgfx::getRendererType())
    {
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Vulkan:
            shaderPath = "shaders/spirv/";
            break;
        default:
            shaderPath = "???";
    }

    shaderPath += FILENAME;

    FILE* file = fopen(shaderPath.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory* mem = bgfx::alloc(fileSize + 1);
    fread(mem->data, 1, fileSize, file);
    mem->data[mem->size - 1] = '\0';
    fclose(file);

    return bgfx::createShader(mem);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "GLFW_BGFX", nullptr, nullptr);

    // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
    // Most graphics APIs must be used on the same thread that created the window.
    bgfx::renderFrame();

    bgfx::Init bgfxInit;
    bgfxInit.platformData.nwh = glfwGetWin32Window(window);
    bgfxInit.type             = bgfx::RendererType::Count; // Automatically choose a renderer.
    // bgfxInit.type = bgfx::RendererType::Vulkan;
    // bgfxInit.type = bgfx::RendererType::OpenGL;
    // bgfxInit.type = bgfx::RendererType::Direct3D12;
    bgfxInit.resolution.width  = WNDW_WIDTH;
    bgfxInit.resolution.height = WNDW_HEIGHT;
    bgfxInit.resolution.reset  = BGFX_RESET_VSYNC;
    bgfx::init(bgfxInit);

    struct PosColorVertex
    {
        float x;
        float y;
        float z;
        uint32_t abgr;
    };

    // clang-format off
    // 顶点数据 立方体共8个顶点
    static PosColorVertex cubeVertices[] = {
            {-1.0f,  1.0f,  1.0f,  0xff000000},
            { 1.0f,  1.0f,  1.0f,  0xff0000ff},
            {-1.0f, -1.0f,  1.0f,  0xff00ff00},
            { 1.0f, -1.0f,  1.0f,  0xff00ffff},
            {-1.0f,  1.0f, -1.0f,  0xffff0000},
            { 1.0f,  1.0f, -1.0f,  0xffff00ff},
            {-1.0f, -1.0f, -1.0f,  0xffffff00},
            { 1.0f, -1.0f, -1.0f,  0xffffffff},
        };

    // 黄色的立方体，颜色布局方式 {0xABGR}
    static PosColorVertex cubeVertices2[] = {
            {-1.0f,  1.0f,  1.0f,  0xff00ffff},
            { 1.0f,  1.0f,  1.0f,  0xff00ffff},
            {-1.0f, -1.0f,  1.0f,  0xff00ffff},
            { 1.0f, -1.0f,  1.0f,  0xff00ffff},
            {-1.0f,  1.0f, -1.0f,  0xff00ffff},
            { 1.0f,  1.0f, -1.0f,  0xff00ffff},
            {-1.0f, -1.0f, -1.0f,  0xff00ffff},
            { 1.0f, -1.0f, -1.0f,  0xff00ffff},
        };

    // 索引数据 立方体共6个面，每个面2个三角形
    static const uint16_t cubeTriList[] = {
            0, 1, 2, 1, 3, 2,
            4, 6, 5, 5, 6, 7,
            0, 2, 4, 4, 2, 6,
            1, 5, 3, 5, 7, 3,
            0, 4, 1, 4, 5, 1,
            2, 3, 6, 6, 3, 7,
        };
    // clang-format on

    // 数据填充
    // VBO EBO
    bgfx::VertexLayout pcvDecl;
    pcvDecl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true).end();
    auto vbh = bgfx::createDynamicVertexBuffer(bgfx::makeRef(cubeVertices, sizeof(cubeVertices)), pcvDecl);
    auto ibh = bgfx::createDynamicIndexBuffer(bgfx::makeRef(cubeTriList, sizeof(cubeTriList)));

    // 更新vertexBuffer
    bgfx::update(vbh, 0, bgfx::makeRef(cubeVertices2, sizeof(cubeVertices2)));

    // 着色器程序
    // shaderProgram
    bgfx::ShaderHandle vsh      = loadShader("vs_cubes.bin");
    bgfx::ShaderHandle fsh      = loadShader("fs_cubes.bin");
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

    // Rendering Loop
    unsigned int counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x204060FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, WNDW_WIDTH, WNDW_HEIGHT);

        // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        const bx::Vec3 at  = {0.0f, 0.0f, 0.0f};
        const bx::Vec3 eye = {0.0f, 0.0f, -5.0f};
        float view[16];
        bx::mtxLookAt(view, eye, at);
        float proj[16];
        bx::mtxProj(proj, 60.0f, float(WNDW_WIDTH) / float(WNDW_HEIGHT), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view, proj);
        float mtx[16];
        bx::mtxRotateXY(mtx, counter * 0.01f, counter * 0.01f);
        bgfx::setTransform(mtx);

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        // submit的第一个参数表示viewid
        bgfx::submit(0, program);
        bgfx::frame();

        counter++;
    }

    bgfx::shutdown();
    glfwTerminate();
    return EXIT_SUCCESS;
}

#endif // TEST3

#ifdef TEST4

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"

#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const int WNDW_WIDTH  = 800;
const int WNDW_HEIGHT = 600;

bgfx::ShaderHandle loadShader(const char* FILENAME)
{
    std::string shaderPath = "???";

    switch (bgfx::getRendererType())
    {
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Vulkan:
            shaderPath = "shaders/spirv/";
            break;
        default:
            shaderPath = "???";
    }

    shaderPath += FILENAME;

    FILE* file = fopen(shaderPath.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory* mem = bgfx::alloc(fileSize + 1);
    fread(mem->data, 1, fileSize, file);
    mem->data[mem->size - 1] = '\0';
    fclose(file);

    return bgfx::createShader(mem);
}

int main()
{
    // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
    // Most graphics APIs must be used on the same thread that created the window.
    bgfx::renderFrame();

    bgfx::Init bgfxInit;
    bgfxInit.platformData.nwh = nullptr;
    // bgfxInit.type             = bgfx::RendererType::Count; // Automatically choose a renderer.
    bgfxInit.type = bgfx::RendererType::Vulkan;
    // bgfxInit.type = bgfx::RendererType::OpenGL;
    // bgfxInit.type = bgfx::RendererType::Direct3D12;
    bgfxInit.resolution.width  = WNDW_WIDTH;
    bgfxInit.resolution.height = WNDW_HEIGHT;
    bgfxInit.resolution.reset  = BGFX_RESET_VSYNC;
    bgfx::init(bgfxInit);

    struct PosColorVertex
    {
        float x;
        float y;
        float z;
        uint32_t abgr;
    };

    // 顶点数据 立方体共8个顶点
    // clang-format off
    static PosColorVertex cubeVertices[] = {
            {-1.0f,  1.0f,  1.0f,  0xff000000},
            { 1.0f,  1.0f,  1.0f,  0xff0000ff},
            {-1.0f, -1.0f,  1.0f,  0xff00ff00},
            { 1.0f, -1.0f,  1.0f,  0xff00ffff},
            {-1.0f,  1.0f, -1.0f,  0xffff0000},
            { 1.0f,  1.0f, -1.0f,  0xffff00ff},
            {-1.0f, -1.0f, -1.0f,  0xffffff00},
            { 1.0f, -1.0f, -1.0f,  0xffffffff},
        };
    // clang-format on

    // 索引数据 立方体共6个面，每个面2个三角形
    // clang-format off
    static const uint16_t cubeTriList[] = {
            0, 1, 2, 1, 3, 2,
            4, 6, 5, 5, 6, 7,
            0, 2, 4, 4, 2, 6,
            1, 5, 3, 5, 7, 3,
            0, 4, 1, 4, 5, 1,
            2, 3, 6, 6, 3, 7,
        };
    // clang-format on

    // 数据填充
    // VBO EBO
    bgfx::VertexLayout pcvDecl;
    pcvDecl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true).end();
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::makeRef(cubeVertices, sizeof(cubeVertices)), pcvDecl);
    bgfx::IndexBufferHandle ibh  = bgfx::createIndexBuffer(bgfx::makeRef(cubeTriList, sizeof(cubeTriList)));

    // 着色器程序
    // shaderProgram
    bgfx::ShaderHandle vsh      = loadShader("vs_cubes.bin");
    bgfx::ShaderHandle fsh      = loadShader("fs_cubes.bin");
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

    // storage pixels
    // 使用new char[]，delete[] 时会崩溃，std::array也是如此
    std::vector<uint8_t> data(WNDW_WIDTH * WNDW_HEIGHT * 4);

    // Rendering Loop
    unsigned int counter = 0;
    while (counter < 10)
    {
        // Create a texture with the BGFX_TEXTURE_RT flag to indicate that it is a render target texture.
        auto colorTexture = bgfx::createTexture2D(
            WNDW_WIDTH,
            WNDW_HEIGHT,
            false,
            1,
            bgfx::TextureFormat::RGBA8,
            0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_RT_MSAA_X16
        );

        // Create a frame buffer object with the texture as its color attachment.
        bgfx::FrameBufferHandle frameBuffer = bgfx::createFrameBuffer(1, &colorTexture, false);

        // Set the current view's frame buffer to the frame buffer object.
        bgfx::setViewFrameBuffer(0, frameBuffer);

        // Render to the frame buffer object.
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFF0000FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, WNDW_WIDTH, WNDW_HEIGHT);

        // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        const bx::Vec3 at  = {0.0f, 0.0f, 0.0f};
        const bx::Vec3 eye = {0.0f, 0.0f, -5.0f};
        float view[16];
        bx::mtxLookAt(view, eye, at);
        float proj[16];
        bx::mtxProj(proj, 60.0f, float(WNDW_WIDTH) / float(WNDW_HEIGHT), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view, proj);
        float mtx[16];
        bx::mtxRotateXY(mtx, counter * 0.01f, counter * 0.01f);
        bgfx::setTransform(mtx);

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        bgfx::submit(0, program);
        bgfx::frame();

        // Create a texture with the BGFX_TEXTURE_READ_BACK flag to indicate that it can be read back from the GPU.
        auto textureHandle = bgfx::createTexture2D(
            WNDW_WIDTH,
            WNDW_HEIGHT,
            false,
            1,
            bgfx::TextureFormat::RGBA8,
            0 | BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT
                | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
        );

        // Blit the color attachment of the frame buffer object to the read-back texture.
        bgfx::blit(0, textureHandle, 0, 0, colorTexture);

        // Read the texture data using bgfx::readTexture().
        bgfx::readTexture(textureHandle, data.data());

        // Save the texture data to an image file using a library such as stb_image_write.
        // 第0帧即第一次调用bgfx::frame()之后，图像数据全为0，第1帧即第二次调用bgfx::frame()之后，图像数据不为0
        auto fileName = "output_" + std::to_string(counter++) + ".png";
        stbi_write_png(fileName.c_str(), WNDW_WIDTH, WNDW_HEIGHT, 4, data.data(), WNDW_WIDTH * 4);

        // Destroy the texture and frame buffer object.
        bgfx::destroy(colorTexture);
        bgfx::destroy(frameBuffer);
        bgfx::destroy(textureHandle);
    }

    std::cout << "save " << counter << " images\n";

    bgfx::shutdown();
    return EXIT_SUCCESS;
}

#endif // TEST4

#ifdef TEST5

#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"

#include <iostream>
#include <string>

int WNDW_WIDTH  = 800;
int WNDW_HEIGHT = 600;

bgfx::ShaderHandle loadShader(const char* FILENAME)
{
    std::string shaderPath = "???";

    switch (bgfx::getRendererType())
    {
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Vulkan:
            shaderPath = "shaders/spirv/";
            break;
        default:
            shaderPath = "???";
    }

    shaderPath += FILENAME;

    FILE* file = fopen(shaderPath.c_str(), "rb");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const bgfx::Memory* mem = bgfx::alloc(fileSize + 1);
    fread(mem->data, 1, fileSize, file);
    mem->data[mem->size - 1] = '\0';
    fclose(file);

    return bgfx::createShader(mem);
}

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) noexcept
{
    std::cout << "Width: " << width << "\tHeight: " << height << std::endl;
    WNDW_WIDTH  = width;
    WNDW_HEIGHT = height;

    bgfx::reset(width, height, BGFX_RESET_VSYNC);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WNDW_WIDTH, WNDW_HEIGHT, "GLFW_BGFX", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

    // Call bgfx::renderFrame before bgfx::init to signal to bgfx not to create a render thread.
    // Most graphics APIs must be used on the same thread that created the window.
    bgfx::renderFrame();

    bgfx::Init bgfxInit;
    bgfxInit.platformData.nwh = glfwGetWin32Window(window);
    // bgfxInit.type             = bgfx::RendererType::Count; // Automatically choose a renderer.
    bgfxInit.type = bgfx::RendererType::Vulkan;
    // bgfxInit.type = bgfx::RendererType::OpenGL;
    // bgfxInit.type = bgfx::RendererType::Direct3D12;
    bgfxInit.resolution.width  = WNDW_WIDTH;
    bgfxInit.resolution.height = WNDW_HEIGHT;
    bgfxInit.resolution.reset  = BGFX_RESET_VSYNC;
    bgfx::init(bgfxInit);

    struct PosColorVertex
    {
        float x;
        float y;
        float z;
        uint32_t abgr;
    };

    // 顶点数据 立方体共8个顶点
    // clang-format off
    static PosColorVertex cubeVertices[] = {
            {-1.0f,  1.0f,  1.0f,  0xff000000},
            { 1.0f,  1.0f,  1.0f,  0xff0000ff},
            {-1.0f, -1.0f,  1.0f,  0xff00ff00},
            { 1.0f, -1.0f,  1.0f,  0xff00ffff},
            {-1.0f,  1.0f, -1.0f,  0xffff0000},
            { 1.0f,  1.0f, -1.0f,  0xffff00ff},
            {-1.0f, -1.0f, -1.0f,  0xffffff00},
            { 1.0f, -1.0f, -1.0f,  0xffffffff},
        };
    // clang-format on

    // 索引数据 立方体共6个面，每个面2个三角形
    // clang-format off
    static const uint16_t cubeTriList[] = {
            0, 1, 2, 1, 3, 2,
            4, 6, 5, 5, 6, 7,
            0, 2, 4, 4, 2, 6,
            1, 5, 3, 5, 7, 3,
            0, 4, 1, 4, 5, 1,
            2, 3, 6, 6, 3, 7,
        };
    // clang-format on

    // 数据填充
    // VBO EBO
    bgfx::VertexLayout pcvDecl;
    pcvDecl.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true).end();
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::makeRef(cubeVertices, sizeof(cubeVertices)), pcvDecl);
    bgfx::IndexBufferHandle ibh  = bgfx::createIndexBuffer(bgfx::makeRef(cubeTriList, sizeof(cubeTriList)));

    // 着色器程序
    // shaderProgram
    bgfx::ShaderHandle vsh      = loadShader("vs_cubes.bin");
    bgfx::ShaderHandle fsh      = loadShader("fs_cubes.bin");
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

    // Rendering Loop
    unsigned int counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        std::cout << "Connter: " << counter << std::endl;

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, WNDW_WIDTH, WNDW_HEIGHT);

        // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        const bx::Vec3 at  = {0.0f, 0.0f, 0.0f};
        const bx::Vec3 eye = {0.0f, 0.0f, -5.0f};
        float view[16];
        bx::mtxLookAt(view, eye, at);
        float proj[16];
        bx::mtxProj(proj, 60.0f, float(WNDW_WIDTH) / float(WNDW_HEIGHT), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view, proj);
        float mtx[16];
        bx::mtxRotateXY(mtx, counter * 0.01f, counter * 0.01f);
        bgfx::setTransform(mtx);

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        // submit的第一个参数表示viewid
        bgfx::submit(0, program);
        bgfx::frame();

        counter++;
    }

    bgfx::shutdown();
    glfwTerminate();
    return EXIT_SUCCESS;
}

#endif // TEST5
