#ifndef _SCENE_H_
#define _SCENE_H_

#include <map>
#include <vector>
#include "camera.h"
#include "object.h"
#include "surface.h"
#include "tracer.h"
enum class render_status
{
    rendering,
    halt
};

struct ViewPlane
{
	double plane_width, plane_height;
	int width_res, height_res;
	double pixel_size;

	ViewPlane(double plane_width, int width_res, int height_res);
};

class Scene
{
	Camera* camera_ptr;
	std::vector<Object*> objects;
	BVH bvh;
	Texture* ibl_ptr;

public:
	Scene();
	~Scene();

	void set_camera(Camera* camera_ptr);
	void set_ibl(Texture* ibl_ptr);
	void add_object(Object* object_ptr);

    [[nodiscard]] std::vector<Object*> get_lights() const;
	void construct();
    [[nodiscard]] std::vector<int> traverse(const Ray& ray) const;
	void render(const std::map<std::string, std::string>& scene_params, std::atomic<unsigned int>& iteration_count, std::atomic<render_status>& current_status);
    void instant_render_with_multi_threads();
};

#endif
