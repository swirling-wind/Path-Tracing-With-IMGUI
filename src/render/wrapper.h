#ifndef WRAPPER_IMAGE_H
#define WRAPPER_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#include <string>
#include "../render/utility.h"
#include "../render/vec.h"
#include "src/libs/stb/stb_image.h"
using namespace instant_renderer;

namespace instant_renderer
{
    inline void load_rgb_image_file(const std::string& file_path, Image& image)
    {
        int x, y, n;
        unsigned char* head_of_image_data = stbi_load(file_path.c_str(), &x, &y, &n, 0);

        image.set_image_size(x, y);
        const int ch = 3;
        for (int i = 0; i < y; i++)
        {
            for (int j = 0; j < x; j++)
            {
                const double r = *(head_of_image_data + (ch * (i * x + j))) / 256.0;  // NOLINT(bugprone-implicit-widening-of-multiplication-result)
                const double g = *(head_of_image_data + (ch * (i * x + j) + 1)) / 256.0; // NOLINT(bugprone-implicit-widening-of-multiplication-result)
                const double b = *(head_of_image_data + (ch * (i * x + j) + 2)) / 256.0; // NOLINT(bugprone-implicit-widening-of-multiplication-result)
                image.set_color(Color(r, g, b), i, j);
            }
        }
    }
}
#endif
