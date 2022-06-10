#pragma once

#include <chrono>
#include <deque>
#include <atomic>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>

#include "image process/film.h"
#include "image process/image.h"
#include "render/gui_param_tool.h"
#include "tools/work-queue.h"

class Integrator;

class Camera
{
public:
    Camera() = default;

    void init_integrator_and_scene(const nlohmann::json& j, bool is_photon_map, std::atomic<gui_params_space::render_status>& status);

    void import_camera_and_image_properties(const nlohmann::json& j);

    void preview_image(std::vector<glm::vec3>& gui_image, std::atomic<gui_params_space::render_status>& status, double& render_progress);
    
    void sample_image_for_preview(double& render_progress);

    void look_at(const glm::dvec3& p);

    size_t sqrtspp;

    glm::dvec3 eye;
    glm::dvec3 forward, left, up;

    double focal_length, sensor_width, aperture_radius, focus_distance;
    Image image;
    Film film;
    bool thin_lens;

    std::string savename;

private:
    struct Bucket
    {
        Bucket() : min(0), max(0) { }
        Bucket(const glm::ivec2& min, const glm::ivec2& max) : min(min), max(max) { }

        glm::ivec2 min;
        glm::ivec2 max;
    };

    void sample_pixel(size_t x, size_t y);
    void sample_image_thread(WorkQueue<Bucket>& buckets);
    void print_preview_info_thread(WorkQueue<Bucket>& buckets, double& render_progress);

    const size_t bucket_size_ = 32;

    std::shared_ptr<Integrator> integrator_;

    std::atomic_size_t num_sampled_pixels_ = 0;
    size_t last_num_sampled_pixels_ = 0;
    std::chrono::time_point<std::chrono::steady_clock> last_update_ = std::chrono::steady_clock::now();
    const size_t num_times_ = 32;
    std::deque<double> times_;
};
