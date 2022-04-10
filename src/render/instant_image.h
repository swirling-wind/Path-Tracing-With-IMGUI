#ifndef INSTANT_IMAGE_H
#define INSTANT_IMAGE_H

#include "vec.h"

constexpr int instant_image_width = 800;
constexpr int instant_image_height = 600;

struct instant_image
{
    Color color_array[instant_image_height * instant_image_width];
    const int image_width = 800;
    const int image_height = 600;

    instant_image() = default;

    void flip()
    {
        for (int i = 0; i < instant_image_height; i++) {
            for (int j = 0; j < instant_image_width; j++) {
                std::swap(color_array[i * instant_image_width + j],
                    color_array[i * instant_image_width + (instant_image_width - 1 - j)]);
            }
        }
    }

    [[nodiscard]] Color get_color(const int row, const int column) const {
        return color_array[row * instant_image_width + column];
    }

    void set_color(const Color& color, const int row, const int column) {
        color_array[row * instant_image_width + column] = color;
    }
};

#endif
