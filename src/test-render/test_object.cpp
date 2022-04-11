#include "pch.h"

#include "../render/constant.h"
#include "../render/object.h"
#include "../render/ray.h"

Material material_magenta{ new ConstantTexture(Color(1.0, 0.0, 1.0)), Color(0.0, 0.0, 0.0),
                           ReflectionType::DIFFUSE };

//Sphere
TEST(sphere_object_test_case, test_correct_bbox_init) {
    Sphere sphere;
    ASSERT_EQ(sphere.bbox.corner0, Vec(-1.0 - EPS, -1.0 - EPS, -1.0 - EPS));
    ASSERT_EQ(sphere.bbox.corner1, Vec(1.0 + EPS, 1.0 + EPS, 1.0 + EPS));

    const double radius = 2.0;
    const Vec center(1.0, 1.0, 1.0);
    sphere = Sphere(radius, center, &material_magenta);
    ASSERT_EQ(sphere.bbox.corner0, Vec(-1.0 - EPS, -1.0 - EPS, -1.0 - EPS));
    ASSERT_EQ(sphere.bbox.corner1, Vec(3.0 + EPS, 3.0 + EPS, 3.0 + EPS));
}

TEST(sphere_object_test_case, test_correct_intersect) {
    const double radius = 1.0;
    const Vec center(2.0, 0.0, 0.0);

    const Sphere sphere(radius, center, &material_magenta);

    Hitpoint hitpoint;

    Ray ray(Vec(0.0, 0.0, 0.0), Vec(0.0, 1.0, 0.0));
    bool result = sphere.intersect(ray, hitpoint);
    ASSERT_EQ(result, false);
    ASSERT_EQ(hitpoint.distance, INF);
    ASSERT_EQ(hitpoint.normal, Vec(0.0, 0.0, 0.0));
    ASSERT_EQ(hitpoint.position, Vec(0.0, 0.0, 0.0));

    ray = Ray(Vec(0.0, 0.0, 0.0), Vec(1.0, 0.0, 0.0));
    result = sphere.intersect(ray, hitpoint);
    ASSERT_EQ(result, true);
    ASSERT_EQ(hitpoint.distance, 1.0);
    ASSERT_EQ(hitpoint.normal, Vec(-1.0, 0.0, 0.0));
    ASSERT_EQ(hitpoint.position, Vec(1.0, 0.0, 0.0));
}

//Plane
TEST(plane_object_test_case, test_correct_bbox_init) {
    Plane plane;
    ASSERT_EQ(plane.bbox.corner0, Vec(-INF, -EPS, -INF));
    ASSERT_EQ(plane.bbox.corner1, Vec(INF, EPS, INF));

    const Vec origin(0.0, 1.0, 0.0);
    const Vec normal(0.0, 1.0, 0.0);
    plane = Plane(origin, normal, &material_magenta);
    ASSERT_EQ(plane.bbox.corner0, Vec(-INF, 1.0 - EPS, -INF));
    ASSERT_EQ(plane.bbox.corner1, Vec(INF, 1.0 + EPS, INF));
}

TEST(plane_object_test_case, test_correct_intersect) {
    const Vec origin(0.0, 0.0, 0.0);
    const Vec normal(0.0, 1.0, 0.0);

    Plane plane(origin, normal, &material_magenta);

    Hitpoint hitpoint;

    Ray ray(Vec(0.0, 1.0, 0.0), Vec(0.0, 1.0, 0.0));
    bool result = plane.intersect(ray, hitpoint);
    ASSERT_EQ(result, false);
    ASSERT_EQ(hitpoint.distance, INF);
    ASSERT_EQ(hitpoint.normal, Vec(0.0, 0.0, 0.0));
    ASSERT_EQ(hitpoint.position, Vec(0.0, 0.0, 0.0));

    ray = Ray(Vec(0.0, 1.0, 0.0), Vec(0.0, -1.0, 0.0));
    result = plane.intersect(ray, hitpoint);
    ASSERT_EQ(result, true);
    ASSERT_EQ(hitpoint.distance, 1.0);
    ASSERT_EQ(hitpoint.normal, Vec(0.0, 1.0, 0.0));
    ASSERT_EQ(hitpoint.position, Vec(0.0, 0.0, 0.0));
}


