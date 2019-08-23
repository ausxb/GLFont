#pragma once
#include <iostream>

#include "FontManager.h"
#include "TextEngine.h"

#include "FreeImage.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Sourcer.h"
#include "Shader.h"
#include "Program.h"

BYTE* load_image(const char *path);

void unload_image(BYTE *bitmap);

void store_image(const char *path, BYTE *raw, FREE_IMAGE_FORMAT fmt, int width, int height, int pitch, unsigned int bpp);

void error_callback(int error, const char* description);