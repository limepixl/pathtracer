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

	// Timing data
	uint32 last_time = 0;
	uint32 last_report = 0;
	uint32 frame_count = 0;
	uint32 delta_time;
	uint32 current_time;

	// Input state
	uint8 *keyboard_state;

    Display(const char *title, uint32 width, uint32 height);

    bool InitRenderBuffer();
	void FrameStartMarker();
	void FrameEndMarker();
	void ProcessEvents(struct Camera &cam);
	void CloseDisplay();
};
