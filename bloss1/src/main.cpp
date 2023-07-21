#include "core/game.hpp"

using namespace bls;

int main()
{
    Game game = Game("Bloss1", 1024, 600);
    game.run();

    return 0;
}
