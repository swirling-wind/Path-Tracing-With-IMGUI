#pragma once

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>

struct CoordinateSystem
{
    CoordinateSystem() : t_() { }
    explicit CoordinateSystem(const glm::dvec3& n);

    [[nodiscard]] glm::dvec3 from(const glm::dvec3& v) const;
    [[nodiscard]] glm::dvec3 to(const glm::dvec3& v) const;
    [[nodiscard]] const glm::dvec3& normal() const;

    static glm::dvec3 from(const glm::dvec3& v, const glm::dvec3& N);

private:
    glm::dmat3 t_;
};
