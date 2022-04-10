#include "pch.h"
#include "random.h"
#include "tracer.h"

using namespace std;

TEST(tracer_test_case, test_correct_create_orthonormal_basis) {
    Vec w(1.0, 0.0, 0.0);
    auto [u, v] = create_orthonormal_basis(w);

    double result = dot(w, u);
    ASSERT_EQ(result, 0.0);

    result = dot(w, v);
    ASSERT_EQ(result, 0.0);
}

TEST(tracer_test_case, test_correct_sample_from_hemisphere) {
    Vec w(1.0, 0.0, 0.0);
    auto [u, v] = create_orthonormal_basis(w);

    UniformRealGenerator rnd;

    Vec result = sample_from_hemisphere(u, v, w, rnd, 1.0);

    ASSERT_GE(result.x, -1.0);
    ASSERT_LE(result.x, 1.0);
    ASSERT_GE(result.y, -1.0);
    ASSERT_LE(result.y, 1.0);
    ASSERT_GE(result.z, -1.0);
    ASSERT_LE(result.z, 1.0);
}
