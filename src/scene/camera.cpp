#include "camera.hpp"
#include "../defines.hpp"
#include "../math/math.hpp"
#include <SDL.h>
#include <glad/glad.h>

Camera::Camera(Vec3f orig, Vec3f fwd, Vec3f r, float fly_speed, float look_sens)
	: origin(orig)
	, forward(fwd)
	, right(r)
	, fly_speed(fly_speed)
	, look_sens(look_sens)
{
	xpos = WIDTH / 2.0f;
	ypos = HEIGHT / 2.0f;

	glCreateBuffers(1, &cam_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, cam_ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cam_ubo);
	glNamedBufferStorage(cam_ubo, sizeof(CameraGLSL), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::mouse_look(float xrel, float yrel)
{
	xpos += xrel * look_sens;
	ypos -= yrel * look_sens;

	forward.x = cosf(Radians(xpos)) * cosf(Radians(ypos));
	forward.y = sinf(Radians(ypos));
	forward.z = sinf(Radians(xpos)) * cosf(Radians(ypos));
	forward = NormalizeVec3f(forward);

	right = NormalizeVec3f(Cross(forward, CreateVec3f(0.0f, 1.0f, 0.0f)));
}

void Camera::move(const uint8 *keyboard_state, uint32 delta_time, uint32 &frame_count)
{
	if (keyboard_state[SDL_SCANCODE_W])
	{
		origin += forward * fly_speed * (float)delta_time;
		frame_count = 0;
	}
	if (keyboard_state[SDL_SCANCODE_S])
	{
		origin += -forward * fly_speed * (float)delta_time;
		frame_count = 0;
	}
	if (keyboard_state[SDL_SCANCODE_A])
	{
		origin += -right * fly_speed * (float)delta_time;
		frame_count = 0;
	}
	if (keyboard_state[SDL_SCANCODE_D])
	{
		origin += right * fly_speed * (float)delta_time;
		frame_count = 0;
	}
	if (keyboard_state[SDL_SCANCODE_SPACE])
	{
		Vec3f cam_up = CreateVec3f(0.0f, 1.0f, 0.0f);
		origin += cam_up * fly_speed * (float)delta_time;
		frame_count = 0;
	}
	if (keyboard_state[SDL_SCANCODE_LSHIFT])
	{
		Vec3f cam_up = CreateVec3f(0.0f, 1.0f, 0.0f);
		origin += -cam_up * fly_speed * (float)delta_time;
		frame_count = 0;
	}
}
