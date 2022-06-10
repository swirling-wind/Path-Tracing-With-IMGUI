#include "bvh.h"

#include <chrono>
#include <iostream>
#include <glm/gtx/component_wise.hpp>

#include "octree.cpp"
#include "camera/intersection.h"
#include "camera/surface/surface.h"
#include "tools/constexpr-math.h"
#include "tools/format.h"
#include "tools/priority-queue.h"
#include "tools/util.h"

BVH::BVH(const BoundingBox &bounding_box, 
         const std::vector<std::shared_ptr<Surface::Base>> &surfaces, 
         const nlohmann::json &j) : depth_first_index_(0)
{
    auto root = std::make_shared<BuildNode>();
    root->bb = bounding_box;

    auto begin = std::chrono::high_resolution_clock::now();

    auto type = getOptional<std::string>(j, "type", "OCTREE");
    if (type == "QUATERNARY_SAH")
    {
        bins_per_axis = getOptional(j, "bins_per_axis", 8);
        std::cout << "\nBuilding quaternary BVH using SAH.\n\n";
        root->surfaces = surfaces;
        recursive_build_quaternary_sah(root);
    }
    else if (type == "BINARY_SAH")
    {
        bins_per_axis = getOptional(j, "bins_per_axis", 16);
        std::cout << "\nBuilding binary BVH using SAH.\n\n";
        root->surfaces = surfaces;
        recursive_build_binary_sah(root);
    }
    else // OCTREE
    {
        std::cout << "\nBuilding BVH from octree.\n\n";

        double half_max = glm::compMax(root->bb.dimensions()) / 2.0;
        BoundingBox cube_BB(root->bb.centroid() - half_max, root->bb.centroid() + half_max);

        Octree<SurfaceCentroid> hierarchy(cube_BB, leaf_surfaces);

        for (const auto &s : surfaces)
        {
            hierarchy.insert(SurfaceCentroid(s));
        }

        recursive_build_from_octree(hierarchy, root);
    }

    size_t num_nodes = 1;
    double num_branching = 0.0;
    for (const auto &b : branching)
    {
        num_branching += b.second;
        num_nodes += b.first * b.second;
    }

    ordered_surfaces_ = std::vector<std::shared_ptr<Surface::Base>>(surfaces.size(), nullptr);

    linear_tree_ = std::vector<LinearNode>(num_nodes, LinearNode());

    uint32_t surface_index = 0;
    compact(root, 0, surface_index);

    auto end = std::chrono::high_resolution_clock::now();
    size_t millisecond_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "BVH constructed in " + Format::time_duration(millisecond_duration)
              << ". Node num: " << num_nodes
              << ". Branching num: " << num_branching
              << ". Branching factor of tree: " << (num_nodes - 1) / num_branching << std::endl;
}

Intersection BVH::intersect(const Ray& ray) const
{
    thread_local PriorityQueue<LinearNode::NodeIntersectionForPriorityQueue> to_visit; to_visit.clear();

    double t;
    Intersection intersect;
    if (linear_tree_[0].bb.intersect(ray, t))
    {
        uint32_t node_idx = 0;
        while (true)
        {
            const auto& [bb, start_surface, num_surfaces, next_sibling] = linear_tree_[node_idx];
            if (num_surfaces != 0)
            {
                const uint32_t end_index = start_surface + num_surfaces;
                for (uint32_t i = start_surface; i < end_index; i++)
                {
                    Intersection t_intersect;
                    if (ordered_surfaces_[i]->intersect(ray, t_intersect))
                    {
                        if (t_intersect.t < intersect.t)
                        {
                            intersect = t_intersect;
                            intersect.surface = ordered_surfaces_[i];
                        }
                    }
                }
            }
            else
            {
                uint32_t child_idx = node_idx + 1;
                while (child_idx != 0)
                {
                    if (linear_tree_[child_idx].bb.intersect(ray, t) && t < intersect.t)
                    {
                        to_visit.push({ t, child_idx });
                    }
                    child_idx = linear_tree_[child_idx].next_sibling;
                }
            }
            if (to_visit.empty() || to_visit.top().t >= intersect.t)
            {
                break;
            }
            node_idx = to_visit.top().node; 
            to_visit.pop();
        }
    }
    return intersect;
}

