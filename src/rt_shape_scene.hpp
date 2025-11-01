//! Shape that is used for combining other shapes

#ifndef RT_SHAPE_SCENE_HPP_
#define RT_SHAPE_SCENE_HPP_

#include "rt_shape_common.hpp"

namespace rt::shape {
    /// Scene shape
    class scene : public shape {
    public:
        /// Scene constructor
        scene() {

        }

        /// Add new shape to the scene
        void add_shape(std::unique_ptr<shape> new_shape) {
            shapes.emplace_back(std::move(new_shape));
        }

        /// `add_shape` method wrapped in nice operator
        scene & operator<<(std::unique_ptr<shape> new_shape) {
            add_shape(std::move(new_shape));
            return *this;
        }

        /// Check shape-ray intersection
        virtual bool check_intersection(ray r) const override {
            for (auto &shape : shapes)
                if (shape->check_intersection(r))
                    return true;
            return false;
        }

        /// Check shape-ray intersection writing intersection info in `intr`
        virtual bool intersect(ray r, intersection &intr) const override {
            intersection best {};

            for (auto &shape : shapes)
                if (shape->intersect(r, intr) && intr.distance <= best.distance)
                    best = intr;

            if (best.distance == intersection::INF_DISTANCE)
                return false;

            intr = best;
            return true;
        }

    private:
        /// Set of scene shapes
        std::vector<std::unique_ptr<shape>> shapes {};
    };
}

#endif // !defined(RT_SHAPE_SCENE_HPP_)

// rt_shape_scene.hpp
