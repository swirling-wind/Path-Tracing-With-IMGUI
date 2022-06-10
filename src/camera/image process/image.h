#pragma once

#include <vector>
#include <cstdint>
#include <functional>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

struct Image
{
    Image() = default;
    explicit Image(const nlohmann::json &j);
    void save(const std::string& filename) const;

    [[nodiscard]] std::vector<glm::vec3> get_adjusted_float_blob() const;
    glm::dvec3& operator()(size_t col, size_t row);
    size_t width, height;
    size_t num_pixels;

private:
    [[nodiscard]] double get_exposure_in_50_percentage() const;
    [[nodiscard]] double get_gain_to_position_histogram_to_right(double exposure_factor) const;

    std::vector<glm::dvec3> blob_;
    double exposure_scale_, gain_scale_;

    std::function<glm::dvec3(const glm::dvec3&)> tonemap_;

    bool without_tonemap_nor_auto_exposure_;

    struct HeaderTGA
    {
        HeaderTGA(uint16_t width, uint16_t height)
            : width_(width), height_(height) {}

    private:
        uint8_t begin_[12] = { 0, 0, 2 };
        uint16_t width_;
        uint16_t height_;
        uint8_t end_[2] = { 24, 32 };
    };
};
