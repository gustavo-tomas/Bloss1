#include "managers/font_manager.hpp"

namespace bls
{
    void FontManager::load(const str &name, std::shared_ptr<Font> font)
    {
        fonts[name] = font;
    }

    std::shared_ptr<Font> FontManager::get_font(const str &name)
    {
        if (exists(name))
            return fonts[name];

        else
            throw std::runtime_error("font '" + name + "' doesn't exist");
    }

    bool FontManager::exists(const str &name)
    {
        return fonts.count(name) > 0;
    }

    FontManager &FontManager::get()
    {
        static FontManager instance;
        return instance;
    }
};  // namespace bls
