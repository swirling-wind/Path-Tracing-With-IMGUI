#include "../path-tracer/path-tracer.h"

#include "../../common/util.h"
#include "../../sampling/sampler.h"
#include "../../common/constants.h"
#include "../../material/material.h"
#include "../../surface/surface.h"
#include "../../ray/interaction.h"
#include "../../common/constexpr-math.h"
#include "../../surface/surface.h"
#include <glm/gtx/component_wise.hpp>
glm::dvec3 PathTracer::sampleRay(Ray ray)
{
    glm::dvec3 radiance(0.0), throughput(1.0);
    RefractionHistory refraction_history(ray);
    glm::dvec3 bsdf_absIdotN;
    LightSample ls;

    while (true)
    {
        Sampler::shuffle();

        Intersection intersection = scene.intersect(ray);

        if (!intersection)
        {
            return radiance + scene.skyColor(ray) * throughput;
        }

        Interaction interaction(intersection, ray, refraction_history.externalIOR(ray));

        radiance += Integrator::sampleEmissive(interaction, ls) * throughput;
        radiance += Integrator::sampleDirect(interaction, ls) * throughput;

        if (!interaction.sampleBSDF(bsdf_absIdotN, ls.bsdf_pdf, ray))
        {
            return radiance;
        }

        throughput *= bsdf_absIdotN / ls.bsdf_pdf;

        if (absorb(ray, throughput))
        {
            return radiance;
        }

        refraction_history.update(ray);
    }
}
