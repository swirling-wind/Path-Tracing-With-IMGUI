#include "scene.h"

ViewPlane::ViewPlane(double plane_width, int width_res, int height_res)
	: plane_width(plane_width), width_res(width_res), height_res(height_res)
{
	plane_height = plane_width * height_res / width_res;
	pixel_size = plane_width / width_res;
}

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

void Scene::render(const std::map<std::string, std::string>& params)
{
	const int samples = std::stoi(params.at("samples"));
	const int super_samples = std::stoi(params.at("super_samples"));
	const ViewPlane view_plane(std::stod(params.at("plane_width")), std::stoi(params.at("width_res")),
		std::stoi(params.at("height_res")));

	Image image(view_plane.width_res, view_plane.height_res);

	// for path-tracing
	UniformRealGenerator rnd(1);

	// for ray-tracing or path-tracing with NEE
	std::vector<Object*> lights = get_lights();

	// Construct the space data structure
	printf("Building BVH...\n");
	construct();

#pragma omp parallel for schedule(dynamic, 1)
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
		}
	}
	save_ppm_file("result_2.ppm", image);
}
