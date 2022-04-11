#ifndef _SCENE_H_
#define _SCENE_H_

#include <map>
#include <vector>
#include "../render/instant_camera.h"
#include "../render/instant_image.h"
#include "../render/object.h"
#include "../render/instant_surface.h"
#include "../render/tracer.h"

using namespace instant_renderer;

namespace instant_renderer
{
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

        void render(const int super_samples, const ViewPlane view_plane);

    };
}
#endif
