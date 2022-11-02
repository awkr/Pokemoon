//
// Created by Hongjian Zhu on 2022/10/22.
//

#pragma once

#include "renderer/types.h"

bool object_shader_create(Context *context, ObjectShader *out);

void object_shader_destroy(Context *context, ObjectShader *shader);

void object_shader_use(Context *context, ObjectShader *shader);

void object_shader_update_global_state(Context *context, ObjectShader *shader);
