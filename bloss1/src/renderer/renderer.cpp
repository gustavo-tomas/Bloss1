#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/opengl/renderer.hpp"
#include "renderer/opengl/shader.hpp"
#include "renderer/opengl/buffers.hpp"
#include "renderer/opengl/texture.hpp"
#include "managers/shader_manager.hpp"
#include "managers/texture_manager.hpp"

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

    // @TODO: doesnt need to be this way (same for the texture manager)
    std::shared_ptr<Shader> Shader::create(const str& name, const str& vertex_path, const str& fragment_path, const str& geometry_path)
    {
        #ifdef _OPENGL
        if (ShaderManager::get().exists(name))
            return ShaderManager::get().get_shader(name);

        else
            return ShaderManager::get().load(name, vertex_path, fragment_path, geometry_path);
        #else
        return nullptr;
        #endif
    }

    std::shared_ptr<Texture> Texture::create(const str& name, const str& path, TextureType texture_type)
    {
        #ifdef _OPENGL
        if (TextureManager::get().exists(name))
            return TextureManager::get().get_texture(name);

        else
            return TextureManager::get().load(name, path, texture_type);
        #else
        return nullptr;
        #endif
    }

    VertexBuffer* VertexBuffer::create(f32* vertices, u32 size)
    {
        #ifdef _OPENGL
        return new OpenGLVertexBuffer(vertices, size);
        #else
        return nullptr;
        #endif
    }

    IndexBuffer* IndexBuffer::create(u32* indices, u32 count)
    {
        #ifdef _OPENGL
        return new OpenGLIndexBuffer(indices, count);
        #else
        return nullptr;
        #endif
    }

    VertexArray* VertexArray::create()
    {
        #ifdef _OPENGL
        return new OpenGLVertexArray();
        #else
        return nullptr;
        #endif
    }
};
