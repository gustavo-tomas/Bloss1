#include "stages/menu_stage.hpp"
#include "stages/test_stage.hpp"
#include "core/game.hpp"
#include "core/input.hpp"

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

        // Load audios
        auto& audio_engine = Game::get().get_audio_engine();
        audio_engine.load("test", "bloss1/assets/sounds/toc.wav", false);
        audio_engine.set_echo_filter("test", 0.2f, 0.15f);
        audio_engine.play("test");
    }

    void MenuStage::update(f32)
    {
        if (Input::is_key_pressed(KEY_SPACE))
            Game::get().change_stage(new TestStage());
    }
};
