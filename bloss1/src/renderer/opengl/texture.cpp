#include "renderer/opengl/texture.hpp"

#include <GL/glew.h> // Include glew before glfw
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <assimp/material.h>
#include <filesystem> // @TODO: temporary (use a filesystem pls tyty)

namespace bls
{
    static aiTextureType convert_to_assimp_texture(TextureType type)
    {
        switch (type)
        {
            case TextureType::Diffuse:          return aiTextureType_DIFFUSE;
            case TextureType::Normal:           return aiTextureType_NORMALS;
            case TextureType::Specular:         return aiTextureType_SPECULAR;
            case TextureType::Metalness:        return aiTextureType_METALNESS;
            case TextureType::Roughness:        return aiTextureType_DIFFUSE_ROUGHNESS;
            case TextureType::AmbientOcclusion: return aiTextureType_AMBIENT_OCCLUSION;

            default: std::cerr << "unknown texture type\n"; exit(1);
        }

        return aiTextureType_UNKNOWN;
    }

    OpenGLTexture::OpenGLTexture(const str& path, TextureType texture_type)
    {
        stbi_set_flip_vertically_on_load(true);

        // @TODO: use a filesystem
        const str extension = std::filesystem::path(path).extension();

        if (extension == ".hdr")
        {
            f32* data = stbi_loadf(path.c_str(), &width, &height, &num_components, 0);
            if (data)
            {
                glGenTextures(1, &texture_id);
                glBindTexture(GL_TEXTURE_2D, texture_id);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
            }

            else
            {
                std::cerr << "failed to load texture: '" << path << "'\n";
                stbi_image_free(data);
                exit(1);
            }
        }

        else
        {
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);

            if (data)
            {
                // Resolve format
                GLenum format;
                if      (num_components == 1) { format = GL_RED; }
                else if (num_components == 3) { format = GL_RGB; }
                else if (num_components == 4) { format = GL_RGBA; }
                else format = GL_RGB;

                // Resolve internal format (sRGB or linear space)
                GLint internalFormat;
                if      (format == GL_RGB  && convert_to_assimp_texture(texture_type) == aiTextureType_DIFFUSE) { internalFormat = GL_SRGB; }
                else if (format == GL_RGBA && convert_to_assimp_texture(texture_type) == aiTextureType_DIFFUSE) { internalFormat = GL_SRGB_ALPHA; }
                else internalFormat = format;

                // Create texture
                glGenTextures(1, &texture_id);
                glBindTexture(GL_TEXTURE_2D, texture_id);
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
            }

            else
            {
                std::cerr << "failed to load texture: '" << path << "'\n";

                const char* failure_reason = stbi_failure_reason();
                if (failure_reason)
                {
                    std::cerr << "failure reason: " << failure_reason << "\n";
                    stbi__err(0, 0);
                }

                stbi_image_free(data);
                exit(1);
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
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture_id);
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
