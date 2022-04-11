#include "pch.h"

#include <corecrt_math_defines.h>

#include "../render/utility.h"
#include "../render/vec.h"

double a{ 1.5 };
Vec v1{ 1.0, 2.0, 3.0 };
Vec v2{ 4.0, 5.0, 6.0 };


TEST(vec_test_case, test_correct_vec_operators) {
    ASSERT_EQ(v1, v1);

    Vec result;
    result = v1 + v2;
    ASSERT_EQ(result, Vec(5.0, 7.0, 9.0));

    result = v1 - v2;
    ASSERT_EQ(result, Vec(-3.0, -3.0, -3.0));

    result = v1 * a;
    ASSERT_EQ(result, Vec(1.5, 3.0, 4.5));

    result = v1 / a;
    ASSERT_EQ(result.x, 1.0 / 1.5);
    ASSERT_EQ(result.y, 2.0 / 1.5);
    ASSERT_EQ(result.z, 3.0 / 1.5);

    result = Vec(1.0, 1.0, 1.0);
    result += v2;
    ASSERT_EQ(result, Vec(5.0, 6.0, 7.0));

    result = Vec(1.0, 1.0, 1.0);
    result -= v2;
    ASSERT_EQ(result, Vec(-3.0, -4.0, -5.0));

    result = Vec(1.0, 1.0, 1.0);
    result *= 2.0;
    ASSERT_EQ(result, Vec(2.0, 2.0, 2.0));

    result = Vec(1.0, 1.0, 1.0);
    result /= 2.0;
    ASSERT_EQ(result, Vec(0.5, 0.5, 0.5));

    result = Vec(2.0, 0.0, 0.0);
    result.normalize();
    ASSERT_EQ(result, Vec(1.0, 0.0, 0.0));
}

TEST(vec_test_case, test_correct_multiplyer)
{
    ASSERT_EQ(a * v1, Vec(1.5, 3.0, 4.5));
}

TEST(vec_test_case, test_correct_dot)
{
    ASSERT_EQ(dot(v1, v2), 32.0);
}

TEST(vec_test_case, test_correct_norm)
{
    const Vec v(1.0, 0.0, 0.0);
    ASSERT_EQ(norm(v), 1.0);
}

TEST(vec_test_case, test_correct_normalize)
{
    ASSERT_EQ(norm(normalize(v1)), 1.0);
}

TEST(vec_test_case, test_correct_multiply)
{
    ASSERT_EQ(multiply(v1, v2), Vec(4.0, 10.0, 18.0));
}

TEST(vec_test_case, test_correct_cross)
{
    const Vec a_vec(1.0, 0.0, 0.0), b(0.0, 1.0, 0.0);
    ASSERT_EQ(cross(a_vec, b), Vec(0.0, 0.0, 1.0));
}

TEST(vec_test_case, test_correct_spherical_coordinate_vec)
{
    Vec result = spherical_coordinate_vec(0.0, 0.0);
    ASSERT_EQ(result, Vec(0.0, 1.0, 0.0));
    result = spherical_coordinate_vec(M_PI / 2, 0.0);
    ASSERT_EQ(result, Vec(0.0, 0.0, 1.0));
    result = spherical_coordinate_vec(M_PI / 2, M_PI / 2);
    ASSERT_EQ(result, Vec(1.0, 0.0, 0.0));
    result = spherical_coordinate_vec(M_PI / 2, M_PI);
    ASSERT_EQ(result, Vec(0.0, 0.0, -1.0));
    result = spherical_coordinate_vec(M_PI / 2, M_PI * 3 / 2);
    ASSERT_EQ(result, Vec(-1.0, 0.0, 0.0));
    result = spherical_coordinate_vec(M_PI / 2, 2 * M_PI);
    ASSERT_EQ(result, Vec(0.0, 0.0, 1.0));
    result = spherical_coordinate_vec(M_PI, 0.0);
    ASSERT_EQ(result, Vec(0.0, -1.0, 0.0));
}
