#include "core/game.hpp"

using namespace bls;

int main()
{
    try
    {
        Game game = Game("Bloss1", 1920, 1080);
        game.run();
    }

    catch (std::exception &e)
    {
        std::cerr << "runtime error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
