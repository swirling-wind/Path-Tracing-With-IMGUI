#pragma once

#include <glm/vec3.hpp>
#include "../ray/ray.h"

struct BoundingBox
{
    BoundingBox() = default;
    BoundingBox(const glm::dvec3 min, const glm::dvec3 max) 
        : min(min), max(max) { }

    bool intersect(const Ray &ray, double &t) const;
    bool contains(const glm::dvec3 &p) const;
    glm::dvec3 dimensions() const;
    glm::dvec3 centroid() const;
    double area() const;
    double smallest_possible_squared_distance_to_box(const glm::dvec3 &point) const;
    double largest_possible_squared_distance_to_box(const glm::dvec3& point) const;
    void merge(const BoundingBox &bb);
    void merge(const glm::dvec3 &p);
    bool valid() const;

    glm::dvec3 min = glm::dvec3(std::numeric_limits<double>::max());
    glm::dvec3 max = glm::dvec3(std::numeric_limits<double>::lowest());
};
