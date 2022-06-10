#pragma once

#include <nlohmann/json.hpp>
#include <glm/vec3.hpp>

#include "../integrator.h"

class PathTracer : public Integrator
{
public:
    PathTracer(const nlohmann::json& j) : Integrator(j) { }

    glm::dvec3 sample_ray(Ray ray) override;
};
