//! This module implements simple linear algebra primitives

#ifndef RT_MATH_HPP_
#define RT_MATH_HPP_

#include <cmath>

namespace rt::math {
    /// Generic 3-component vector class
    template <typename type>
    class vec3 {
    public:

        /// X component
        type x;

        /// Y component
        type y;

        /// Z component
        type z;


        /// Construct default 3-component vector
        vec3(): x(0), y(0), z(0) {

        }

        /// Construct 3-component vector from components
        vec3(type x, type y, type z): x(x), y(y), z(z) {

        }

        /// Construct 3-component vector from single number
        explicit vec3(type c): x(c), y(c), z(c) {

        }

        /// Addition
        vec3 operator+( const vec3 &v ) const {
            return vec3(x + v.x, y + v.y, z + v.z);
        }

        /// Substraction
        vec3 operator-( const vec3 &v ) const {
            return vec3(x - v.x, y - v.y, z - v.z);
        }

        /// Multiplication
        vec3 operator*( const vec3 &v ) const {
            return vec3(x * v.x, y * v.y, z * v.z);
        }

        /// Division
        vec3 operator/( const vec3 &v ) const {
            return vec3(x / v.x, y / v.y, z / v.z);
        }

        /// Dot product
        type dot( const vec3 &v ) const {
            return x * v.x + y * v.y + z * v.z;
        }

        /// Squared length
        type length2() const {
            return x * x + y * y + z * z;
        }

        /// Length
        type length() const {
            return std::sqrt(length2());
        }

        /// Cross product
        vec3 cross( const vec3 &v ) const {
            return vec3(
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            );
        }

        /// With unit length
        vec3 normalized() const {
            return *this * vec3(1.0f / length());
        }
    };
}

#endif // !defined(RT_MATH_HPP_)

// rt_math.hpp
