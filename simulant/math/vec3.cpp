#include "../types.h"
#include "../random.h"

#include "vec3.h"

namespace smlt {

const Vec3 Vec3::NEGATIVE_X = Vec3(-1, 0, 0);
const Vec3 Vec3::POSITIVE_X = Vec3(1, 0, 0);
const Vec3 Vec3::NEGATIVE_Y = Vec3(0, -1, 0);
const Vec3 Vec3::POSITIVE_Y = Vec3(0, 1, 0);
const Vec3 Vec3::POSITIVE_Z = Vec3(0, 0, 1);
const Vec3 Vec3::NEGATIVE_Z = Vec3(0, 0, -1);

std::ostream& operator<<(std::ostream& stream, const Vec3& vec) {
    stream << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
    return stream;
}

Vec3::Vec3(const Vec2 &v2, float z):
    x(v2.x), y(v2.y), z(z) {

}

Vec3 Vec3::rotated_by(const Quaternion &q) const {
    return q * (*this);
}

Vec3 Vec3::rotated_by(const Mat3 &rot) const {
    return rot.transform_vector(*this);
}

Vec3 operator*(float lhs, const Vec3& rhs) {
    smlt::Vec3 result = rhs;
    result *= lhs;
    return result;
}

Vec3 operator/(float lhs, const Vec3& rhs) {
    smlt::Vec3 result = rhs;
    result /= lhs;
    return result;
}

Vec3 Vec3::random_deviant(const Degrees& angle, const Vec3 up) const {
    //Lovingly adapted from ogre
    Vec3 new_up = (up == Vec3()) ? perpendicular() : up;

    Quaternion q(*this, Radians(RandomGenerator().float_in_range(0, 1) * (PI * 2.0)));

    new_up = new_up * q;

    q = Quaternion(new_up, angle);

    return *this * q;

}

Vec3 Vec3::operator*(const Quaternion &rhs) const {
    return rhs.rotate_vector(*this);
}

Vec3 Vec3::rotated_by(const Mat4 &rot) const {
    // Avoid going through the operator for performance
    const float* m = &rot[0];

    return Vec3(
        x * m[0] + y * m[4] + z * m[8],
        x * m[1] + y * m[5] + z * m[9],
        x * m[2] + y * m[6] + z * m[10]
    );
}

Vec3 Vec3::transformed_by(const Mat4 &trans) const {
    // Avoid going through the operator for performance
    const float* m = &trans[0];

    return Vec3(
        x * m[0] + y * m[4] + z * m[8] + 1.0 * m[12],
        x * m[1] + y * m[5] + z * m[9] + 1.0 * m[13],
        x * m[2] + y * m[6] + z * m[10] + 1.0 * m[14]
    );
}

Vec3 Vec3::perpendicular() const {
    //Lovingly adapted from Ogre
    static const float square_zero = (float)(1e-06 * 1e-06);
    Vec3 perp = this->cross(Vec3(1, 0, 0));

    if(perp.length_squared() < square_zero) {
        //This vector is the X-axis, so use another
        perp = this->cross(Vec3(0, 1, 0));
    }
    return perp.normalized();
}

smlt::Vec3 operator-(const smlt::Vec3& vec) {
    return smlt::Vec3(-vec.x, -vec.y, -vec.z);
}


}
