#include "../camera/camera.h"

#include <thread>
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>

#include "../ray/ray.h"
#include "../integrator/path-tracer/path-tracer.h"
#include "../integrator/photon-mapper/photon-mapper.h"
#include "../sampling/sampling.h"
#include "../sampling/sampler.h"
#include "../common/util.h"
#include "../common/constexpr-math.h"
#include "../common/format.h"
#include "../common/constexpr-math.h"
#include "render/gui_param_tool.h"

void Camera::init_integrator_and_scene(const nlohmann::json& j, const bool is_photon_map, std::atomic<gui_params_space::render_status>& status)
{
    if (is_photon_map)
    {
        integrator_ = std::make_shared<PhotonMapper>(j);
    }
    else
    {
        integrator_ = std::make_shared<PathTracer>(j);
    }
    status = gui_params_space::render_status::scene_prepared_ready_to_preview;
    std::cout << "Successfully Prepare the scene, ready to preview\n";
}


void Camera::import_camera_and_image_properties(const nlohmann::json& j)
{
    const nlohmann::json& cam_json = j.at("cameras").at(0);
    image = Image(cam_json.at("image"));
    film = Film(image.width, image.height);

    eye = cam_json.at("eye");
    focal_length = cam_json.at("focal_length").get<double>() / 1000.0;
    sensor_width = cam_json.at("sensor_width").get<double>() / 1000.0;
    sqrtspp = cam_json.at("sqrtspp");
    savename = cam_json.at("savename");
    aperture_radius = (focal_length / getOptional(cam_json, "f_stop", -1.0)) / 2.0;
    focus_distance = getOptional(cam_json, "focus_distance", -1.0);

    if (cam_json.find("look_at") != cam_json.end())
    {
        const glm::dvec3 look_at_property = cam_json.at("look_at");
        look_at(look_at_property);
        if (focus_distance < 0.0)
        {
            focus_distance = distance(eye, look_at_property);
        }
    }
    else
    {
        forward = normalize(cam_json.at("forward").get<glm::dvec3>());
        up = normalize(cam_json.at("up").get<glm::dvec3>());
        left = normalize(cross(up, forward));
    }

    thin_lens = aperture_radius > 0.0 && focus_distance > 0.0;
}


void Camera::sample_pixel(size_t x, size_t y)
{
    double pixel_size = sensor_width / image.width;
    size_t spp = pow2(sqrtspp);

    glm::dvec2 half_dim = glm::dvec2(image.width, image.height) * 0.5;

    Sampler::initiate(static_cast<uint32_t>(y * image.width + x));

    for (size_t i = 0; i < spp; i++)
    {
        Sampler::setIndex(i);

        auto u = Sampler::get<PIXEL, 2>();
        glm::dvec2 px(x + u[0], y + u[1]);
        glm::dvec2 local = pixel_size * (half_dim - px);
        glm::dvec3 direction = glm::normalize(forward * focal_length + left * local.x + up * local.y);

        // Pinhole camera ray
        Ray ray(eye, direction, integrator_->scene.ior);

        if (thin_lens)
        {
            // Thin lens camera ray for depth of field
            auto u = Sampler::get<Dim::LENS, 2>();
            glm::dvec2 aperture_sample = Sampling::uniformDisk(u[0], u[1]) * aperture_radius;
            glm::dvec3 focus_point = ray(focus_distance / glm::dot(ray.direction, forward));
            glm::dvec3 start = eye + left * aperture_sample.x + up * aperture_sample.y;
            ray = Ray(start, glm::normalize(focus_point - start), integrator_->scene.ior);
        }
        film.deposit(px, integrator_->sample_ray(ray));
    }
    ++num_sampled_pixels_;
}


