#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "../render/material.h"
#include "../render/ray.h"


using namespace instant_renderer;

namespace instant_renderer
{
    struct BBox
    {
        Vec corner0;
        Vec corner1;
        Vec center;

        BBox();
        BBox(const Vec& corner0, const Vec& corner1);
        ~BBox();

        void set_corner(const Vec& corner0, const Vec& corner1);
        void empty();
        bool hit(const Ray& ray) const;
    };

    struct Object
    {
        Vec center;
        BBox bbox;
        Material* material_ptr;

        Object();
        Object(const Vec& center, const BBox& bbox, Material* material_ptr);
        virtual ~Object();

        void set_material(Material* material_ptr);
        virtual bool intersect(const Ray& ray, Hitpoint& hitpoint) const = 0;
    };

    struct Sphere : Object
    {
        double radius;

        Sphere();
        Sphere(const double radius, const Vec& center, Material* material_ptr);
        ~Sphere() override;

        bool intersect(const Ray& ray, Hitpoint& hitpoint) const override;
    };

    struct Plane final : Object
    {
        Vec normal;

        Plane();
        Plane(const Vec& center, const Vec& normal, Material* material_ptr);
        ~Plane() override;

        bool intersect(const Ray& ray, Hitpoint& hitpoint) const override;
    };

    struct Box final : Object
    {
        Vec corner0;
        Vec corner1;

        Box();
        Box(const Vec& corner0, const Vec& corner1, Material* material_ptr);
        ~Box() override;

        Vec get_normal(const int face_hit) const;
        bool intersect(const Ray& ray, Hitpoint& hitpoint) const override;
    };

    struct Triangle final : Object
    {
        Vec v0, v1, v2;
        Vec normal;

        Triangle();
        Triangle(const Vec& v0, const Vec& v1, const Vec& v2, Material* material_ptr);
        ~Triangle() override;

        bool intersect(const Ray& ray, Hitpoint& hitpoint) const override;
    };
}
#endif
