#pragma once

#include <glm/vec3.hpp>
#include "../ray/ray.h"

struct BoundingBox
{
    BoundingBox() = default;
    BoundingBox(const glm::dvec3 min, const glm::dvec3 max) 
        : min(min), max(max) { }

    glm::dvec3 min = glm::dvec3(std::numeric_limits<double>::max());
    glm::dvec3 max = glm::dvec3(std::numeric_limits<double>::lowest());

    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] bool intersect(const Ray &ray, double &t) const;
    [[nodiscard]] bool contains(const glm::dvec3 &point) const;
    [[nodiscard]] glm::dvec3 dimensions() const;
    [[nodiscard]] glm::dvec3 centroid() const;
    [[nodiscard]] double area() const;
    [[nodiscard]] double smallest_possible_squared_distance_to_box(const glm::dvec3 &point) const;
    [[nodiscard]] double largest_possible_squared_distance_to_box(const glm::dvec3& point) const;
    void merge(const BoundingBox& bb);
    void merge(const glm::dvec3& point);
    };
