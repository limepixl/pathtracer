#pragma once
#include "../math/vec.hpp"

struct Camera
{
  Vec3f origin;
  Vec3f forward;
  Vec3f right;
  float fly_speed;
  float look_sens;

  float xpos;
  float ypos;

  uint32 cam_ubo;

  Camera(Vec3f orig, Vec3f fwd, Vec3f r, float fly_speed, float look_sens);

  void mouse_look(float xrel, float yrel);

  void move(const uint8 *keyboard_state, uint32 delta_time, uint32 &frame_count);
};

struct CameraGLSL
{
  Vec4f data1; // o.x, o.y, o.z, speed
  Vec4f data2; // f.x, f.y, f.z, sens
  Vec4f data3; // r.x, r.y, r.z, 0

  CameraGLSL() = default;

  CameraGLSL(const Vec3f &origin,
             const Vec3f &forward,
             const Vec3f &right,
             float speed,
             float sens);
};
