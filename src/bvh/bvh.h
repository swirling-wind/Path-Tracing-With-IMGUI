#pragma once

#include <nlohmann/json.hpp>

#include "../ray/intersection.h"
#include "../octree/octree.h"
#include "../common/bounding-box.h"

namespace Surface { class Base; }

class BVH
{
    struct BuildNode
    {
        BuildNode() : depth_first_index(0){}

        BoundingBox bb;
        std::vector<std::shared_ptr<BuildNode>> children;
        std::vector<std::shared_ptr<Surface::Base>> surfaces;
        uint32_t depth_first_index;

        [[nodiscard]] bool leaf() const
        {
            return children.empty();
        }
    };

    struct SurfaceCentroid
    {
        explicit SurfaceCentroid(std::shared_ptr<Surface::Base> surface);
        glm::dvec3 centroid;
        std::shared_ptr<Surface::Base> surface;

        [[nodiscard]] glm::dvec3 pos() const
        {
            return centroid;
        }
    };

    struct alignas(64) LinearNode
    {
        BoundingBox bb;
        uint32_t start_surface;
        uint8_t num_surfaces;
        uint32_t next_sibling; // 0 if there is none
        
        struct alignas(16) NodeIntersectionForPriorityQueue
        {
            bool operator< (const NodeIntersectionForPriorityQueue& i) const { return i.t < t; }
            double t;
            uint32_t node;
        };
    };

public:
    BVH(const BoundingBox &bounding_box, 
        const std::vector<std::shared_ptr<Surface::Base>> &surfaces, 
        const nlohmann::json &j);

    [[nodiscard]] Intersection intersect(const Ray& ray) const;

    static constexpr size_t leaf_surfaces = 8;
    static constexpr size_t max_leaf_surfaces = 0xFF;
    std::map<size_t, size_t> branching;

    int bins_per_axis = 16;

private:
    void compact(std::shared_ptr<BuildNode> bvh_node, uint32_t next_sibling, uint32_t& surface_idx);
    void arbitrary_split(std::shared_ptr<BuildNode> bvh_node, size_t N);
    void recursive_build_from_octree(const Octree<SurfaceCentroid> &octree_node, std::shared_ptr<BuildNode> bvh_node);
    void recursive_build_binary_sah(std::shared_ptr<BuildNode> bvh_node);
    void recursive_build_quaternary_sah(const std::shared_ptr<BuildNode>& bvh_node);
    std::vector<LinearNode> linear_tree_;
    std::vector<std::shared_ptr<Surface::Base>> ordered_surfaces_;
    uint32_t depth_first_index_;
};