void BVH::recursive_build_from_octree(const Octree<SurfaceCentroid> &octree_node, std::shared_ptr<BuildNode> bvh_node)
{
    bvh_node->depth_first_index = depth_first_index_++;

    BoundingBox BB;

    if (octree_node.leaf())
    {
        bvh_node->surfaces.resize(octree_node.data_vec.size());
        for (size_t i = 0; i < bvh_node->surfaces.size(); i++)
        {
            bvh_node->surfaces[i] = octree_node.data_vec[i].surface;
            BB.merge(bvh_node->surfaces[i]->BB());
        }
    }
    else
    {
        size_t num_children = 0;
        for (size_t i = 0; i < octree_node.octants.size(); i++)
        {
            if (!(octree_node.octants[i]->leaf() && octree_node.octants[i]->data_vec.empty()))
            {
                num_children++;
                std::shared_ptr<BuildNode> child = std::make_shared<BuildNode>();
                bvh_node->children.push_back(child);
                recursive_build_from_octree(*octree_node.octants[i], child);
                BB.merge(child->bb);
            }
        }
        branching[num_children]++;
    }
    bvh_node->bb = BB;
}

void BVH::recursive_build_binary_sah(std::shared_ptr<BuildNode> bvh_node)
{
    bvh_node->depth_first_index = depth_first_index_++;

    auto &surfaces_list = bvh_node->surfaces;

    if (surfaces_list.size() <= leaf_surfaces)
    {
        return;
    }

    BoundingBox centroid_extent;
    for (const auto &s : surfaces_list)
    {
        centroid_extent.merge(s->BB().centroid());
    }
    glm::dvec3 extent_dims = centroid_extent.dimensions();

    uint8_t split_axis = extent_dims.x > extent_dims.y ? 
                        (extent_dims.x > extent_dims.z ? 0 : 2) : 
                        (extent_dims.y > extent_dims.z ? 1 : 2);

    if (extent_dims[split_axis] < Constant::EPSILON)
    {
        if (surfaces_list.size() > max_leaf_surfaces)
        {
            arbitrary_split(bvh_node, 2);
            for (const auto& child : bvh_node->children) recursive_build_binary_sah(child);
        }
        return;
    }

    auto get_index = [&](const glm::dvec3 &centroid)
    {
        double f = (centroid[split_axis] - centroid_extent.min[split_axis]) / extent_dims[split_axis];
        const int index = static_cast<int>(glm::floor(f * bins_per_axis));
        return glm::min(index, bins_per_axis - 1);
    };

    std::vector<std::pair<size_t, BoundingBox>> bins(bins_per_axis, { 0, BoundingBox() });
    for (const auto &s : surfaces_list)
    {
        int idx = get_index(s->BB().centroid());
        bins[idx].first++;
        bins[idx].second.merge(s->BB());
    }

    std::shared_ptr<BuildNode> A = std::make_shared<BuildNode>();
    std::shared_ptr<BuildNode> B = std::make_shared<BuildNode>();

    double min_cost = std::numeric_limits<double>::max();
    size_t split_bin = 0;

    for (size_t i = 0; i < bins_per_axis - 1; i++)
    {
        size_t A_count = 0;
        BoundingBox A_BB;
        for (size_t j = 0; j < i + 1; j++)
        {
            A_count += bins[j].first;
            A_BB.merge(bins[j].second);
        }

        size_t B_count = 0;
        BoundingBox B_BB;
        for (size_t j = i + 1; j < bins_per_axis; j++)
        {
            B_count += bins[j].first;
            B_BB.merge(bins[j].second);
        }

        double cost = 1.0 + (A_count * A_BB.area() + B_count * B_BB.area()) / bvh_node->bb.area();

        if (cost < min_cost)
        {
            split_bin = i;
            min_cost = cost;
        }
    }

    if (min_cost > surfaces_list.size())
    {
        if (surfaces_list.size() > max_leaf_surfaces)
        {
            arbitrary_split(bvh_node, 2);
            for (const auto& child : bvh_node->children) recursive_build_binary_sah(child);
        }
        return;
    }

    for (const auto &s : surfaces_list)
    {
        int index = get_index(s->BB().centroid());
        if (index <= split_bin)
        {
            A->surfaces.push_back(s);
            A->bb.merge(s->BB());
        }
        else
        {
            B->surfaces.push_back(s);
            B->bb.merge(s->BB());
        }
    }

    surfaces_list.clear();
   
    size_t num_children = 0;

    if (!A->surfaces.empty())
    {
        num_children++;
        bvh_node->children.push_back(A);
        recursive_build_binary_sah(A);
    }
    if (!B->surfaces.empty())
    {
        num_children++;
        bvh_node->children.push_back(B);
        recursive_build_binary_sah(B);
    }

    branching[num_children]++;
}

