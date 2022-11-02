//
// Created by Hongjian Zhu on 2022/10/23.
//

#pragma once

#include <glm/glm.hpp>

#define VEC3_ONE                                                                                   \
  { 1.0f, 1.0f, 1.0f }

#define VEC3_ZERO                                                                                  \
  { 0.0f, 0.0f, 0.0f }

#define VEC4_ZERO                                                                                  \
  { 0.0f, 0.0f, 0.0f, 0.0f }

#define VEC4_ONE                                                                                   \
  { 1.0f, 1.0f, 1.0f, 1.0f }

struct Vertex3D {
  glm::vec3 position;
  glm::vec3 color;
};
