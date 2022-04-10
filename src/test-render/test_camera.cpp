#include "pch.h"
#include <iostream>
#include <map>
#include "camera.h"

using namespace std;

TEST(pin_hole_camera_test_case, test_correct_ray_direction) {
    Vec eye(0.0, 0.0, 0.0), lookat(1.0, 0.0, 0.0);
    float d = 0.5;

    Pinhole camera(eye, lookat, d);
    Vec dir(camera.ray_direction(Point2D(0.0, 0.0)));
    ASSERT_EQ(dir, Vec(1.0, 0.0, 0.0));
}
