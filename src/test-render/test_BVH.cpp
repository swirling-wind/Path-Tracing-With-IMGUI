#include "pch.h"

#include "../render/bvh.h"
#include "../render/object.h"
#include "../render/vec.h"
using namespace instant_renderer;

TEST(BVH_test_case, test_correct_calculate_bbox_area)
{
    //GTEST_FAIL();
    Vec corner0(-1.0, 0.0, -1.0), corner1(1.0, 2.0, 1.0);
    BBox bbox(corner0, corner1);

    double result = calc_bbox_area(bbox);
    ASSERT_EQ(result, 24.0);
}


TEST(BVH_test_case, test_correct_merge_box) {
    Vec corner0(-1.0, 0.0, -1.0), corner1(1.0, 2.0, 1.0);
    BBox bbox1(corner0, corner1);
    BBox bbox2(corner0 + 2.0, corner1 + 2.0);
    BBox result;

    merge_bbox(bbox1, bbox2, result);
    ASSERT_EQ(result.corner0, Vec(-1.0, 0.0, -1.0));
    ASSERT_EQ(result.corner1, Vec(3.0, 4.0, 3.0));
    ASSERT_EQ(result.center, Vec(1.0, 2.0, 1.0));
}

TEST(BVH_test_case, test_correct_bvh_enclosing_bbox) {
    BBox bbox1(Vec(-1.0, 0.0, -1.0), Vec(0.0, 1.0, 0.0));
    BBox bbox2(Vec(0.0, 1.0, 0.0), Vec(1.0, 2.0, 1.0));
    BBox bbox3(Vec(1.0, 2.0, 1.0), Vec(2.0, 3.0, 2.0));
    std::vector<std::pair<int, BBox*>> bboxes;
    bboxes.push_back(std::make_pair(0, &bbox1));
    bboxes.push_back(std::make_pair(1, &bbox2));
    bboxes.push_back(std::make_pair(2, &bbox3));

    BVH bvh;
    BBox result;
    result.empty();
    bvh.enclosing_bbox(bboxes, result);

    ASSERT_EQ(result.corner0, Vec(-1.0, 0.0, -1.0));
    ASSERT_EQ(result.corner1, Vec(2.0, 3.0, 2.0));
    ASSERT_EQ(result.center, Vec(0.5, 1.5, 0.5));
}

TEST(BVH_test_case, test_correct_bvh_make_leaf) {
    BBox bbox1(Vec(-1.0, 0.0, -1.0), Vec(0.0, 1.0, 0.0));
    BBox bbox2(Vec(0.0, 1.0, 0.0), Vec(1.0, 2.0, 1.0));
    BBox bbox3(Vec(1.0, 2.0, 1.0), Vec(2.0, 3.0, 2.0));
    std::vector<std::pair<int, BBox*>> bboxes;
    bboxes.push_back(std::make_pair(1000, &bbox1));
    bboxes.push_back(std::make_pair(20, &bbox2));
    bboxes.push_back(std::make_pair(500, &bbox3));

    BVH bvh;
    BvhNode result;
    bvh.make_leaf(bboxes, result);

    ASSERT_EQ(result.child_node_idx.first, -1);
    ASSERT_EQ(result.child_node_idx.second, -1);
    ASSERT_EQ(result.target_indices[0], 1000);
    ASSERT_EQ(result.target_indices[1], 20);
    ASSERT_EQ(result.target_indices[2], 500);
}

TEST(BVH_test_case, test_correct_bvh_axis_decending_sort) {
    BBox bbox1(Vec(-1.0, 0.0, -1.0), Vec(0.0, 1.0, 0.0));
    BBox bbox2(Vec(1.0, 2.0, 1.0), Vec(2.0, 3.0, 2.0));
    BBox bbox3(Vec(0.0, 1.0, 0.0), Vec(1.0, 2.0, 1.0));

    std::vector<std::pair<int, BBox*>> bboxes;
    bboxes.push_back(std::make_pair(0, &bbox1));
    bboxes.push_back(std::make_pair(1, &bbox2));
    bboxes.push_back(std::make_pair(2, &bbox3));

    BVH bvh;
    bvh.axis_decending_sort(bboxes, 0);
    ASSERT_EQ(bboxes[0].first, 1);
    ASSERT_EQ(bboxes[1].first, 2);
    ASSERT_EQ(bboxes[2].first, 0);

    bvh.axis_decending_sort(bboxes, 1);
    ASSERT_EQ(bboxes[0].first, 1);
    ASSERT_EQ(bboxes[1].first, 2);
    ASSERT_EQ(bboxes[2].first, 0);

    bvh.axis_decending_sort(bboxes, 2);
    ASSERT_EQ(bboxes[0].first, 1);
    ASSERT_EQ(bboxes[1].first, 2);
    ASSERT_EQ(bboxes[2].first, 0);

}

