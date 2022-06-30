#pragma once
#include "../defines.hpp"
#include "../core/array.hpp"
#include <SDL_video.h>

struct Display
{
	// Window specific data
	uint32 width, height;
	SDL_Window *window_handle;
	SDL_GLContext context;
	bool is_open;

	// Render buffer data
	uint32 vao, ebo;
	uint32 render_buffer_texture;
	uint32 rb_shader_program;
	uint32 compute_shader_program;
};

Display CreateDisplay(const char* title, uint32 width, uint32 height);
bool InitRenderBuffer(Display &window);
void CloseDisplay(Display &window);