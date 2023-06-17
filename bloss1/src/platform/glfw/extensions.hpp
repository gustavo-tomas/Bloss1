#pragma once

/**
 * @brief Glew extensions. Mostly for debugging opengl.
 */

#include <GLFW/glfw3.h>

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*);
