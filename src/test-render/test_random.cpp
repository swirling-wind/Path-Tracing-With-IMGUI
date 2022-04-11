#include "pch.h"

#include "../render/random.h"
using namespace instant_renderer;

TEST(uniform_real_generator_test_case, test_correct_rand)
{
    UniformRealGenerator rnd;

    int N = 1000;
    double result;

    for (int i = 0; i < N; i++) {
        result = rnd();
        ASSERT_GT(result, 0.0);
        ASSERT_LT(result, 1.0);
    }

    rnd = UniformRealGenerator(0, 100.0, 200.0);
    for (int i = 0; i < N; i++) {
        result = rnd();
        ASSERT_GT(result, 100.0);
        ASSERT_LT(result, 200.0);
    }
}