//Triangle
TEST(triangle_object_test_case, test_correct_bbox_init) {
    Triangle triangle;
    ASSERT_EQ(triangle.bbox.corner0, Vec(-EPS, -EPS, -EPS));
    ASSERT_EQ(triangle.bbox.corner1, Vec(1.0 + EPS, EPS, 1.0 + EPS));

    const Vec v0(-2.0, 0.0, 0.0), v1(2.0, 0.0, 0.0), v2(0.0, 2.0, 0.0);
    triangle = Triangle(v0, v1, v2, &material_magenta);
    ASSERT_EQ(triangle.bbox.corner0, Vec(-2.0 - EPS, -EPS, -EPS));
    ASSERT_EQ(triangle.bbox.corner1, Vec(2.0 + EPS, 2.0 + EPS, EPS));
}

TEST(triangle_object_test_case, test_correct_intersect) {
    const Vec v0(-1.0, 0.0, 0.0), v1(1.0, 0.0, 0.0), v2(0.0, 2.0, 0.0);
    const Triangle triangle(v0, v1, v2, &material_magenta);

    Hitpoint hitpoint;

    Ray ray(Vec(1.0, 2.0, 1.0), Vec(0.0, 0.0, -1.0));
    bool result = triangle.intersect(ray, hitpoint);
    ASSERT_EQ(result, false);
    ASSERT_EQ(hitpoint.distance, INF);
    ASSERT_EQ(hitpoint.normal, Vec(0.0, 0.0, 0.0));
    ASSERT_EQ(hitpoint.position, Vec(0.0, 0.0, 0.0));

    ray = Ray(Vec(0.0, 1.0, 1.0), Vec(0.0, 0.0, -1.0));
    result = triangle.intersect(ray, hitpoint);
    ASSERT_EQ(result, true);
    ASSERT_EQ(hitpoint.distance, 1.0);
    ASSERT_EQ(hitpoint.normal, Vec(0.0, 0.0, 1.0));
    ASSERT_EQ(hitpoint.position, Vec(0.0, 1.0, 0.0));
}

//Box
TEST(box_object_test_case, test_correct_intersect) {
    const Vec corner0(-1.0, 1.0, -1.0);
    const Vec corner1(1.0, -1.0, 1.0);

    Box box(corner0, corner1, &material_magenta);
    Hitpoint hitpoint;

    Ray ray(Vec(0.0, 2.0, 3.0), Vec(0.0, 0.0, -1.0));
    bool result = box.intersect(ray, hitpoint);
    ASSERT_EQ(result, false);
    ASSERT_EQ(hitpoint.distance, INF);
    ASSERT_EQ(hitpoint.normal, Vec(0.0, 0.0, 0.0));
    ASSERT_EQ(hitpoint.position, Vec(0.0, 0.0, 0.0));

    ray = Ray(Vec(0.0, 0.0, 3.0), Vec(0.0, 0.0, -1.0));
    result = box.intersect(ray, hitpoint);
    ASSERT_EQ(result, true);
    ASSERT_EQ(hitpoint.distance, 2.0);
    ASSERT_EQ(hitpoint.normal, Vec(0.0, 0.0, 1.0));
    ASSERT_EQ(hitpoint.position, Vec(0.0, 0.0, 1.0));
}

//BVH bbox
TEST(bbox_object_test_case, test_correct_hit)
{
    const Vec corner0(-1.0, 1.0, -1.0);
    const Vec corner1(1.0, -1.0, 1.0);

    BBox bbox(corner0, corner1);

    // shoot a ray from outside the bounding box
    Ray ray(Vec(0.0, 2.0, 3.0), Vec(0.0, 0.0, -1.0));
    bool result = bbox.hit(ray);
    ASSERT_EQ(result, false);

    ray = Ray(Vec(0.0, 0.0, 3.0), Vec(0.0, 0.00001, -1.0));
    result = bbox.hit(ray);
    ASSERT_EQ(result, true);

    // Axis-parallel ray
    ray = Ray(Vec(0.0, 0.0, 3.0), Vec(0.0, 0.0, -1.0));
    result = bbox.hit(ray);
    ASSERT_EQ(result, true);

    // shoot a ray from inside the bounding box
    ray = Ray(Vec(0.0, 0.0, 0.0), normalize(Vec(1.0, 2.0, 0.0)));
    result = bbox.hit(ray);
    ASSERT_EQ(result, true);
}

TEST(bbox_object_test_case, test_correct_empty_bbox) {
    Vec corner0(-1.0, 0.0, -1.0), corner1(1.0, 2.0, 1.0);
    BBox bbox(corner0, corner1);

    bbox.empty();
    ASSERT_EQ(bbox.corner0.x, INF);
    ASSERT_EQ(bbox.corner0.y, INF);
    ASSERT_EQ(bbox.corner0.z, INF);
    ASSERT_EQ(bbox.corner1.x, -INF);
    ASSERT_EQ(bbox.corner1.y, -INF);
    ASSERT_EQ(bbox.corner1.z, -INF);
}

