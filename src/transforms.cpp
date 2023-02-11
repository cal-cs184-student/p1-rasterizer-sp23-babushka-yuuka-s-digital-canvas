#include "transforms.h"

#include "CGL/matrix3x3.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

namespace CGL {

Vector2D operator*(const Matrix3x3 &m, const Vector2D &v) {
	Vector3D mv = m * Vector3D(v.x, v.y, 1);
	return Vector2D(mv.x / mv.z, mv.y / mv.z);
}

Matrix3x3 translate(float dx, float dy) {
	// Part 3: Fill this in.
	Matrix3x3 m(1, 0, dx, 0, 1, dy, 0, 0, 1);

	return m;
}

Matrix3x3 scale(float sx, float sy) {
	// Part 3: Fill this in.
	Matrix3x3 m(sx, 0, 0, 0, sy, 0, 0, 0, 1);

	return m;
}

// The input argument is in degrees counterclockwise
Matrix3x3 rotate(float deg) {
	// Part 3: Fill this in.
	float sin_deg = std::sin(radians(deg));
	float cos_deg = std::cos(radians(deg));
	Matrix3x3 m(cos_deg, -sin_deg, 0, sin_deg, cos_deg, 0, 0, 0, 1);

	return m;
}

}
