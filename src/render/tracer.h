#ifndef TRACER_H
#define TRACER_H

#include "../render/bvh.h"
#include "../render/object.h"
#include "../render/random.h"
#include "../render/ray.h"
#include "../render/vec.h"

const Color BACKGROUND_COLOR = BLACK;
const double AMBIENT_COEF = 0.05;  // environment light
const int DEPTH = 4;
const int DEPTH_LIMIT = 16;
const double IOR = 1.5;
constexpr double M_PI = 3.14159265358979323846;

bool intersect_objects(const Ray& ray, const std::vector<Object*>& objects, const BVH& bvh,
	Intersection& intersection);

// Calculate the orthogonal basis
// Assume the w is an unit vector
inline std::pair<Vec, Vec> create_orthonormal_basis(const Vec w)
{
	Vec u;
	if (fabs(w.x) > EPS)
	{
		u = normalize(cross(Vec(0.0, 1.0, 0.0), w));
	}
	else
	{
		u = normalize(cross(Vec(1.0, 0.0, 0.0), w));
	}
	Vec v = cross(w, u);
	return std::make_pair(u, v);
}

inline Vec sample_from_hemisphere(const Vec& u, const Vec& v, const Vec& w,
	UniformRealGenerator& rnd, const double e)
{
	const double phi = 2 * M_PI * rnd();
	const double cos_phi = std::cos(phi);
	const double sin_phi = std::sin(phi);

	const double cos_theta = std::pow(rnd(), 1.0 / (e + 1.0));
	const double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

	return sin_theta * cos_phi * u + sin_theta * sin_phi * v + cos_theta * w;
}


inline Ray calc_reflection_ray(const Vec& dir, const Hitpoint& hitpoint)
{
	return Ray(hitpoint.position, dir - hitpoint.normal * 2.0 * dot(hitpoint.normal, dir));
}

Color trace_for_debug(const Ray& ray, const std::vector<Object*>& objects, const BVH& bvh);

Color ray_trace(const Ray& ray, const std::vector<Object*>& objects,
	const std::vector<Object*> lights, const BVH& bvh);

Color path_trace(const Ray& ray, const std::vector<Object*>& objects, const BVH& bvh,
	Texture* ibl_ptr, UniformRealGenerator& rnd, const int depth);

#endif
