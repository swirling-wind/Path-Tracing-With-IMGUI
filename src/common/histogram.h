#pragma once

#include <vector>

class Histogram
{
public:
    Histogram(const std::vector<double>& data, size_t num_bins);

    double level(double count_percentage) const;
    double bin_size;
    size_t data_size;
    std::vector<size_t> counts;
};
