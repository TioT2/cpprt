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
            return false;
        }

    private:

        /// Sphere center
        vec3 center;

        /// Squared sphere radius
        float radius2;

        /// Sphere material
        std::shared_ptr<material> mtl;
    };
}

#endif // !defined(RT_SHAPE_SPHERE_HPP_)

// rt_shape_sphere.hpp
