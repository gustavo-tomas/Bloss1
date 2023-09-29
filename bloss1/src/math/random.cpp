#include "math/random.hpp"

namespace bls
{
    Random::Random()
    {
        random_engine.seed(std::random_device()());
    }

    Random::~Random()
    {

    }

    f32 Random::get_float(f32 begin, f32 end)
    {
        assert(begin <= end);

        // Number is a value between 0 and 1
        f32 number = static_cast<f32>(distribution(random_engine)) / static_cast<f32>(std::numeric_limits<uint_fast32_t>::max());
        return begin + (end - begin) * number;
    }

    i32 Random::get_int(i32 begin, i32 end)
    {
        return static_cast<i32>(get_float(static_cast<f32>(begin), static_cast<f32>(end)));
    }
};
