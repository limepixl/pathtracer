#include "camera.hpp"
#include "../defines.hpp"
#include "../math/math.hpp"
#include <SDL.h>
#include <glad/glad.h>
#include <cstdio>
#include <cmath>

Camera::Camera(Vec3f orig, Vec3f fwd, Vec3f r, float speed, float sens)
	: origin(orig), forward(fwd), right(r), fly_speed(speed), look_sens(sens)
{
	xpos = -90.0f;
	ypos = 0.0f;

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

	if (ypos > 89.5f)
		ypos = 89.5f;
	if (ypos < -89.5f)
		ypos = -89.5f;

	forward.x = cosf(Radians(xpos)) * cosf(Radians(ypos));
	forward.y = sinf(Radians(ypos));
	forward.z = sinf(Radians(xpos)) * cosf(Radians(ypos));
	forward = NormalizeVec3f(forward);

	right = NormalizeVec3f(Cross(forward, Vec3f(0.0f, 1.0f, 0.0f)));
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
		Vec3f cam_up(0.0f, 1.0f, 0.0f);
		origin += cam_up * fly_speed * (float)delta_time;
		frame_count = 0;
	}
	if (keyboard_state[SDL_SCANCODE_LSHIFT])
	{
		Vec3f cam_up(0.0f, 1.0f, 0.0f);
		origin += -cam_up * fly_speed * (float)delta_time;
		frame_count = 0;
	}
}
CameraGLSL::CameraGLSL(const Vec3f &origin, const Vec3f &forward, const Vec3f &right, float speed, float sens)
{
	data1 = Vec4f(origin.x, origin.y, origin.z, speed);
	data2 = Vec4f(forward.x, forward.y, forward.z, sens);
	data3 = Vec4f(right.x, right.y, right.z, 0.0f);
}
