#include "film.h"

Film::Film(size_t width, size_t height)
    : radius_(0.5), width_(width),  height_(height), blob_(width * height)
{ }

void Film::deposit(const glm::dvec2 & p, const glm::dvec3 & v)
{   
    const vec_with_2_int min_boundary = max(vec_with_2_int(p + 0.5 - radius_), vec_with_2_int(0));
    const vec_with_2_int max_boundary = min(vec_with_2_int(p - 0.5 + radius_), vec_with_2_int(width_ - 1, height_ - 1));
    for (int64_t y = min_boundary.y; y <= max_boundary.y; y++)
    {
        for (int64_t x = min_boundary.x; x <= max_boundary.x; x++)
        {
            blob_[y * width_ + x].update(v);
        }
    }
}

glm::dvec3 Film::scan(const size_t col, const size_t row) const
{
    return blob_[row * width_ + col].get();
}

void Film::Splat::update(const glm::dvec3& v)
{
    rgb_sum_[0] = rgb_sum_[0] + v[0];
    rgb_sum_[1] = rgb_sum_[1] + v[1] ;
    rgb_sum_[2] = rgb_sum_[2] + v[2];
    weight_sum_ = weight_sum_ + 1.0;
}

glm::dvec3 Film::Splat::get() const
{
    glm::dvec3 res(rgb_sum_[0].load(), rgb_sum_[1].load(), rgb_sum_[2].load());
    const double w = weight_sum_.load();
    if (w == 0.0) return glm::dvec3(0.0);
    return max(res / w, 0.0);
}
