#include "bounding-box.h"

#include <algorithm>
#include <glm/common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtx/component_wise.hpp>

#include "camera/ray.h"


bool BoundingBox::is_valid() const
{
    for (int i = 0; i < 3; i++)
    {
        if (min[i] > max[i]) return false;
    }
    return true;
}

bool BoundingBox::intersect(const Ray &ray, double &t) const
{
    const glm::dvec3 t0{ (min - ray.start) * ray.inv_direction };
    const glm::dvec3 t1{ (max - ray.start) * ray.inv_direction };

    t = std::max(compMax(glm::min(t0, t1)), 0.0);

    return compMin(glm::max(t0, t1)) >= t;
}

bool BoundingBox::contains(const glm::dvec3 &point) const
{
    return point.x >= min.x && point.y >= min.y && point.z >= min.z && 
           point.x <= max.x && point.y <= max.y && point.z <= max.z;
}

glm::dvec3 BoundingBox::dimensions() const
{
    return max - min;
}

glm::dvec3 BoundingBox::centroid() const
{
    return (max + min) / 2.0;
}

double BoundingBox::area() const
{
    if (!is_valid()) return 0.0;
    const glm::dvec3 dim = dimensions();
    return 2.0 * (dim.x * dim.y + dim.x * dim.z + dim.y * dim.z);
}

double BoundingBox::smallest_possible_squared_distance_to_box(const glm::dvec3 &point) const
{
    const glm::dvec3 distance = glm::max(glm::max(min - point, point - max), glm::dvec3(0.0));
    return dot(distance, distance);
}

double BoundingBox::largest_possible_squared_distance_to_box(const glm::dvec3& point) const
{
    const glm::dvec3 distance = glm::max(max - point, point - min);
    return dot(distance, distance);
}

void BoundingBox::merge(const BoundingBox &bb)
{
    for (int i = 0; i < 3; i++)
    {
        if (min[i] > bb.min[i]) min[i] = bb.min[i];
        if (max[i] < bb.max[i]) max[i] = bb.max[i];
    }
}

void BoundingBox::merge(const glm::dvec3 &point)
{
    for (int i = 0; i < 3; i++)
    {
        if (min[i] > point[i]) min[i] = point[i];
        if (max[i] < point[i]) max[i] = point[i];
    }
}
