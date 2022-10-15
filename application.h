//
// Created by Hongjian Zhu on 2022/10/8.
//

#ifndef POKEMOON_APPLICATION_H
#define POKEMOON_APPLICATION_H

#include "defines.h"

struct ApplicationConfig {
  CString name;
  u16     width;
  u16     height;
};

void application_create(const ApplicationConfig &config);

void application_run();

void application_get_framebuffer_size(u32 &width, u32 &height);

#endif // POKEMOON_APPLICATION_H
