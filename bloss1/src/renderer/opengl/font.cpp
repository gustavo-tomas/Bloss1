#include "renderer/opengl/font.hpp"

#include "core/game.hpp"
#include "core/logger.hpp"

// Free type for loading ttf
#include <freetype2/freetype/freetype.h>
#include FT_FREETYPE_H

#include <GL/glew.h>  // Include glew before glfw

#include "GLFW/glfw3.h"

namespace bls
{
    OpenGLFont::OpenGLFont(const str &path)
    {
        auto width = Game::get().get_window().get_width();
        auto height = Game::get().get_window().get_height();

        mat4 projection = ortho(0.0f, static_cast<f32>(width), 0.0f, static_cast<f32>(height));

        // Configure texture quad
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Configure font shader
        shader = Shader::create("font_shader", "bloss1/assets/shaders/font.vs", "bloss1/assets/shaders/font.fs");
        shader->bind();
        shader->set_uniform4("projection", projection);

        // Initialize free type
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
            throw std::runtime_error(
                "failed to initialize "
                "FreeType");

        // Load font as face
        FT_Face face;
        if (FT_New_Face(ft, path.c_str(), 0, &face)) throw std::runtime_error("failed to load font: '" + path + "'");

        // Set glyph size
        FT_Set_Pixel_Sizes(face, 0, 80);

        // Disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Load ASCII characters
        for (u64 c = 0; c < 256; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                LOG_ERROR("failed to load glyph '%c'", static_cast<char>(c));
                continue;
            }

            // Generate texture
            u32 texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RED,
                         face->glyph->bitmap.width,
                         face->glyph->bitmap.rows,
                         0,
                         GL_RED,
                         GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer);

            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Create character
            Character character = {texture,
                                   vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                   vec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                   static_cast<u32>(face->glyph->advance.x)};

            characters[c] = character;
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        // Destroy free type object
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    OpenGLFont::~OpenGLFont()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        for (auto &[ch, Ch] : characters) glDeleteTextures(1, &Ch.texture_id);
    }

    void OpenGLFont::render(str text, f32 x, f32 y, f32 scale, vec3 color)
    {
        // Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader->bind();
        shader->set_uniform3("textColor", color);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        // Iterate through all characters
        for (auto c = text.begin(); c != text.end(); c++)
        {
            Character ch = characters[*c];

            f32 xpos = x + ch.bearing.x * scale;
            f32 ypos = y - (ch.size.y - ch.bearing.y) * scale;

            f32 w = ch.size.x * scale;
            f32 h = ch.size.y * scale;

            // Update VBO for each character
            f32 vertices[6][4] = {{xpos, ypos + h, 0.0f, 0.0f},
                                  {xpos, ypos, 0.0f, 1.0f},
                                  {xpos + w, ypos, 1.0f, 1.0f},

                                  {xpos, ypos + h, 0.0f, 0.0f},
                                  {xpos + w, ypos, 1.0f, 1.0f},
                                  {xpos + w, ypos + h, 1.0f, 0.0f}};

            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.texture_id);

            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(
                GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);  // Use glBufferSubData and not glBufferData
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale;  // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th
                                             // pixels by 64 to get amount of pixels))
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Disable blending
        glDisable(GL_BLEND);
    }
};  // namespace bls
