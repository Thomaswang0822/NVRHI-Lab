#pragma once

// Left-handed coordinate system: used by D3D and Vulkan (positive Z points into the screen)
#define GLM_FORCE_LEFT_HANDED
// Depth range [0, 1]: D3D and Vulkan use 0 at near plane, 1 at far plane (OpenGL uses [-1, 1])
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
