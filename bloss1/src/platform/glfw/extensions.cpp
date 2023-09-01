#include "platform/glfw/extensions.hpp"
#include "core/core.hpp"

void APIENTRY debug_callback(GLenum source, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*)
{
    if (source == GL_DEBUG_SOURCE_OTHER_ARB || type == GL_DEBUG_TYPE_OTHER_ARB) { return; } // Hide 'Other'
    std::cout << "\n--- OpenGL Debug Output message ---\n";

    if (source == GL_DEBUG_SOURCE_API_ARB)					std::cout << "Source: API;\n";
    else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)	std::cout << "Source: WINDOW_SYSTEM;\n";
    else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)	std::cout << "Source: SHADER_COMPILER;\n";
    else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)		std::cout << "Source: THIRD_PARTY;\n";
    else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB)		std::cout << "Source: APPLICATION;\n";
    else if (source == GL_DEBUG_SOURCE_OTHER_ARB)			return; // Hide 'Other'

    if (type == GL_DEBUG_TYPE_ERROR_ARB)				    std::cout << "Type: ERROR;\n";
    else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)	std::cout << "Type: DEPRECATED_BEHAVIOR;\n";
    else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)	std::cout << "Type: UNDEFINED_BEHAVIOR;\n";
    else if (type == GL_DEBUG_TYPE_PORTABILITY_ARB)			std::cout << "Type: PORTABILITY;\n";
    else if (type == GL_DEBUG_TYPE_PERFORMANCE_ARB)			std::cout << "Type: PERFORMANCE;\n";
    else if (type == GL_DEBUG_TYPE_OTHER_ARB)				return; // Hide 'Other'

    if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)				std::cout << "Severity: HIGH;\n";
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)		std::cout << "Severity: MEDIUM;\n";
    else if (severity == GL_DEBUG_SEVERITY_LOW_ARB)			std::cout << "Severity: LOW;\n";

    std::cout << "Message: " << message << "\n";
    // std::cout << "ID: " << id << "\n";
    // std::cout << "Length: " << length << "\n";
    // std::cout << "User param: " << userParam << "\n";
    std::cout << "--- OpenGL Debug Output message ---\n\n";
}
