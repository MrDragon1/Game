#include "Quaternion.h"

#include "math/Vector.h"
#include "math/Matrix.h"

const Quaternion Quaternion::ZERO(0, 0, 0, 0);
const Quaternion Quaternion::IDENTITY(1, 0, 0, 0);

Quaternion::Quaternion() { operator=(ZERO); }
Quaternion::Quaternion(const Quaternion& q) {
	this->w = q.w;
	this->x = q.x;
	this->y = q.y;
	this->z = q.z;
}

Quaternion::Quaternion(float w, float x, float y, float z) {
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}
Quaternion::Quaternion(const Vector3& eulerAngle) {
	// Pitch Yaw Roll
	Vector3 c = Math::Cos(eulerAngle * 0.5f);
	Vector3 s = Math::Sin(eulerAngle * 0.5f);

	this->w = c.x * c.y * c.z + s.x * s.y * s.z;
	this->x = s.x * c.y * c.z - c.x * s.y * s.z;
	this->y = c.x * s.y * c.z + s.x * c.y * s.z;
	this->z = c.x * c.y * s.z - s.x * s.y * c.z;
	*this = Math::Normalize(*this);
}

Quaternion::Quaternion(const Matrix3& m) {
	*this = Matrix3ToQuaternion(m);
}
Quaternion::Quaternion(const Matrix4& m) {
	Quaternion(Matrix3(m));
}

float& Quaternion::operator[](int i) {
	assert(i < 4);
	switch (i) {
	default:
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	}
}

const float& Quaternion::operator[](int i) const {
	assert(i < 4);
	switch (i) {
	default:
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	}
}

Quaternion& Quaternion::operator=(const Quaternion& q) {
	this->w = q.w;
	this->x = q.x;
	this->y = q.y;
	this->z = q.z;
	return *this;
}

Quaternion& Quaternion::operator+=(const Quaternion& q) {
	this->w += q.w;
	this->x += q.x;
	this->y += q.y;
	this->z += q.z;
	return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& q) {
	this->w -= q.w;
	this->x -= q.x;
	this->y -= q.y;
	this->z -= q.z;
	return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& q) {
	Quaternion  p(*this);

	this->w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
	this->x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
	this->y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
	this->z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
	return *this;
}

Quaternion& Quaternion::operator*=(float v) {
	this->w *= v;
	this->x *= v;
	this->y *= v;
	this->z *= v;
	return *this;
}

Quaternion& Quaternion::operator/=(float v) {
	this->w /= v;
	this->x /= v;
	this->y /= v;
	this->z /= v;
	return *this;
}

Quaternion::operator Matrix3() const {
	return QuaternionToMatrix3();
}

Quaternion::operator Matrix4() const {
	return Matrix4(operator Matrix3());
}

Quaternion::operator Vector3() const {
	return QuaternionToEuler();
}

float Quaternion::Roll() const {
	float a = static_cast<float>(2) * (x * y + w * z);
	float b = w * w + x * x - y * y - z * z;

	if (Math::EpsilonEqual(Vector2(a, b), Vector2(0))) //avoid atan2(0,0) - handle singularity - Matiis
		return static_cast<float>(0);

	return static_cast<float>(atan2(a, b));
}
float Quaternion::Pitch() const {
	//return T(atan(T(2) * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z));
	float a = static_cast<float>(2) * (y * z + w * x);
	float b = w * w - x * x - y * y + z * z;

	if (Math::EpsilonEqual(Vector2(b, a), Vector2(0)))//avoid atan2(0,0) - handle singularity - Matiis
		return static_cast<float>(static_cast<float>(2) * atan2(x, w));

	return static_cast<float>(atan2(a, b));
}

float Quaternion::Yaw() const {
	return asin(Math::Clamp(static_cast<float>(-2) * (x * z - w * y), static_cast<float>(-1), static_cast<float>(1)));
}

Quaternion Quaternion::Matrix3ToQuaternion(const Matrix3& m) {
	float fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
	float fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
	float fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
	float fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

	int biggestIndex = 0;
	float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	float biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<float>(1)) * static_cast<float>(0.5);
	float mult = static_cast<float>(0.25) / biggestVal;

	switch (biggestIndex)
	{
	case 0:
		return Quaternion(biggestVal, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult);
	case 1:
		return Quaternion((m[1][2] - m[2][1]) * mult, biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult);
	case 2:
		return Quaternion((m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult);
	case 3:
		return Quaternion((m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal);
	default: // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
		assert(false);
		return Quaternion(1, 0, 0, 0);
	}
}

Matrix3 Quaternion::QuaternionToMatrix3() const {
	Matrix3 Result(float(1));
	float qxx(x * x);
	float qyy(y * y);
	float qzz(z * z);
	float qxz(x * z);
	float qxy(x * y);
	float qyz(y * z);
	float qwx(w * x);
	float qwy(w * y);
	float qwz(w * z);

	Result[0][0] = 1.f - 2.f * (qyy + qzz);
	Result[0][1] = 2.f * (qxy + qwz);
	Result[0][2] = 2.f * (qxz - qwy);

	Result[1][0] = 2.f * (qxy - qwz);
	Result[1][1] = 1.f - 2.f * (qxx + qzz);
	Result[1][2] = 2.f * (qyz + qwx);

	Result[2][0] = 2.f * (qxz + qwy);
	Result[2][1] = 2.f * (qyz - qwx);
	Result[2][2] = 1.f - 2.f * (qxx + qyy);
	return Result;
}

Vector3 Quaternion::QuaternionToEuler() const {
	return Vector3(Pitch(), Yaw(), Roll());
}

// -- Unary operators --
Quaternion operator+(const Quaternion& q) {
	return q;
}

Quaternion operator-(const Quaternion& q) {
	return Quaternion(-q.w, -q.x, -q.y, -q.z);
}

// -- Binary operators --
Quaternion operator+(const Quaternion& m, const Quaternion& q) {
	return Quaternion(m) += q;
}

Quaternion operator-(const Quaternion& m, const Quaternion& q) {
	return Quaternion(m) -= q;
}

Quaternion operator*(const Quaternion& m, const Quaternion& q) {
	return Quaternion(m) *= q;
}

Quaternion operator*(const float& m, const Quaternion& q) {
	return q * m;
}

Quaternion operator*(const Quaternion& q, const float& s) {
	return Quaternion(q.w * s, q.x * s, q.y * s, q.z * s);
}

Vector3 operator*(const Quaternion& q, const Vector3& v) {
	Vector3 QuatVector(q.x, q.y, q.z);
	Vector3 uv(Math::Cross(QuatVector, v));
	Vector3 uuv(Math::Cross(QuatVector, uv));

	return v + ((uv * q.w) + uuv) * static_cast<float>(2);
}

Vector3 operator*(const Vector3& v, const Quaternion& q) {
	return Math::Inverse(q) * v;
}

Quaternion operator/(const Quaternion& q, const float& s) {
	return Quaternion(q.w / s, q.x / s, q.y / s, q.z / s);
}

// -- Boolean operators --
bool operator==(const Quaternion& m, const Quaternion& s) {
	return m.x == s.x && m.y == s.y && m.z == s.z && m.w == s.w;
}

bool operator!=(const Quaternion& m, const Quaternion& s) {
	return m.x != s.x || m.y != s.y || m.z != s.z || m.w != s.w;
}
