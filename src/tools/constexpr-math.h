#pragma once

#include <glm/matrix.hpp>

namespace Constant
{
    inline constexpr double EPSILON = 1e-9;
    inline constexpr double PI = 3.14159265358979323846;
    inline constexpr double INV_PI = 0.31830988618379067154;
    inline constexpr double HALF_PI = 1.57079632679489661923;
    inline constexpr double TWO_PI = 6.283185307179586476925;
}

constexpr glm::dvec3 const_multiply(const glm::dvec3& v, const double a)
{
    return { v[0] * a, v[1] * a, v[2] * a };
}

constexpr double const_floor(double x)
{
    return static_cast<double>(static_cast<intmax_t>(x));
}

template <class T>
constexpr T const_max(T x, T y)
{
    return x > y ? x : y;
}

template <class T>
constexpr T pow2(T x)
{
    return x * x;
}

template <class T>
constexpr size_t next_power_of_two(T i)
{
    // Equivalent to 2^ceil(log2(i))
    return i == 0 ? 0 : 1ull << (sizeof(T) * 8 - std::countl_zero(i - 1));
}

constexpr double determinant(const glm::dmat3& matrix)
{
    return
        matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[2][1] * matrix[1][2]) -
        matrix[1][0] * (matrix[0][1] * matrix[2][2] - matrix[2][1] * matrix[0][2]) +
        matrix[2][0] * (matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2]);
}

constexpr glm::dmat3 inverse(const glm::dmat3& matrix)
{
    double inv_det = 1.0 / determinant(matrix);

    return
    {
        {
            inv_det * (matrix[1][1] * matrix[2][2] - matrix[2][1] * matrix[1][2]),
            inv_det * (matrix[2][1] * matrix[0][2] - matrix[0][1] * matrix[2][2]),
            inv_det * (matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2])
        },
        {
            inv_det * (matrix[2][0] * matrix[1][2] - matrix[1][0] * matrix[2][2]),
            inv_det * (matrix[0][0] * matrix[2][2] - matrix[2][0] * matrix[0][2]),
            inv_det * (matrix[1][0] * matrix[0][2] - matrix[0][0] * matrix[1][2])
        },
        {
            inv_det * (matrix[1][0] * matrix[2][1] - matrix[2][0] * matrix[1][1]),
            inv_det * (matrix[2][0] * matrix[0][1] - matrix[0][0] * matrix[2][1]),
            inv_det * (matrix[0][0] * matrix[1][1] - matrix[1][0] * matrix[0][1])
        }
    };
}

constexpr glm::dvec3 const_multiply(const glm::dmat3& matrix, const glm::dvec3 v)
{
    return
    {
        matrix[0][0] * v[0] + matrix[1][0] * v[1] + matrix[2][0] * v[2],
        matrix[0][1] * v[0] + matrix[1][1] * v[1] + matrix[2][1] * v[2],
        matrix[0][2] * v[0] + matrix[1][2] * v[1] + matrix[2][2] * v[2]
    };
}


