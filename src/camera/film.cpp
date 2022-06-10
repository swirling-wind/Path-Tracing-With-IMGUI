#include "../camera/film.h"

#include "../camera/filter.h"
#include "../common/util.h"

Film::Film() = default;

Film::Film(size_t width, size_t height)
    : blob_(width * height), radius_(0.5), two_inv_radius_(2.0 / radius_),
    inv_dx_(0.0), width_(width), height_(height), filter_function_(Filter::box)
{ }

void Film::deposit(const glm::dvec2 & p, const glm::dvec3 & v)
{   // To save the pixel
    // p: glm::dvec2 px(x + sobol->u[0], y + sobol->u[1]);
    // v:  integrator->sampleRay(ray) {return radiance;}

    const vec_with_2_int min_boundary = glm::max(vec_with_2_int(p + 0.5 - radius_), vec_with_2_int(0));
    const vec_with_2_int max_boundary = glm::min(vec_with_2_int(p - 0.5 + radius_), vec_with_2_int(width_ - 1, height_ - 1));

    // Lazy but general and about as fast as can be
    thread_local std::vector<double> weights_x; weights_x.clear();
    for (int64_t x = min_boundary.x; x <= max_boundary.x; x++)
        weights_x.push_back(filter(x + 0.5 - p.x));

    for (int64_t y = min_boundary.y; y <= max_boundary.y; y++)
    {
        const double weight_y = filter(y + 0.5 - p.y);
        for (int64_t x = min_boundary.x; x <= max_boundary.x; x++)
        {
            blob_[y * width_ + x].update(v, weight_y * weights_x[x - min_boundary.x]);
        }
    }
}

glm::dvec3 Film::scan(size_t col, size_t row) const
{
    return blob_[row * width_ + col].get();
}

double Film::filter(double x) const
{
    if (filter_cache_.empty())
    {
        return filter_function_(two_inv_radius_ * std::abs(x));
    }
    else
    {
        // nearest neighbor index = round(inv_dx * |x|) = floor(inv_dx * |x| + 0.5)
        return filter_cache_[static_cast<size_t>(inv_dx_ * std::abs(x) + 0.5)];
    }
}

void Film::Splat::update(const glm::dvec3 & v, double weight)
{
    rgb_sum_[0] = rgb_sum_[0] + v[0] * weight;
    rgb_sum_[1] = rgb_sum_[1] + v[1] * weight;
    rgb_sum_[2] = rgb_sum_[2] + v[2] * weight;
    weight_sum_ = weight_sum_ + weight;
}

glm::dvec3 Film::Splat::get() const
{
    glm::dvec3 res(rgb_sum_[0].load(), rgb_sum_[1].load(), rgb_sum_[2].load());
    double w = weight_sum_.load();
    if (w == 0.0) return glm::dvec3(0.0);
    return glm::max(res / w, 0.0);
}
