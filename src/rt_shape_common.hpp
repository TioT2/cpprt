//! Set of shape common definitions

#ifndef RT_SHAPE_COMMON_HPP_
#define RT_SHAPE_COMMON_HPP_

#include "rt_common.hpp"
#include "rt_math.hpp"

namespace rt {
    /// Common vec3 definition
    using vec3 = math::vec3<float>;

    /// Ray class
    class ray {
    public:

        /// Ray origin
        vec3 origin;

        /// Ray direction (**must be** unit)
        vec3 direction;

        /// Calculate point interpreting ray as coordinate axis
        vec3 at(float x) const {
            return origin + direction * vec3(x);
        }
    };

    /// Material class
    class material {
    public:

        /// Construct material
        material(vec3 color): color(color) {

        }

        // Material base color (yeah...)
        vec3 color;
    };

    /// Intersection descriptor
    class intersection {
    public:
        /// Intersection normal
        vec3 normal = vec3(0.0f);

        /// Distance from ray origin to intersection point
        float distance = std::numeric_limits<float>::max();

        /// Object material pointer
        std::shared_ptr<material> hit_material = nullptr;
    };

    // Shape namespace
    namespace shape {

        /// Shape interface
        class shape {
        public:

            /// Check shape-ray intersection
            virtual bool check_intersection(ray r) const = 0;

            /// Check shape-ray intersection, writing description in `intr` structure
            virtual bool intersect(ray r, intersection &intr) const = 0;

            /// Shape destructor
            virtual ~shape() = default;
        };
    }
}

#endif // !defiend(RT_SHAPE_COMMON_HPP_)

// rt_shape_common.hpp
