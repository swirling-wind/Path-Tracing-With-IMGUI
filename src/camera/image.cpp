#include "../camera/image.h"
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include "pixel-operators.h"
#include "../common/util.h"
#include "../color/srgb.h"
#include "../common/histogram.h"
#include "../render/gui_param_tool.h"
#include <glm/gtx/component_wise.hpp>

Image::Image(const nlohmann::json &j)
{
    width = j.at("width");
    height = j.at("height");
    num_pixels = width * height;
    blob_ = std::vector<glm::dvec3>(num_pixels, glm::dvec3());

    without_tonemap_nor_auto_exposure_ = getOptional(j, "plain", false);

    const double exposure_compensation = getOptional(j, "exposure_compensation", 0.0);
    const double gain_compensation = getOptional(j, "gain_compensation", 0.0);

    exposure_scale_ = std::pow(2, exposure_compensation);
    gain_scale_ = std::pow(2, gain_compensation);

    std::string tonemapper = getOptional<std::string>(j, "tonemapper", "HABLE");
    std::transform(tonemapper.begin(), tonemapper.end(), tonemapper.begin(), toupper);

    if (without_tonemap_nor_auto_exposure_)
        tonemap_ = linear;
    else
        if (tonemapper == "ACES")
            tonemap_ = filmicACES;
        else 
            tonemap_ = filmicHable;
}

inline double clamp(double x, double min, double max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

std::vector<glm::vec3> Image::get_adjusted_float_blob() const
{
    const double exposure_factor = without_tonemap_nor_auto_exposure_ ? 1.0 : get_exposure_in_50_percentage() * exposure_scale_;
    const double gain_factor = without_tonemap_nor_auto_exposure_ ? 1.0 : get_gain_to_position_histogram_to_right(exposure_factor) * gain_scale_;

    std::vector<glm::vec3> float_blob;
    float_blob.reserve(blob_.size());
    for (auto& pixel : blob_)
    {
        float_blob.emplace_back(sRGB::gammaCompress(tonemap_(pixel * exposure_factor) * gain_factor));
    }
    return float_blob;
}

void Image::save(const std::string& filename) const
{
    double exposure_factor = without_tonemap_nor_auto_exposure_ ? 1.0 : get_exposure_in_50_percentage() * exposure_scale_;
    double gain_factor = without_tonemap_nor_auto_exposure_ ? 1.0 : get_gain_to_position_histogram_to_right(exposure_factor) * gain_scale_;

    // Save in .ppm
    std::ofstream output_image( gui_constant_params::image_file_path / (filename + ".ppm"));
    output_image << "P3\n" << width << " " << height << "\n255\n";
    for (const auto& pixel : blob_)
    {
        auto compressed_pixel = sRGB::gammaCompress(tonemap_(pixel * exposure_factor) * gain_factor);
        output_image << static_cast<int>(256 * clamp(compressed_pixel.r, 0.0, 0.9999)) << ' '
            << static_cast<int>(256 * clamp(compressed_pixel.g, 0.0, 0.9999)) << ' '
            << static_cast<int>(256 * clamp(compressed_pixel.b, 0.0, 0.9999)) << '\n';
    }
    std::cerr << "\nPPM save Done.\n";

    // Save in .tga
    HeaderTGA header(static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    std::ofstream out_tonemapped(gui_constant_params::image_file_path / (filename + ".tga"), std::ios::binary);
    out_tonemapped.write(reinterpret_cast<char*>(&header), sizeof(header));
    for (const auto& p : blob_) //Image::Blob <vec3>
    {
        auto fp = truncate(sRGB::gammaCompress(tonemap_(p * exposure_factor) * gain_factor));
        out_tonemapped.write(reinterpret_cast<char*>(fp.data()), fp.size() * sizeof(uint8_t));
    }
    out_tonemapped.close();
    std::cerr << "\nTGA save Done.\n";
}

glm::dvec3& Image::operator()(size_t col, size_t row)
{
    return blob_[row * width + col];
}

double Image::get_exposure_in_50_percentage() const
{
    std::vector<double> brightness(blob_.size());
    for (size_t i = 0; i < blob_.size(); i++)
    {
        brightness[i] = compAdd(blob_[i]) / 3.0;
    }
    const Histogram histogram(brightness, 65536);
    const double intensity_level = histogram.level(0.5);
    return intensity_level > 0.0 ? 0.5 / intensity_level : 1.0;
}

double Image::get_gain_to_position_histogram_to_right(double exposure_factor) const
{
    std::vector<double> brightness(blob_.size());
    for (size_t i = 0; i < blob_.size(); i++)
    {
        brightness[i] = compAdd(tonemap_(blob_[i] * exposure_factor)) / 3.0;
    }
    const Histogram histogram(brightness, 65536);
    const double intensity_level = histogram.level(0.99);
    return intensity_level > 0.0 ? 0.99 / intensity_level : 1.0;
}
