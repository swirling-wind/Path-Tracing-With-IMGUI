#include "../camera/film.h"

#include "../camera/filter.h"
#include "../common/util.h"

Film::Film() = default;

Film::Film(size_t width, size_t height)
    : blob(width * height), radius(0.5), two_inv_radius(2.0 / radius),
    inv_dx(0.0), width(width),
    height(height), filter_function(Filter::box)
{ }

void Film::deposit(const glm::dvec2 & p, const glm::dvec3 & v)
{   // To save the pixel
    // p: glm::dvec2 px(x + sobol->u[0], y + sobol->u[1]);
    // v:  integrator->sampleRay(ray) {return radiance;}

    // min and max to ensure not to render outside the image
    ivec2 min = glm::max(ivec2(p + 0.5 - radius), ivec2(0));
    ivec2 max = glm::min(ivec2(p - 0.5 + radius), ivec2(width - 1, height - 1));

    // Lazy but general and about as fast as can be
    thread_local std::vector<double> weights_x; weights_x.clear();
    for (int64_t x = min.x; x <= max.x; x++)
        weights_x.push_back(filter(x + 0.5 - p.x));

    for (int64_t y = min.y; y <= max.y; y++)
    {
        double weight_y = filter(y + 0.5 - p.y);
        for (int64_t x = min.x; x <= max.x; x++)
        {
            blob[y * width + x].update(v, weight_y * weights_x[x - min.x]);
        }
    }
}

glm::dvec3 Film::scan(size_t col, size_t row) const
{
    return blob[row * width + col].get();
}

double Film::filter(double x) const
{
    if (filter_cache.empty())
    {
        return filter_function(two_inv_radius * std::abs(x));
    }
    else
    {
        // nearest neighbor index = round(inv_dx * |x|) = floor(inv_dx * |x| + 0.5)
        return filter_cache[static_cast<size_t>(inv_dx * std::abs(x) + 0.5)];
    }
}

void Film::Splat::update(const glm::dvec3 & v, double weight)
{
    rgb_sum[0] = rgb_sum[0] + v[0] * weight;
    rgb_sum[1] = rgb_sum[1] + v[1] * weight;
    rgb_sum[2] = rgb_sum[2] + v[2] * weight;
    weight_sum = weight_sum + weight;
}

glm::dvec3 Film::Splat::get() const
{
    glm::dvec3 res(rgb_sum[0].load(), rgb_sum[1].load(), rgb_sum[2].load());
    double w = weight_sum.load();
    if (w == 0.0) return glm::dvec3(0.0);
    return glm::max(res / w, 0.0);
}
