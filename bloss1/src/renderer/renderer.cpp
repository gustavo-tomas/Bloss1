#include "renderer/renderer.hpp"
#include "renderer/model.hpp"
#include "renderer/skybox.hpp"
#include "renderer/font.hpp"
#include "renderer/opengl/renderer.hpp"
#include "renderer/opengl/shader.hpp"
#include "renderer/opengl/buffers.hpp"
#include "renderer/opengl/texture.hpp"
#include "renderer/opengl/skybox.hpp"
#include "renderer/opengl/font.hpp"
#include "managers/shader_manager.hpp"
#include "managers/texture_manager.hpp"
#include "managers/model_manager.hpp"
#include "managers/font_manager.hpp"

namespace bls
{
    Renderer* Renderer::create()
    {
        #ifdef _OPENGL
        return new OpenGLRenderer();
        #else
        throw std::runtime_error("no valid renderer defined");
        #endif
    }

    Skybox* Skybox::create(const str& path,
                           const u32 skybox_resolution,
                           const u32 irradiance_resolution,
                           const u32 brdf_resolution,
                           const u32 prefilter_resolution,
                           const u32 max_mip_levels)
    {
        #ifdef _OPENGL
        return new OpenGLSkybox(path, skybox_resolution, irradiance_resolution, brdf_resolution, prefilter_resolution, max_mip_levels);
        #else
        return nullptr;
        #endif
    }

    std::shared_ptr<Shader> Shader::create(const str& name, const str& vertex_path, const str& fragment_path, const str& geometry_path)
    {
        if (ShaderManager::get().exists(name))
            return ShaderManager::get().get_shader(name);

        #ifdef _OPENGL
        auto shader = std::make_shared<OpenGLShader>(vertex_path, fragment_path, geometry_path);
        ShaderManager::get().load(name, shader);
        return shader;
        #else
        return nullptr;
        #endif
    }

    // @TODO: there must be a better way
    std::shared_ptr<Model> Model::create(const str& name, const str& path, bool flip_uvs)
    {
        if (ModelManager::get().exists(name))
            return ModelManager::get().get_model(name);

        auto model = std::make_shared<Model>(path, flip_uvs);
        ModelManager::get().load(name, model);
        return model;
    }

    std::shared_ptr<Texture> Texture::create(u32 width, u32 height, ImageFormat format,
            TextureParameter wrap_s, TextureParameter wrap_t,
            TextureParameter min_filter, TextureParameter mag_filter)
    {
        #ifdef _OPENGL
        auto texture = std::make_shared<OpenGLTexture>(width, height, format, wrap_s, wrap_t, min_filter, mag_filter);
        return texture;
        #else
        return nullptr;
        #endif
    }

    std::shared_ptr<Texture> Texture::create(const str& name, const str& path, TextureType texture_type)
    {
        if (TextureManager::get().exists(name))
            return TextureManager::get().get_texture(name);

        #ifdef _OPENGL
        auto texture = std::make_shared<OpenGLTexture>(path, texture_type);
        TextureManager::get().load(name, texture);
        return texture;
        #else
        return nullptr;
        #endif
    }

    std::shared_ptr<Texture> Texture::get_default(TextureType texture_type)
    {
        str name = "";
        switch (texture_type)
        {
            case TextureType::Diffuse:
                name = "default_diffuse";
                break;

            case TextureType::Normal:
                name = "default_normal";
                break;

            case TextureType::Specular:
                name = "default_specular";
                break;

            case TextureType::Metalness:
                name = "default_arm";
                break;

            case TextureType::Roughness:
                name = "default_roughness";
                break;

            case TextureType::AmbientOcclusion:
                name = "default_ao";
                break;

            case TextureType::Emissive:
                name = "default_emissive";
                break;

            default:
                std::cerr << "invalid texture type for default texture\n";
                break;
        }

        return Texture::create(name, "bloss1/assets/textures/" + name + ".png", texture_type);
    }

    std::shared_ptr<Font> Font::create(const str& name, const str& path)
    {
        if (FontManager::get().exists(name))
            return FontManager::get().get_font(name);

        #ifdef _OPENGL
        auto font = std::make_shared<OpenGLFont>(path);
        FontManager::get().load(name, font);
        return font;
        #else
        return nullptr;
        #endif
    }

    VertexBuffer* VertexBuffer::create(void* vertices, u32 size)
    {
        #ifdef _OPENGL
        return new OpenGLVertexBuffer(vertices, size);
        #else
        return nullptr;
        #endif
    }

    IndexBuffer* IndexBuffer::create(const std::vector<u32>& indices, u32 count)
    {
        #ifdef _OPENGL
        return new OpenGLIndexBuffer(indices, count);
        #else
        return nullptr;
        #endif
    }

    FrameBuffer* FrameBuffer::create()
    {
        #ifdef _OPENGL
        return new OpenGLFrameBuffer();
        #else
        return nullptr;
        #endif
    }

    RenderBuffer* RenderBuffer::create(u32 width, u32 height, AttachmentType type)
    {
        #ifdef _OPENGL
        return new OpenGLRenderBuffer(width, height, type);
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
