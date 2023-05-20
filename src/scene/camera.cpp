#include "camera.hpp"
#include <glad/glad.h>
#include <SDL.h>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

Camera::Camera(glm::vec3 orig, glm::vec3 fwd, glm::vec3 r, float speed, float sens)
        : origin(orig), forward(fwd), right(r), fly_speed(speed), look_sens(sens), xpos(-90.0f), ypos(0.0f)
{
    glCreateBuffers(1, &cam_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, cam_ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, cam_ubo);
    glNamedBufferStorage(cam_ubo, sizeof(CameraGLSL), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::mouse_look(float xrel, float yrel)
{
    xpos += xrel * look_sens;
    ypos -= yrel * look_sens;

    if (ypos > 89.5f)
    {
        ypos = 89.5f;
    }
    if (ypos < -89.5f)
    {
        ypos = -89.5f;
    }

    forward.x = cosf(glm::radians(xpos)) * cosf(glm::radians(ypos));
    forward.y = sinf(glm::radians(ypos));
    forward.z = sinf(glm::radians(xpos)) * cosf(glm::radians(ypos));
    forward = glm::normalize(forward);

    right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
}

void Camera::move(const uint8 *keyboard_state, uint32 delta_time, uint32 &frame_count)
{
    if (keyboard_state[SDL_SCANCODE_W])
    {
        origin += forward * fly_speed * (float) delta_time;
        frame_count = 0;
    }
    if (keyboard_state[SDL_SCANCODE_S])
    {
        origin += -forward * fly_speed * (float) delta_time;
        frame_count = 0;
    }
    if (keyboard_state[SDL_SCANCODE_A])
    {
        origin += -right * fly_speed * (float) delta_time;
        frame_count = 0;
    }
    if (keyboard_state[SDL_SCANCODE_D])
    {
        origin += right * fly_speed * (float) delta_time;
        frame_count = 0;
    }
    if (keyboard_state[SDL_SCANCODE_SPACE])
    {
		glm::vec3 cam_up(0.0f, 1.0f, 0.0f);
        origin += cam_up * fly_speed * (float) delta_time;
        frame_count = 0;
    }
    if (keyboard_state[SDL_SCANCODE_LSHIFT])
    {
		glm::vec3 cam_up(0.0f, 1.0f, 0.0f);
        origin += -cam_up * fly_speed * (float) delta_time;
        frame_count = 0;
    }
}

CameraGLSL::CameraGLSL(const glm::vec3 &origin, const glm::vec3 &forward, const glm::vec3 &right, float speed, float sens)
{
    data1 = glm::vec4(origin.x, origin.y, origin.z, speed);
    data2 = glm::vec4(forward.x, forward.y, forward.z, sens);
    data3 = glm::vec4(right.x, right.y, right.z, 0.0f);
}
