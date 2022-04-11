#include "../render/vec.h"
#include <ostream>
using namespace instant_renderer;

namespace instant_renderer
{
    Vec::Vec(const double x, const double y, const double z) : x(x), y(y), z(z) {}

    void Vec::normalize()
    {
        double l = sqrt(x * x + y * y + z * z);
        x /= l;
        y /= l;
        z /= l;
    }

    Vec spherical_coordinate_vec(const double theta, const double phi)
    {
        double x = std::sin(theta) * std::sin(phi);
        double y = std::cos(theta);
        double z = std::sin(theta) * std::cos(phi);
        if (x == -0.0) x = 0.0;  // NOLINT(clang-diagnostic-float-equal)
        if (z == -0.0) z = 0.0;  // NOLINT(clang-diagnostic-float-equal)
        return Vec(x, y, z);
    }

    std::ostream& operator<<(std::ostream& os, const Vec& v)
    {
        os << "Vec(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
}
