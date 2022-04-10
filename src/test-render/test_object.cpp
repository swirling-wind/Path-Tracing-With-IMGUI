#include "pch.h"

#include "constant.h"
#include "object.h"
#include "ray.h"

Material material_magenta{ new ConstantTexture(Color(1.0, 0.0, 1.0)), Color(0.0, 0.0, 0.0),
                           ReflectionType::DIFFUSE };

TEST(object_test_case, test_correct_bbox_init_for_shpere) {
    Sphere sphere;
    ASSERT_EQ(sphere.bbox.corner0, Vec(-1.0 - EPS, -1.0 - EPS, -1.0 - EPS));
    ASSERT_EQ(sphere.bbox.corner1, Vec(1.0 + EPS, 1.0 + EPS, 1.0 + EPS));

    const double radius = 2.0;
    const Vec center(1.0, 1.0, 1.0);
    sphere = Sphere(radius, center, &material_magenta);
    ASSERT_EQ(sphere.bbox.corner0, Vec(-1.0 - EPS, -1.0 - EPS, -1.0 - EPS));
    ASSERT_EQ(sphere.bbox.corner1, Vec(3.0 + EPS, 3.0 + EPS, 3.0 + EPS));
}

TEST(object_test_case, test_correct_intersect) {
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
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_plane_sute)
BOOST_AUTO_TEST_CASE(test_correct_bbox_init) {
    Plane plane;
    BOOST_CHECK(plane.bbox.corner0 == Vec(-INF, -EPS, -INF));
    BOOST_CHECK(plane.bbox.corner1 == Vec(INF, EPS, INF));

    const Vec origin(0.0, 1.0, 0.0);
    const Vec normal(0.0, 1.0, 0.0);
    plane = Plane(origin, normal, &material_magenta);
    BOOST_CHECK(plane.bbox.corner0 == Vec(-INF, 1.0 - EPS, -INF));
    BOOST_CHECK(plane.bbox.corner1 == Vec(INF, 1.0 + EPS, INF));
}

BOOST_AUTO_TEST_CASE(test_correct_intersect) {
    const Vec origin(0.0, 0.0, 0.0);
    const Vec normal(0.0, 1.0, 0.0);

    Plane plane(origin, normal, &material_magenta);

    Hitpoint hitpoint;

    Ray ray(Vec(0.0, 1.0, 0.0), Vec(0.0, 1.0, 0.0));
    bool result = plane.intersect(ray, hitpoint);
    BOOST_CHECK_EQUAL(result, false);
    BOOST_CHECK_EQUAL(hitpoint.distance, INF);
    BOOST_CHECK(hitpoint.normal == Vec(0.0, 0.0, 0.0));
    BOOST_CHECK(hitpoint.position == Vec(0.0, 0.0, 0.0));

    ray = Ray(Vec(0.0, 1.0, 0.0), Vec(0.0, -1.0, 0.0));
    result = plane.intersect(ray, hitpoint);
    BOOST_CHECK_EQUAL(result, true);
    BOOST_CHECK_EQUAL(hitpoint.distance, 1.0);
    BOOST_CHECK(hitpoint.normal == Vec(0.0, 1.0, 0.0));
    BOOST_CHECK(hitpoint.position == Vec(0.0, 0.0, 0.0));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_triangle_sute)
BOOST_AUTO_TEST_CASE(test_correct_bbox_init) {
    Triangle triangle;
    BOOST_CHECK(triangle.bbox.corner0 == Vec(-EPS, -EPS, -EPS));
    BOOST_CHECK(triangle.bbox.corner1 == Vec(1.0 + EPS, EPS, 1.0 + EPS));

    const Vec v0(-2.0, 0.0, 0.0), v1(2.0, 0.0, 0.0), v2(0.0, 2.0, 0.0);
    triangle = Triangle(v0, v1, v2, &material_magenta);
    BOOST_CHECK(triangle.bbox.corner0 == Vec(-2.0 - EPS, -EPS, -EPS));
    BOOST_CHECK(triangle.bbox.corner1 == Vec(2.0 + EPS, 2.0 + EPS, EPS));
}

