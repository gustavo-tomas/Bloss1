#include "core/game.hpp"

using namespace bls;

int main()
{
    try
    {
        Game game = Game("Bloss1", 1024, 600);
        game.run();
    }

    catch (std::exception& e)
    {
        std::cerr << "runtime error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
