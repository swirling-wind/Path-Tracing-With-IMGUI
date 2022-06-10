#pragma once

#include <atomic>
#include <vector>
#include <glm/glm.hpp>

class Film
{
    using vec_with_2_int = glm::vec<2, int64_t>;

public:
    Film() = default;
    Film(size_t width, size_t height);

    void deposit(const glm::dvec2& p, const glm::dvec3& v);
    [[nodiscard]] glm::dvec3 scan(size_t col, size_t row) const;

private:
    struct Splat
    {
        void update(const glm::dvec3& v);
        glm::dvec3 get() const;

    private:
        std::atomic<double> rgb_sum_[3];
        std::atomic<double> weight_sum_;
    };

    double radius_;
    size_t width_, height_;
    std::vector<Splat> blob_;
};
