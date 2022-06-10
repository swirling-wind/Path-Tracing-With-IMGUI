#include "fresnel.h"

#include <glm/glm.hpp>

#include "tools/constexpr-math.h"
#include "tools/util.h"

double Fresnel::dielectric(double n1, double n2, double cos_theta)
{
    double g2 = pow2(n2 / n1) + pow2(cos_theta) - 1.0;

    if (g2 < 0.0) return 1.0;

    double g = std::sqrt(g2);
    double g_p_c = g + cos_theta;
    double g_m_c = g - cos_theta;

    return 0.5 * pow2(g_m_c / g_p_c) * (1.0 + pow2((g_p_c * cos_theta - 1.0) / (g_m_c * cos_theta + 1.0)));
}

glm::dvec3 Fresnel::conductor(double n1, ComplexIOR* n2, double cos_theta)
{
    double cos_theta2 = pow2(cos_theta);
    double sin_theta2 = 1.0 - cos_theta2;

    glm::dvec3 eta2 = pow2(n2->real / n1);
    glm::dvec3 eta_k2 = pow2(n2->imaginary / n1);

    glm::dvec3 t0 = eta2 - eta_k2 - sin_theta2;
    glm::dvec3 a2_p_b2 = glm::sqrt(pow2(t0) + 4.0 * eta2 * eta_k2);
    glm::dvec3 t1 = a2_p_b2 + cos_theta2;
    glm::dvec3 t2 = 2.0 * cos_theta * glm::sqrt(0.5 * (a2_p_b2 + t0));
    glm::dvec3 r_perpendicular = (t1 - t2) / (t1 + t2); 

    glm::dvec3 t3 = cos_theta2 * a2_p_b2 + pow2(sin_theta2);
    glm::dvec3 t4 = t2 * sin_theta2;
    glm::dvec3 r_parallel = r_perpendicular * (t3 - t4) / (t3 + t4); 

    return (r_parallel + r_perpendicular) * 0.5;
}

void from_json(const nlohmann::json &j, ComplexIOR &c)
{
    if (j.type() == nlohmann::json::value_t::object)
    {
        c.real = getOptional(j, "real", glm::dvec3(1.0));
        c.imaginary = getOptional(j, "imaginary", glm::dvec3(0.0));
    }
}
