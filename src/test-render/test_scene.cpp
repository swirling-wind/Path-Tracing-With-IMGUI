#include "pch.h"

#include <iostream>
#include <map>
#include "../render/scene.h"
using namespace instant_renderer;
using namespace std;

TEST(scene_test_case, test_correct_viewplane_constructor)
{
    double plane_width = 640.0;
    int width_res = 640, height_res = 480;
    ViewPlane vp(plane_width, width_res, height_res);

    ASSERT_EQ(vp.plane_height, 480.0);
    ASSERT_EQ(vp.pixel_size, 1.0);
}
