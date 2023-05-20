#pragma once
#include "../defines.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Camera
{
	glm::vec3 origin;
	glm::vec3 forward;
	glm::vec3 right;
    float fly_speed;
    float look_sens;

    float xpos;
    float ypos;

    uint32 cam_ubo {};

    Camera(glm::vec3 orig, glm::vec3 fwd, glm::vec3 r, float speed, float sens);

    void mouse_look(float xrel, float yrel);

    void move(const uint8 *keyboard_state, uint32 delta_time, uint32 &frame_count);
};

struct CameraGLSL
{
	glm::vec4 data1 {}; // o.x, o.y, o.z, speed
	glm::vec4 data2 {}; // f.x, f.y, f.z, sens
	glm::vec4 data3 {}; // r.x, r.y, r.z, 0

    CameraGLSL() = default;

    CameraGLSL(const glm::vec3 &origin,
			   const glm::vec3 &forward,
			   const glm::vec3 &right,
               float speed,
               float sens);
};

enum struct ViewMode
{
    BRDF_IMPORTANCE_SAMPLING = 1,
    NEXT_EVENT_ESTIMATION,
    MULTIPLE_IMPORTANCE_SAMPLING_BRDF_NEE
};
