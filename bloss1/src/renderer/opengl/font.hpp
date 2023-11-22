#pragma once

/**
 * @brief The font implementation for OpenGL.
 */

#include "renderer/font.hpp"

namespace bls
{
    class OpenGLFont : public Font
    {
        public:
            OpenGLFont(const str &path);
            ~OpenGLFont();

            void render(str text, f32 x, f32 y, f32 scale, vec3 color) override;

        private:
            struct Character
            {
                    u32 texture_id;  // ID handle of the glyph texture
                    vec2 size;       // Size of glyph
                    vec2 bearing;    // Offset from baseline to left/top of glyph
                    u32 advance;     // Offset to advance to next glyph
            };

            std::map<char, Character> characters;
            std::shared_ptr<Shader> shader;
            u32 VAO, VBO;
    };
};  // namespace bls
