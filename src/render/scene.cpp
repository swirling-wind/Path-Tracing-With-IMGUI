#include "scene.h"

#include <mutex>
#include <thread>

std::mutex mtx;

ViewPlane::ViewPlane(const double plane_width, const int width_res, const int height_res)
    : plane_width(plane_width), plane_height(plane_width* height_res / width_res), width_res(width_res), height_res(height_res),
    pixel_size(plane_width / width_res)
{}

Scene::Scene() : camera_ptr(nullptr), objects(0), ibl_ptr(nullptr) {}
Scene::~Scene()
{
    delete camera_ptr;
    delete ibl_ptr;
}

void Scene::set_camera(Camera* camera_ptr)
{
    this->camera_ptr = camera_ptr;
}
void Scene::set_ibl(Texture* ibl_ptr)
{
    this->ibl_ptr = ibl_ptr;
}
void Scene::add_object(Object* object_ptr)
{
    objects.push_back(object_ptr);
}

std::vector<Object*> Scene::get_lights() const
{
    std::vector<Object*> lights;
    for (const auto& obj : this->objects)
    {
        if (obj->material_ptr->emission > Vec(0.0, 0.0, 0.0))
        {
            lights.push_back(obj);
        }
    }
    return lights;
}

void Scene::construct()
{
    std::vector<BBox*> bboxes(objects.size());
    for (size_t i = 0; i < objects.size(); i++)
    {
        bboxes[i] = &objects[i]->bbox;
    }
    bvh.construct(bboxes);
}

std::vector<int> Scene::traverse(const Ray& ray) const
{
    return bvh.traverse(ray);
}

void Scene::render(const std::map<std::string, std::string>& scene_params, std::atomic<unsigned int>& iteration_count, std::atomic<render_status>& current_status)
{
    const int samples = std::stoi(scene_params.at("samples"));
    const int super_samples = std::stoi(scene_params.at("super_samples"));
    const ViewPlane view_plane(std::stod(scene_params.at("plane_width")), std::stoi(scene_params.at("width_res")),
        std::stoi(scene_params.at("height_res")));

    Image image(view_plane.width_res, view_plane.height_res);

    // for path-tracing
    UniformRealGenerator rnd(1);

    // for ray-tracing or path-tracing with NEE
    std::vector<Object*> lights = get_lights();

    // Construct the space data structure
    printf("Building BVH...\n");
    construct();


#pragma omp parallel for schedule(dynamic, 1)  // NOLINT(clang-diagnostic-source-uses-openmp)
    for (int row = 0; row < view_plane.height_res; row++)
    {
        if (row % 20 == 0)
        {
            std::cerr << "processing line " << row << "\n";
        }
        for (int col = 0; col < view_plane.width_res; col++)
        {
            const int index = row * view_plane.width_res + col;
            for (int i = 0; i < super_samples; i++)
            {
                for (int j = 0; j < super_samples; j++)
                {
                    Color accumulated_radiance;

                    Point2D pp(
                        view_plane.pixel_size * (col - 0.5 * view_plane.width_res + (j + 0.5) / super_samples),
                        view_plane.pixel_size * (0.5 * view_plane.height_res - row - 1 + (i + 0.5) / super_samples));
                    Ray ray(camera_ptr->eye, camera_ptr->ray_direction(pp));
                    for (int k = 0; k < samples; k++)
                    {
                        accumulated_radiance += path_trace(ray, objects, bvh, ibl_ptr, rnd, 0);
                    }
                    image.color_vec[index] +=
                        accumulated_radiance / (samples * super_samples * super_samples);
                }
            }

            //const int samples_per_pixel = super_samples * super_samples;
            //for (int i = 0; i < super_samples * super_samples; i++)
            //{
            //    Color accumulated_radiance;
            //    Point2D pp(
            //        view_plane.pixel_size * (col - 0.5 * view_plane.width_res + (j + 0.5) / super_samples),
            //        view_plane.pixel_size * (0.5 * view_plane.height_res - row - 1 + (i + 0.5) / super_samples));
            //    Ray ray(camera_ptr->eye, camera_ptr->ray_direction(pp));

            //    accumulated_radiance = path_trace(ray, objects, bvh, ibl_ptr, rnd, 0) +
            //        path_trace(ray, objects, bvh, ibl_ptr, rnd, 0);

            //    image.color_vec[index] +=
            //        accumulated_radiance / (samples * super_samples * super_samples);
            //}

        }
    }
    
    save_ppm_file("result_3_100.ppm", image);
}

//void Scene::instant_render_with_multi_threads()
//{
//    
//    constexpr struct instant_render_params
//    {
//        const int thread_num = 20;
//        const int samples = 2;
//        const int super_samples = 4;
//
//        const double plane_width = 1.5;
//        const int width_res = 1200;
//        const int height_res = 900;
//        const double pixel_size = plane_width / width_res;
//
//        const int row_num_each_thread = height_res / thread_num;
//    } params;
//
//    Image image(params.width_res, params.height_res);
//
//    // for ray-tracing or path-tracing with NEE
//    std::vector<Object*> lights = get_lights();
//
//    // Construct the space data structure
//    printf("Building BVH...\n");
//    construct();
//
//    auto render_rows = [this, params](const int start_row, const int end_row)
//    {
//        // for path-tracing
//        UniformRealGenerator rnd(1);
//        const int start_image_index = start_row * params.width_res;
//        const int end_image_index = end_row * params.width_res - 1;
//
//        //std::array<Color, end_image_index - start_image_index> temp_image_content;
//        std::vector temp_image_content(end_image_index - start_image_index, Color{0,0,0});
//
//        for (int row = start_row; row < end_row; row++)
//        {
//            for (int col = 0; col < params.width_res; col++)
//            {
//                const int index = row * params.width_res + col;
//
//                //in a pixel
//                for (int i = 0; i < params.super_samples; i++)
//                {
//                    for (int j = 0; j < params.super_samples; j++)
//                    {
//                        Point2D ray_direction(
//                            params.pixel_size * (col - 0.5 * params.width_res + (j + 0.5) / params.super_samples),
//                            params.pixel_size * (0.5 * params.height_res - row - 1 + (i + 0.5) / params.super_samples));
//                        Ray ray(this->camera_ptr->eye, camera_ptr->ray_direction(ray_direction));
//
//                        Color accumulated_radiance;
//                        for (int k = 0; k < params.samples; k++)
//                        {
//                            accumulated_radiance += path_trace(ray, objects, bvh, ibl_ptr, rnd, 0);
//                        }
//                        
//                        temp_image_content[index] += accumulated_radiance / (params.samples * params.super_samples * params.super_samples);
//                    }
//                }
//            }
//        }
//
//        
//        //image.color_vec[start_image_index : end_image_index];
//    };
//
//    // height == 900
//    // thread_num == 20
//    // row_num_for_each_thread == 900 / 20 = 45
//
//    std::vector<std::thread> thread_list;
//    thread_list.reserve(params.thread_num);
//    for (int index = 0; index < params.thread_num; ++index)
//    {
//        thread_list.emplace_back(render_rows, (index * params.row_num_each_thread), ((index + 1) * params.row_num_each_thread));
//    }
//
//
//}
