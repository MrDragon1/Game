#pragma once

#include "Defines.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "math/Math.h"

#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Utils {
	unsigned int LoadTexture( const string& path, bool gamma = false);
}

