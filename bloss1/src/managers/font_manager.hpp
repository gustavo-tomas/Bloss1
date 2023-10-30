#pragma once

/**
 * @brief Creates, loads and delete fonts.
 */

#include "renderer/font.hpp"

namespace bls
{
    class FontManager
    {
        public:
            void load(const str &name, std::shared_ptr<Font> font);
            std::shared_ptr<Font> get_font(const str &name);
            bool exists(const str &name);

            static FontManager &get();

        private:
            FontManager(){};
            ~FontManager(){};

            std::map<str, std::shared_ptr<Font>> fonts;
    };
};  // namespace bls
