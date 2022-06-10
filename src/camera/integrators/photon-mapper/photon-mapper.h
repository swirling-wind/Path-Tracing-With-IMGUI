#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

#include "../photon-mapper/photon.h"
#include "../integrator.h"
#include "camera/scene manage/linear-octree.h"

class PhotonMapper : public Integrator
{
public:
    PhotonMapper(const nlohmann::json& j);

    void emitPhoton(Ray ray, glm::dvec3 flux, size_t thread);

    virtual glm::dvec3 sample_ray(Ray ray);
    
    glm::dvec3 estimate_global_radiance(const Interaction& interaction); // All radiance except caustic
    glm::dvec3 estimate_caustic_radiance(const Interaction& interaction);

private:
    LinearOctree<Photon> caustic_map;
    LinearOctree<Photon> global_map; // all photons except caustic photons
    
    std::vector<std::vector<Photon>> caustic_vecs;
    std::vector<std::vector<Photon>> global_vecs;

    double non_caustic_reject;

    bool direct_visualization;

    uint16_t max_node_data;
    size_t k_nearest_photons;
};
