#include "core/game.hpp"

using namespace bls;

int main()
{
    game* gm = create_game("Bloss1");
    update_game(*gm);
    destroy_game(*gm);

    return 0;
}
