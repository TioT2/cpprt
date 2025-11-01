//! Sphere shape implementation file

#ifndef RT_SHAPE_SPHERE_HPP_
#define RT_SHAPE_SPHERE_HPP_

#include "rt_shape_common.hpp"

// Shape namespace
namespace rt::shape {
    /// Sphere shape
    class sphere : public shape {
    public:

        /// Sphere constructor
        sphere(vec3 origin, float radius, std::shared_ptr<material> material) :
            center(origin),
            inv_radius(1.0f / radius),
            radius2(radius * radius),
            mtl(material)
        {

        }

        /// Check for (any) intersection
        virtual bool check_intersection(ray r) const override {
            vec3 delta = center - r.origin;
            float proj = delta.dot(r.direction);
            float dist = radius2 - delta.length2() + proj * proj;

            return dist >= 0.0f && proj + std::sqrt(dist) >= 0.0f;
        }

        /// 'Deep' intersection
        virtual bool intersect(ray r, intersection &intr) const override {
            vec3 delta = center - r.origin;
            float proj = delta.dot(r.direction);
            float dist = radius2 - delta.length2() + proj * proj;

            if (dist < 0.0f)
                return false;
            dist = std::sqrt(dist);

            if (proj - dist >= 0.0f)
                intr.distance = proj - dist;
            else if (proj + dist >= 0.0f)
                intr.distance = proj + dist;
            else
                return false;

            intr.normal = (r.at(intr.distance) - center) * vec3(inv_radius);
            intr.hit_material = mtl;

            return true;
        }

    private:

        /// Sphere center
        vec3 center;

        /// Sphere radius
        float inv_radius;

        /// Squared sphere radius
        float radius2;

        /// Sphere material
        std::shared_ptr<material> mtl;
    };
}

#endif // !defined(RT_SHAPE_SPHERE_HPP_)

// rt_shape_sphere.hpp
