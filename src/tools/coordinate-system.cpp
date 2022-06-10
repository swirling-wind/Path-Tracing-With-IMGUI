#
#include "coordinate-system.h"

#include <cmath>

#include "constexpr-math.h"

glm::dmat3 orthonormal_basis(const glm::dvec3& n)
{
    double sign = std::copysign(1.0, n.z);
    const double a = -1.0 / (sign + n.z);
    double b = n.x * n.y * a;

    return
    {
        { 1.0 + sign * n.x * n.x * a, sign * b, -sign * n.x },
        { b, sign + n.y * n.y * a, -n.y },
        n
    };
}

CoordinateSystem::CoordinateSystem(const glm::dvec3& n) : t_(orthonormal_basis(n)) { }

glm::dvec3 CoordinateSystem::from(const glm::dvec3& v) const
{
    return t_ * v;
}

glm::dvec3 CoordinateSystem::to(const glm::dvec3& v) const
{
    return transpose(t_) * v;
}

const glm::dvec3& CoordinateSystem::normal() const
{
    return t_[2];
}

glm::dvec3 CoordinateSystem::from(const glm::dvec3& v, const glm::dvec3& N)
{
    return orthonormal_basis(N) * v;
}
