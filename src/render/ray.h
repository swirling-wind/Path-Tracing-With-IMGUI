#ifndef _RAY_H_
#define _RAY_H_

#include "../render/constant.h"
#include "../render/vec.h"

using namespace instant_renderer;

namespace instant_renderer
{
    struct Ray {
        Vec org, dir;

        Ray();
        Ray(const Vec& org, const Vec& dir);
    };

    struct Hitpoint {
        double distance;
        Vec normal;
        Vec position;
        double u, v;

        Hitpoint();
    };

    struct Intersection {
        Hitpoint hitpoint;
        int object_id;

        Intersection();
    };
}
#endif
