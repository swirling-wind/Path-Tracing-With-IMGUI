#ifndef LOADER_H
#define LOADER_H

#include <string>
#include "../render/instant_surface.h"

using namespace instant_renderer;

namespace instant_renderer
{
    inline bool load_ply_file(const std::string file_path, surface* surface)
    {
        std::ifstream infile(file_path);
        std::string line;

        // Check file format
        getline(infile, line);
        if (line != "ply")
        {
            std::cout << "The file is not ply format." << std::endl;
            return false;
        }

        // Check the ASCII format
        std::string keyword, fileformat;
        int version;
        infile >> keyword >> fileformat >> version;
        if (fileformat != "ascii")
        {
            std::cout << "The file is not ascii format." << std::endl;
            return false;
        }

        std::vector<std::string> sep_s;
        size_t n_vertices = 0;
        std::vector<std::string> vertex_property;
        size_t n_triangles = 0;
        std::vector<std::string> triangle_property;
        std::string type;
        while (getline(infile, line))
        {
            sep_s = split_reg(line, " +");
            keyword = sep_s[0];

            if (keyword == "end_header") break;
            if (keyword == "comment") continue;

            if (keyword == "element")
            {
                type = sep_s[1];
                if (type == "vertex")
                {
                    n_vertices = stoi(sep_s[2]);
                }
                else if (type == "face")
                {
                    n_triangles = stoi(sep_s[2]);
                }
            }
            else if (keyword == "property")
            {
                if (type == "vertex")
                {
                    vertex_property.push_back(sep_s[2]);
                }
                else if (type == "face")
                {
                    triangle_property.push_back(sep_s[1]);
                }
            }
        }

        double min_x = INF, min_y = INF, min_z = INF;
        double max_x = -INF, max_y = -INF, max_z = -INF;

        // Read vertexes
        for (size_t i = 0; i < n_vertices; i++)
        {
            getline(infile, line);
            sep_s = split_reg(line, " +");

            // Calculate vertexes' coordinates
            double x = stod(sep_s[0]), y = stod(sep_s[1]), z = stod(sep_s[2]);
            surface->vertices.push_back(Vec(x, y, z));

            // Calculate the minimum and maximum of bounding
            if (x < min_x) min_x = x;
            if (y < min_y) min_y = y;
            if (z < min_z) min_z = z;
            if (x > max_x) max_x = x;
            if (y > max_y) max_y = y;
            if (z > max_z) max_z = z;
        }

        // Construct curves according to vertex coordinates
        int n_vert;
        int idx0, idx1, idx2;
        for (size_t i = 0; i < n_triangles; i++)
        {
            getline(infile, line);
            sep_s = split_reg(line, " +");

            // Ignore those polygons which have more than 4 vertexes
            n_vert = stoi(sep_s[0]);
            if (n_vert == 3)
            {
                idx0 = std::stoi(sep_s[1]);
                idx1 = std::stoi(sep_s[2]);
                idx2 = std::stoi(sep_s[3]);
                surface->triangles.push_back(std::make_tuple(idx0, idx1, idx2));
            }
        }

        infile.close();

        // Set the bounding
        // Vec corner0(min_x - EPS, min_y - EPS, min_z - EPS);
        // Vec corner1(max_x + EPS, max_y + EPS, max_z + EPS);
        // surface->bbox.set_corner(corner0, corner1);

        // Set the center of coordinates
        // surface->center = (corner0 + corner1) / 2.0;

        // Calculate the normal line
        surface->compute_normals();

        // Create a polygon bounding
        surface->compute_bboxes();

        // Construct the space data structure
        surface->construct();

        return surface;
    }
}
#endif