BOOST_AUTO_TEST_CASE(test_correct_intersect) {
    const Vec v0(-1.0, 0.0, 0.0), v1(1.0, 0.0, 0.0), v2(0.0, 2.0, 0.0);
    const Triangle triangle(v0, v1, v2, &material_magenta);

    Hitpoint hitpoint;

    Ray ray(Vec(1.0, 2.0, 1.0), Vec(0.0, 0.0, -1.0));
    bool result = triangle.intersect(ray, hitpoint);
    BOOST_CHECK_EQUAL(result, false);
    BOOST_CHECK_EQUAL(hitpoint.distance, INF);
    BOOST_CHECK(hitpoint.normal == Vec(0.0, 0.0, 0.0));
    BOOST_CHECK(hitpoint.position == Vec(0.0, 0.0, 0.0));

    ray = Ray(Vec(0.0, 1.0, 1.0), Vec(0.0, 0.0, -1.0));
    result = triangle.intersect(ray, hitpoint);
    BOOST_CHECK_EQUAL(result, true);
    BOOST_CHECK_EQUAL(hitpoint.distance, 1.0);
    BOOST_CHECK(hitpoint.normal == Vec(0.0, 0.0, 1.0));
    BOOST_CHECK(hitpoint.position == Vec(0.0, 1.0, 0.0));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_box_sute)
BOOST_AUTO_TEST_CASE(test_correct_intersect) {
    const Vec corner0(-1.0, 1.0, -1.0);
    const Vec corner1(1.0, -1.0, 1.0);

    Box box(corner0, corner1, &material_magenta);
    Hitpoint hitpoint;

    Ray ray(Vec(0.0, 2.0, 3.0), Vec(0.0, 0.0, -1.0));
    bool result = box.intersect(ray, hitpoint);
    BOOST_CHECK_EQUAL(result, false);
    BOOST_CHECK_EQUAL(hitpoint.distance, INF);
    BOOST_CHECK(hitpoint.normal == Vec(0.0, 0.0, 0.0));
    BOOST_CHECK(hitpoint.position == Vec(0.0, 0.0, 0.0));

    ray = Ray(Vec(0.0, 0.0, 3.0), Vec(0.0, 0.0, -1.0));
    result = box.intersect(ray, hitpoint);
    BOOST_CHECK_EQUAL(result, true);
    BOOST_CHECK_EQUAL(hitpoint.distance, 2.0);
    BOOST_CHECK(hitpoint.normal == Vec(0.0, 0.0, 1.0));
    BOOST_CHECK(hitpoint.position == Vec(0.0, 0.0, 1.0));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_bbox_sute)
BOOST_AUTO_TEST_CASE(test_correct_hit) {
    const Vec corner0(-1.0, 1.0, -1.0);
    const Vec corner1(1.0, -1.0, 1.0);

    BBox bbox(corner0, corner1);

    // バウンディングボックスの外部からレイを飛ばす
    Ray ray(Vec(0.0, 2.0, 3.0), Vec(0.0, 0.0, -1.0));
    bool result = bbox.hit(ray);
    BOOST_CHECK_EQUAL(result, false);

    ray = Ray(Vec(0.0, 0.0, 3.0), Vec(0.0, 0.00001, -1.0));
    result = bbox.hit(ray);
    BOOST_CHECK_EQUAL(result, true);

    // 軸平行なレイ
    ray = Ray(Vec(0.0, 0.0, 3.0), Vec(0.0, 0.0, -1.0));
    result = bbox.hit(ray);
    BOOST_CHECK_EQUAL(result, true);

    // バウンディングボックスの内部からレイを飛ばす
    ray = Ray(Vec(0.0, 0.0, 0.0), normalize(Vec(1.0, 2.0, 0.0)));
    result = bbox.hit(ray);
    BOOST_CHECK_EQUAL(result, true);
}

BOOST_AUTO_TEST_CASE(test_correct_empty_bbox) {
    Vec corner0(-1.0, 0.0, -1.0), corner1(1.0, 2.0, 1.0);
    BBox bbox(corner0, corner1);

    bbox.empty();
    BOOST_CHECK_EQUAL(bbox.corner0.x, INF);
    BOOST_CHECK_EQUAL(bbox.corner0.y, INF);
    BOOST_CHECK_EQUAL(bbox.corner0.z, INF);
    BOOST_CHECK_EQUAL(bbox.corner1.x, -INF);
    BOOST_CHECK_EQUAL(bbox.corner1.y, -INF);
    BOOST_CHECK_EQUAL(bbox.corner1.z, -INF);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
