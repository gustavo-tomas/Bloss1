#include "stages/menu_stage.hpp"

#include "core/game.hpp"
#include "core/input.hpp"
#include "stages/test_stage.hpp"

namespace bls
{
    MenuStage::MenuStage()
    {
    }

    MenuStage::~MenuStage()
    {
    }

    void MenuStage::start()
    {
        // Create a video player
        video_player = std::make_unique<VideoPlayer>("bloss1/assets/videos/mh_pro_skate.mp4");
        video_player->play_video();
    }

    void MenuStage::update(f32)
    {
        if (Input::is_key_pressed(KEY_SPACE)) Game::get().change_stage(new TestStage());
    }
};  // namespace bls
