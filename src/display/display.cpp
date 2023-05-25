#include "display.hpp"
#include <SDL.h>
#include <cstdio>
#include <glad/glad.h>
#include <stb_image.h>

// https://www.khronos.org/opengl/wiki/Debug_Output
static void GLAPIENTRY MessageCallback(GLenum source,
                                       GLenum type,
                                       GLuint id,
                                       GLenum severity,
                                       GLsizei length,
                                       const GLchar *message,
                                       const void *userParam)
{
    (void) userParam;
    (void) length;
    (void) source;
    (void) id;

    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
               (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
               type,
               severity,
               message);
    }
}

Display::Display(const char *title,
                 uint32 width,
                 uint32 height)
    : width(width),
      height(height),
      is_open(true),
      vao(0),
      render_buffer_texture(0),
      cubemap_texture(0)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("ERROR: Failed to initialize SDL.\n");
        exit(-1);
    }

    // Load the default OGL library and get all function
    // definitions via GLAD down below.
    SDL_GL_LoadLibrary(nullptr);

    // OpenGL context version and profile
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Bit depth
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    // Window creation
    window_handle = SDL_CreateWindow(title,
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     (int32) width, (int32) height,
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (window_handle == nullptr)
    {
        printf("ERROR (WINDOW): Failed to create SDL Window!\n");
        exit(-1);
    }

    // Create OpenGL context and attach to window
    context = SDL_GL_CreateContext(window_handle);

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        printf("ERROR (WINDOW): Failed to initialize OpenGL context\n");
        exit(-1);
    }

    render_buffer_shader = LoadShaderFromFiles("shaders/framebuffer.vert", "shaders/framebuffer.frag");
    compute_shader = LoadShaderFromFiles("shaders/framebuffer.comp");

    // Query limits
    GLint data1 = -1;
    GLint data2 = -1;
    GLint data3 = -1;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &data1);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &data2);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &data3);
    printf("max size of work group (x, y, z): %d %d %d\n", data1, data2, data3);

    // Turn off VSync
    SDL_GL_SetSwapInterval(0);

    // SDL input specifics
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetWindowInputFocus(window_handle);

    // Enable OpenGL debug callback
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, nullptr);

    InitRenderBuffer();
}

bool Display::InitRenderBuffer()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f
    };

    float uvs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
    };

    // Set up the fullscreen tris
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo[2];
    glCreateBuffers(2, vbo);

    glNamedBufferStorage(vbo[0], sizeof(vertices), vertices, 0);
    glNamedBufferStorage(vbo[1], sizeof(uvs), uvs, 0);

    glVertexArrayVertexBuffer(vao, 0, vbo[0], 0, 3 * sizeof(float));
    glVertexArrayVertexBuffer(vao, 1, vbo[1], 0, 2 * sizeof(float));

    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);

    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 1);

    // Set up the framebuffer texture
    glCreateTextures(GL_TEXTURE_2D, 1, &render_buffer_texture);
    glBindTextureUnit(0, render_buffer_texture);
    glTextureParameteri(render_buffer_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(render_buffer_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(render_buffer_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(render_buffer_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(render_buffer_texture, 1, GL_RGBA16F, (GLsizei) width, (GLsizei) height);
    glBindImageTexture(0, render_buffer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    glBindTextureUnit(0, 0);

    glUseProgram(render_buffer_shader.id);
    glUniform1i(glGetUniformLocation(render_buffer_shader.id, "tex"), 0);

    glUseProgram(compute_shader.id);
    glUniform1i(glGetUniformLocation(compute_shader.id, "screen"), 0);
    glUniform1i(glGetUniformLocation(compute_shader.id, "u_cubemap"), 1);

    // Set up cubemap

    glCreateTextures(GL_TEXTURE_2D, 1, &cubemap_texture);
    glBindTextureUnit(1, cubemap_texture);

    glTextureParameteri(cubemap_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(cubemap_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(cubemap_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemap_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemap_texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    int w = -1, h = -1, c = -1;
    stbi_hdr_to_ldr_gamma(1.0);
	uint8 *data = stbi_load("res/cubemaps/solitude_interior_4k.hdr", &w, &h, &c, 3);
    if (data != nullptr)
    {
        glTextureStorage2D(cubemap_texture, 1, GL_RGB16F, w, h);
        glTextureSubImage2D(cubemap_texture, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glBindTextureUnit(1, 0);

    return true;
}

void CloseDisplay(Display &window)
{
    SDL_GL_DeleteContext(window.context);
    SDL_DestroyWindow(window.window_handle);
    SDL_Quit();
}

void UpdateDisplayTitle(Display &window, const char *new_title)
{
    SDL_SetWindowTitle(window.window_handle, new_title);
}
