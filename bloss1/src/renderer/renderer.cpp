#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/opengl/renderer.hpp"
#include "renderer/opengl/shader.hpp"

namespace bls
{
    Renderer* Renderer::create()
    {
        #ifdef _OPENGL
        return new OpenGLRenderer();
        #else
        std::cerr << "no valid renderer defined\n";
        exit(1);
        #endif
    }
};
