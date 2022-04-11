#include "pch.h"
#include <fstream>
#include <iostream>
#include <string>

#include "../render/utility.h"
using namespace instant_renderer;
TEST(utility_test_case, test_correct_clamp) {
    ASSERT_EQ(clamp(1.1), 1.0);
    ASSERT_EQ(clamp(-0.1), 0.0);
    ASSERT_EQ(clamp(0.5), 0.5);
}

TEST(utility_test_case, test_correct_to_int) {
    ASSERT_EQ(to_int(0.0), 0);
    ASSERT_EQ(to_int(1.0), 255);

    ASSERT_EQ(to_int(-0.1), 0);
    ASSERT_EQ(to_int(1.1), 255);
}

TEST(utility_test_case, test_correct_linspace) {
    double start = 123, end = 456;
    int n = 50;

    auto result = linspace(start, end, n);
    ASSERT_EQ(result.size(), n);
    ASSERT_EQ(result[0], start);
    ASSERT_EQ(result[n - 1], end);

    start = 5;
    end = 10;
    n = 5;
    result = linspace(start, end, n, false);
    ASSERT_EQ(result.size(), n);
    ASSERT_EQ(result[0], start);
    ASSERT_EQ(result[n - 1], 9.0);
}

TEST(utility_test_case, test_correct_save_and_load_ppm_file) {
    // save ppm file
    std::string filename = "test_image.ppm";
    int width_res = 4, height_res = 3;
    Color c = Color(1.0, 1.0, 1.0);
    Image image(width_res, height_res);

    for (int i = 0; i < image.height_res; i++) {
        for (int j = 0; j < image.width_res; j++) {
            image.set_color(c, i, j);
        }
    }
    save_ppm_file(filename, image);

    // load ppm file
    Image load_image = load_ascii_ppm_file(filename);

    ASSERT_EQ(load_image.width_res, width_res);
    ASSERT_EQ(load_image.height_res, height_res);
    for (int i = 0; i < load_image.height_res; i++) {
        for (int j = 0; j < load_image.width_res; j++) {
            ASSERT_EQ(load_image.get_color(i, j), c);
        }
    }
}

TEST(utility_test_case, test_correct_strip) {
    std::string s(" abc,def");
    std::string result = strip(s);
    ASSERT_EQ(result, "abc,def");

    s = "abc,def ";
    result = strip(s);
    ASSERT_EQ(result, "abc,def");

    s = " abc,def ";
    result = strip(s);
    ASSERT_EQ(result, "abc,def");

    s = "   abc,def   ";
    result = strip(s);
    ASSERT_EQ(result, "abc,def");

    s = ",abc,def,";
    result = strip(s, ",");
    ASSERT_EQ(result, "abc,def");
}

TEST(utility_test_case, test_correct_split) {
    std::string s = "abc,def";

    std::vector<std::string> result;
    result = split(s, ',');
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");

    s = "abc,def,";
    result = split(s, ',');
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");
    ASSERT_EQ(result[2], "");

    s = "abc,,def";
    result = split(s, ',');
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "");
    ASSERT_EQ(result[2], "def");

    s = "abc def";
    result = split(s, ' ');
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");

    s = "abc  def";
    result = split(s, ' ');
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "");
    ASSERT_EQ(result[2], "def");
}

TEST(utility_test_case, test_correct_split_reg) {
    std::string s = "abc,def";

    std::vector<std::string> result;
    result = split_reg(s, ",");
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");

    s = "abc,def,";
    result = split_reg(s, ",");
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");

    s = "abc,,def";
    result = split_reg(s, ",");
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "");
    ASSERT_EQ(result[2], "def");

    s = "abc def";
    result = split_reg(s, " ");
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");

    s = "abc  def";
    result = split_reg(s, " +");
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");

    s = "abc def ";
    result = split_reg(s, " +");
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], "abc");
    ASSERT_EQ(result[1], "def");
}