void BVH::recursive_build_quaternary_sah(const std::shared_ptr<BuildNode>& bvh_node)
{
    bvh_node->depth_first_index = depth_first_index_++;

    glm::ivec2 num_bins(bins_per_axis);

    auto &surfaces_list = bvh_node->surfaces;

    if (surfaces_list.size() <= leaf_surfaces)
    {
        return;
    }

    BoundingBox centroid_extent;
    for (const auto &s : surfaces_list)
    {
        centroid_extent.merge(s->BB().centroid());
    }
    glm::dvec3 extent_dims = centroid_extent.dimensions();

    glm::ivec2 axes = extent_dims.x > extent_dims.y ?
                     (extent_dims.y > extent_dims.z ? glm::ivec2(0, 1) : glm::ivec2(0, 2)) :
                     (extent_dims.x > extent_dims.z ? glm::ivec2(0, 1) : glm::ivec2(1, 2));
    
    if (extent_dims[axes.x] < Constant::EPSILON || extent_dims[axes.y] < Constant::EPSILON)
    {
        depth_first_index_--;
        recursive_build_binary_sah(bvh_node);
        return;
    }

    glm::dvec2 axes_min(centroid_extent.min[axes.x], centroid_extent.min[axes.y]);
    glm::dvec2 axes_dim(extent_dims[axes.x], extent_dims[axes.y]);

    auto getIdx = [&](const glm::dvec3 &centroid)
    {
        glm::dvec2 f = (glm::dvec2(centroid[axes.x], centroid[axes.y]) - axes_min) / axes_dim;
        glm::ivec2 idx = glm::floor(f * glm::dvec2(num_bins));
        return glm::min(idx, num_bins - 1);
    };

    std::vector<std::vector<std::pair<size_t, BoundingBox>>> bins(num_bins.x,
        std::vector<std::pair<size_t, BoundingBox>>(num_bins.y, { 0, BoundingBox() })
    );

    for (const auto &s : surfaces_list)
    {
        glm::ivec2 idx = getIdx(s->BB().centroid());
        bins[idx.x][idx.y].first++;
        bins[idx.x][idx.y].second.merge(s->BB());
    }

    double min_cost = std::numeric_limits<double>::max();
    glm::ivec2 split_bin(0);

    for (size_t i = 0; i < num_bins.x - 1; i++)
    {
        for (size_t j = 0; j < num_bins.y - 1; j++)
        {
            std::vector<BoundingBox> b_boxs(4, BoundingBox());
            std::vector<size_t> counts(4, 0);

            for (uint8_t v = 0b00; v <= 0b11; v++)
            {
                std::vector<glm::ivec2> range(2, glm::ivec2(0));
                range[0] = v & 0b01 ? glm::ivec2(i + 1, num_bins.x) : glm::ivec2(0, i + 1);
                range[1] = v & 0b10 ? glm::ivec2(j + 1, num_bins.y) : glm::ivec2(0, j + 1);

                for (size_t x = range[0][0]; x < range[0][1]; x++)
                {
                    for (size_t y = range[1][0]; y < range[1][1]; y++)
                    {
                        counts[v] += bins[x][y].first;
                        b_boxs[v].merge(bins[x][y].second);
                    }
                }
            }

            double cost = 0.0;
            for (uint8_t v = 0b00; v <= 0b11; v++)
            {
                cost += b_boxs[v].area() * counts[v];
            }

            cost = 1.0 + cost / bvh_node->bb.area();

            if (cost < min_cost)
            {
                split_bin = glm::ivec2(i, j);
                min_cost = cost;
            }
        }
    }

    if (min_cost > surfaces_list.size())
    {
        if (surfaces_list.size() > max_leaf_surfaces)
        {
            arbitrary_split(bvh_node, 4);
            for (const auto& child : bvh_node->children) recursive_build_quaternary_sah(child);
        }
        return;
    }

    std::vector<std::shared_ptr<BuildNode>> new_nodes(4, nullptr);

    for (const auto &s : surfaces_list)
    {
        glm::ivec2 idx = getIdx(s->BB().centroid());

        uint8_t child_idx = 0b00;
        if (idx.x > split_bin.x) child_idx |= 0b01;
        if (idx.y > split_bin.y) child_idx |= 0b10;

        if (!new_nodes[child_idx])
        {
            new_nodes[child_idx] = std::make_shared<BuildNode>();
        }

        new_nodes[child_idx]->surfaces.push_back(s);
        new_nodes[child_idx]->bb.merge(s->BB());
    }

    surfaces_list.clear();

    size_t num_children = 0;
    for (const auto &child : new_nodes)
    {
        if (child)
        {
            num_children++;
            bvh_node->children.push_back(child);
            recursive_build_quaternary_sah(child);
        }
    }
    branching[num_children]++;
}

