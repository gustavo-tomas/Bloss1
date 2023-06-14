#include "core/game.hpp"

using namespace bls;

int main()
{
    Game* game = new Game("Bloss1", 1024, 600);
    game->run();
    delete game;

    return 0;
}
