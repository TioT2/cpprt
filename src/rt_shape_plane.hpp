//! Plane shape implementation file

#ifndef RT_SHAPE_PLANE_HPP_
#define RT_SHAPE_PLANE_HPP_

#include "rt_shape_common.hpp"

namespace rt::shape {
    /// Plane shape class
    class plane : public shape {
    public:

        /// Plane constructor
        plane(vec3 point, vec3 normal, std::shared_ptr<material> mtl):
            normal(normal),
            normal_origin(normal.dot(point)),
            mtl(std::move(mtl))
        {

        }

        /// Check shape-ray intersection
        virtual bool check_intersection(ray r) const override {
            return (normal_origin - normal.dot(r.origin)) / normal.dot(r.direction) > 0.0f;
        }

        /// Check shape-ray intersection, writing description in `intr` structure
        virtual bool intersect(ray r, intersection &intr) const override {
            float dist = (normal_origin - normal.dot(r.origin)) / normal.dot(r.direction);
            if (dist < 0.0f)
                return false;

            intr.distance = dist;
            intr.normal = normal;
            intr.hit_material = mtl;
            return true;
        }

    private:
        /// Normal vector
        vec3 normal;

        /// Dot product of normal and plane origin point
        float normal_origin;

        /// Plane material
        std::shared_ptr<material> mtl;
    };
}

#endif // !defined(RT_SHAPE_PLANE_HPP_)

// rt_shape_plane.hpp
