#include "path-tracer.h"

#include <glm/gtx/component_wise.hpp>

#include "camera/surface/surface.h"
#include "camera/interaction.h"
#include "camera/sampler/sampler.h"

glm::dvec3 PathTracer::sample_ray(Ray ray)
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

        radiance += Integrator::sample_emissive(interaction, ls) * throughput;
        radiance += Integrator::sample_direct(interaction, ls) * throughput;

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
