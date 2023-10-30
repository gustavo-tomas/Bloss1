#pragma once

/**
 * @brief The interface for a font. Each renderer must implement the methods.
 */

#include "renderer/shader.hpp"

namespace bls
{
    class Font
    {
        public:
            virtual ~Font(){};

            virtual void render(str text, f32 x, f32 y, f32 scale, vec3 color) = 0;

            static std::shared_ptr<Font> create(const str &name, const str &path);
    };
};  // namespace bls