void BVH::compact(std::shared_ptr<BuildNode> bvh_node, uint32_t next_sibling, uint32_t &surface_idx)
{
    linear_tree_[bvh_node->depth_first_index].bb = bvh_node->bb;
    linear_tree_[bvh_node->depth_first_index].next_sibling = next_sibling;
    linear_tree_[bvh_node->depth_first_index].start_surface = surface_idx;
    linear_tree_[bvh_node->depth_first_index].num_surfaces = static_cast<uint8_t>(bvh_node->surfaces.size());

    for (const auto &surface : bvh_node->surfaces)
    {
        ordered_surfaces_[surface_idx] = surface;
        surface_idx++;
    }

    if (!bvh_node->children.empty())
    {
        for (size_t i = 0; i < bvh_node->children.size() - 1; i++)
        {
            compact(bvh_node->children[i], bvh_node->children[i + 1]->depth_first_index, surface_idx);
        }
        compact(bvh_node->children.back(), 0, surface_idx);
    }
}

void BVH::arbitrary_split(std::shared_ptr<BuildNode> bvh_node, size_t N)
{
    auto& S = bvh_node->surfaces;

    N = std::min(N, S.size());

    for (size_t i = 0; i < N; i++)
    {
        bvh_node->children.push_back(std::make_shared<BuildNode>());
    }

    for (size_t i = 0; i < S.size(); i++)
    {
        const size_t index = i % N;
        bvh_node->children[index]->surfaces.push_back(S[i]);
        bvh_node->children[index]->bb.merge(S[i]->BB());
    }

    S.clear();
    branching[N]++;
}

BVH::SurfaceCentroid::SurfaceCentroid(const std::shared_ptr<Surface::Base> surface)
    : centroid(surface->BB().centroid()), surface(surface) { }
