#include "renderer/opengl/texture.hpp"
#include "math/math.hpp"

#include <GL/glew.h> // Include glew before glfw
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// @TODO: temporary (use a filesystem pls tyty)

namespace bls
{
    static GLenum convert_to_opengl_internal_format(ImageFormat format)
    {
        switch (format)
        {
            case ImageFormat::RGB8:  return GL_RGB8;
            case ImageFormat::RGBA8: return GL_RGBA8;
            case ImageFormat::RGB32F: return GL_RGB32F;
            case ImageFormat::RGBA32F: return GL_RGBA32F;
            default: throw std::runtime_error("invalid image format\n");
        }

        return 0;
    }

    static GLenum convert_to_opengl_parameter_format(TextureParameter parameter)
    {
        switch (parameter)
        {
            case TextureParameter::Repeat:  return GL_REPEAT;
            case TextureParameter::ClampToEdge: return GL_CLAMP_TO_EDGE;
            case TextureParameter::Nearest: return GL_NEAREST;
            case TextureParameter::Linear: return GL_LINEAR;
            default: throw std::runtime_error("invalid texture parameter\n");
        }

        return 0;
    }

    OpenGLTexture::OpenGLTexture(u32 width, u32 height, ImageFormat format,
                                 TextureParameter wrap_s, TextureParameter wrap_t,
                                 TextureParameter min_filter, TextureParameter mag_filter)
    {
        auto internal_format = convert_to_opengl_internal_format(format);
        auto internal_wrap_s = convert_to_opengl_parameter_format(wrap_s);
        auto internal_wrap_t = convert_to_opengl_parameter_format(wrap_t);
        auto internal_min_filter = convert_to_opengl_parameter_format(min_filter);
        auto internal_mag_filter = convert_to_opengl_parameter_format(mag_filter);

        // Create texture
        glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
        glTextureStorage2D(texture_id, 1, internal_format, width, height);

        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, internal_wrap_s);
        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, internal_wrap_t);
        glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, internal_min_filter);
        glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, internal_mag_filter);

        this->type = TextureType::None; // Type doesn't really matter for this kind of texture
        this->width = width;
        this->height = height;
    }

    OpenGLTexture::OpenGLTexture(const str& path, TextureType texture_type)
    {
        stbi_set_flip_vertically_on_load(true);

        // @TODO: use a filesystem (and make the other params configurable)
        const str extension = std::filesystem::path(path).extension();

        if (extension == ".hdr")
        {
            f32* data = stbi_loadf(path.c_str(), &width, &height, &num_components, 0);
            if (data)
            {
                glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
                glTextureStorage2D(texture_id, 1, GL_RGB32F, width, height);
                glTextureSubImage2D(texture_id, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);

                glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
            }

            else
            {
                stbi_image_free(data);
                throw std::runtime_error("failed to load texture: '" + path + "'");
            }
        }

        else
        {
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);

            if (data)
            {
                // Resolve format
                GLenum internal_format = 0, data_format = 0;
                if (num_components == 4)
                {
                    internal_format = (texture_type == TextureType::Diffuse || texture_type == TextureType::Emissive)
                                      ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                    data_format = GL_RGBA;
                }

                else if (num_components == 3)
                {
                    internal_format = (texture_type == TextureType::Diffuse || texture_type == TextureType::Emissive)
                                      ? GL_SRGB8 : GL_RGB8;
                    data_format = GL_RGB;
                }

                else
                    throw std::runtime_error("unsupported number of channels: '" + to_str(num_components) + "'");

                // Create texture
                glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
                glTextureStorage2D(texture_id, 1, internal_format, width, height);
                glTextureSubImage2D(texture_id, 0, 0, 0, width, height, data_format, GL_UNSIGNED_BYTE, data);

                glGenerateTextureMipmap(texture_id);
                glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
            }

            else
            {
                const char* failure_reason = stbi_failure_reason();
                if (failure_reason)
                {
                    std::cerr << "failure reason: " << failure_reason << "\n";
                    stbi__err(0, 0);
                }

                stbi_image_free(data);
                throw std::runtime_error("failed to load texture: '" + path + "'");
            }
        }

        this->path = path;
        this->type = texture_type;
    }

    OpenGLTexture::~OpenGLTexture()
    {
        glDeleteTextures(1, &texture_id);

        std::cout << "texture '" << texture_id << "' destroyed successfully\n";
    }

    void OpenGLTexture::bind(u32 slot)
    {
        glBindTextureUnit(slot, texture_id);
    }

    void OpenGLTexture::set_data(void* pixels)
    {
        glTextureSubImage2D(texture_id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }

    u32 OpenGLTexture::get_id()
    {
        return texture_id;
    }

    u32 OpenGLTexture::get_width()
    {
        return width;
    }

    u32 OpenGLTexture::get_height()
    {
        return height;
    }

    TextureType OpenGLTexture::get_type()
    {
        return type;
    }
};
