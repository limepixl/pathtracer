#pragma once
#include "../defines.hpp"
#include "../core/array.hpp"
#include "../resource/shader.hpp"
#include <SDL_video.h>

struct Display
{
    // Window specific data
    uint32 width, height;
    SDL_Window *window_handle;
    SDL_GLContext context;
    bool is_open;

    // Render buffer data
    uint32 vao;
    uint32 render_buffer_texture;
    Shader render_buffer_shader;
    Shader compute_shader;

    uint32 cubemap_texture;

    Display(const char *title, uint32 width, uint32 height);
    bool InitRenderBuffer();
};

void CloseDisplay(Display &window);

void UpdateDisplayTitle(Display &window, const char *new_title);
