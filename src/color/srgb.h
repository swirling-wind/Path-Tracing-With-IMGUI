#pragma once

#include <glm/vec2.hpp>

#include "../color/cie.h"

#include "../common/constexpr-math.h"

namespace sRGB
{
    inline glm::dvec3 gammaCompress(const glm::dvec3& in)
    {
        glm::dvec3 out;
        for (uint8_t c = 0; c < 3; c++)
        {
            out[c] = in[c] <= 0.0031308 ? 12.92 * in[c] : 1.055 * std::pow(in[c], 1.0 / 2.4) - 0.055;
        }
        return out;
    }

    inline glm::dvec3 gammaExpand(const glm::dvec3& in)
    {
        glm::dvec3 out;
        for (uint8_t c = 0; c < 3; c++)
        {
            out[c] = in[c] <= 0.04045 ? in[c] / 12.92 : std::pow((in[c] + 0.055) / 1.055, 2.4);
        }
        return out;
    }

    using float3_array = std::array<float, 3>;
    inline float3_array gammaExpand(const float3_array& in)
    {
        float3_array out;
        for (uint8_t c = 0; c < 3; c++)
        {
            out[c] = in[c] <= 0.04045 ? in[c] / 12.92 : std::pow((in[c] + 0.055) / 1.055, 2.4);
        }
        return out;
    }

}
