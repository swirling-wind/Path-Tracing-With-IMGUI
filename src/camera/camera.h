#pragma once

#include <chrono>
#include <deque>
#include <atomic>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <nlohmann/json.hpp>

#include "../camera/image.h"
#include "../camera/film.h"

#include "../common/work-queue.h"
#include "render/gui_param_tool.h"

class Integrator;

class Camera
{
public:
    Camera() = default;

    void init_integrator_and_scene(nlohmann::json j, bool is_photon_map, std::atomic<gui_params_space::render_status>& status);

    void import_camera_and_image_properties(const nlohmann::json& j);

    void previewImage(std::vector<glm::vec3>& gui_image, std::atomic<gui_params_space::render_status>& status, double& render_progress);

    void capture();

    void sampleImage();
    void sampleImageForPreview(double& render_progress);
    

    void saveImage() const
    {
        image.save(savename);
    }

    void setPosition(const glm::dvec3& p)
    {
        eye = p;
    }

    void lookAt(const glm::dvec3& p);

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

   

    void samplePixel(size_t x, size_t y);
    void sampleImageThread(WorkQueue<Bucket>& buckets);

    void printPreviewInfoThread(WorkQueue<Bucket>& buckets, double& render_progress);
    void printInfoThread(WorkQueue<Bucket>& buckets);

    const size_t bucket_size = 32;

    std::shared_ptr<Integrator> integrator;

    std::atomic_size_t num_sampled_pixels = 0;
    size_t last_num_sampled_pixels = 0;
    std::chrono::time_point<std::chrono::steady_clock> last_update = std::chrono::steady_clock::now();
    const size_t num_times = 32;
    std::deque<double> times;
};