void Camera::sample_image_for_preview(double& render_progress)
{
    std::vector<Bucket> buckets_vec;
    for (size_t x = 0; x < image.width; x += bucket_size_)
    {
        size_t x_end = x + bucket_size_;
        if (x_end >= image.width) x_end = image.width;
        for (size_t y = 0; y < image.height; y += bucket_size_)
        {
            size_t y_end = y + bucket_size_;
            if (y_end >= image.height) y_end = image.height;
            buckets_vec.emplace_back(glm::ivec2(x, y), glm::ivec2(x_end, y_end));
        }
    }

    std::shuffle(buckets_vec.begin(), buckets_vec.end(), Random::engine);
    WorkQueue<Bucket> buckets(buckets_vec);
    buckets_vec.clear();

    std::function<void(Camera*, WorkQueue<Bucket>&)> f = &Camera::sample_image_thread;

    std::vector<std::unique_ptr<std::thread>> threads(integrator_->num_threads);
    for (auto& thread : threads)
    {
        thread = std::make_unique<std::thread>(f, this, std::ref(buckets));
    }

    std::function<void(Camera*, WorkQueue<Bucket>&, double&)> p = &Camera::print_preview_info_thread;
    std::thread print_thread(p, this, std::ref(buckets), std::ref(render_progress));
    print_thread.join();

    for (const auto& thread : threads)
    {
        thread->join();
    }

    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            image(x, y) = film.scan(x, y);
        }
    }
}


void Camera::sample_image_thread(WorkQueue<Bucket>& buckets)
{
    Bucket bucket;
    while (buckets.getWork(bucket))
    {
        for (size_t y = bucket.min.y; y < bucket.max.y; y++)
        {
            for (size_t x = bucket.min.x; x < bucket.max.x; x++)
            {
                sample_pixel(x, y);
            }
        }
    }
}

void Camera::look_at(const glm::dvec3& p)
{
    forward = normalize(p - eye);
    left = cross({ 0.0, 1.0, 0.0 }, forward);
    left = length(left) < Constant::EPSILON ? glm::dvec3(-1.0, 0.0, 0.0) : glm::normalize(left);
    up = normalize(glm::cross(forward, left));
}

void Camera::preview_image(std::vector<glm::vec3>& gui_image, std::atomic<gui_params_space::render_status>& status, double& render_progress)
{
    std::cout << std::endl << std::string(28, '-') << "| CHILD THREAD PREVIEW RENDERING PASS |" << std::string(28, '-') << std::endl;
    std::cout << std::endl << "Samples per pixel: " << pow2(static_cast<double>(sqrtspp)) << std::endl << std::endl;

    const auto before = std::chrono::system_clock::now();
    sample_image_for_preview(render_progress);
    const auto now = std::chrono::system_clock::now();
    std::cout << "\r" + std::string(100, ' ') + "\r";
    std::cout << "Preview Completed: " << Format::date(now);
    std::cout << ", Elapsed Time: " << Format::time_duration(std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()) << std::endl;

    gui_image = image.get_adjusted_float_blob();
    status = gui_params_space::render_status::finished_preview;
    std::cerr << "Finish inner preview render\n";
}

void Camera::print_preview_info_thread(WorkQueue<Bucket>& buckets, double& render_progress)
{
    auto print_progress_info = [](double progress, size_t milliseconds_duration, double& render_progress, std::ostream& output)
    {
        const auto estimated_time = std::chrono::system_clock::now() + std::chrono::milliseconds(milliseconds_duration);
        render_progress = progress;

        std::stringstream string_stream;
        string_stream << "\rTime remaining: " << Format::time_duration(milliseconds_duration)
            << " || " << Format::progress(progress * 100.0)
            << " || ETA: " << Format::date(estimated_time) + "    ";

        output << string_stream.str();
    };

    times_.clear();
    num_sampled_pixels_ = 0;
    last_num_sampled_pixels_ = 0;
    last_update_ = std::chrono::steady_clock::now();

    while (!buckets.empty())
    {
        if (num_sampled_pixels_ != last_num_sampled_pixels_)
        {
            const size_t delta_pixels = num_sampled_pixels_ - last_num_sampled_pixels_;
            const size_t pixels_left = image.num_pixels - num_sampled_pixels_;

            auto now = std::chrono::steady_clock::now();
            auto delta_t = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_);

            times_.push_back(static_cast<double>(delta_pixels) / delta_t.count());

            if (times_.size() > num_times_)
                times_.pop_front();

            // moving average
            const double pixels_per_milliseconds = std::accumulate(times_.begin(), times_.end(), 0.0) / times_.size();
            const double progress = static_cast<double>(num_sampled_pixels_) / image.num_pixels;
            const size_t milliseconds_left = static_cast<size_t>(pixels_left / pixels_per_milliseconds);

            print_progress_info(progress, milliseconds_left, render_progress, std::cout);

            last_update_ = now;
            last_num_sampled_pixels_ = num_sampled_pixels_;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
