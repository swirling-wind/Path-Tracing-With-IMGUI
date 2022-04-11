#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../render/vec.h"

using namespace instant_renderer;

namespace instant_renderer
{

    struct Camera {
        Vec eye, lookat;
        Vec up;
        Vec u, v, w;
        float exposure_time;

        Camera(Vec eye, Vec lookat);
        virtual ~Camera();

        virtual Vec ray_direction(const Point2D& p) const = 0;
    };

    struct Pinhole : public Camera {
        float dist;

        Pinhole(Vec eye, Vec lookat, float dist);
        ~Pinhole() override;

        Vec ray_direction(const Point2D& p) const override;
    };


}
#endif
