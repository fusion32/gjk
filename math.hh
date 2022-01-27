#ifndef GJK_MATH_HH_
#define GJK_MATH_HH_ 1

#include "common.hh"
#include <math.h>

// ----------------------------------------------------------------
// i32 operations
// ----------------------------------------------------------------

static INLINE
i32 i32_min(i32 a, i32 b){
	return a < b ? a : b;
}

static INLINE
i32 i32_max(i32 a, i32 b){
	return a > b ? a : b;
}

// ----------------------------------------------------------------
// f32 operations
// ----------------------------------------------------------------

// NOTE: These are arbitrary.
#define F32_EPSILON (1.0e-3f)
#define F32_EPSILON2 (F32_EPSILON * F32_EPSILON)

static INLINE
f32 f32_abs(f32 value){
	return value > 0.0f ? value : -value;
}

static INLINE
bool f32_cmp_zero(f32 value){
	return f32_abs(value) < F32_EPSILON;
}

// ----------------------------------------------------------------
// Vector3
// ----------------------------------------------------------------
struct Vector3{
	f32 x, y, z;
};

static Vector3 v3_zero = {};

static INLINE
Vector3 make_v3(f32 x, f32 y, f32 z){
	Vector3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

static INLINE
Vector3 operator-(const Vector3 &v){
	Vector3 result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;
	return result;
}

static INLINE
Vector3 operator+(const Vector3 &a, const Vector3 &b){
	Vector3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

static INLINE
Vector3 operator-(const Vector3 &a, const Vector3 &b){
	Vector3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

static INLINE
Vector3 operator*(const Vector3 &a, const Vector3 &b){
	Vector3 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}

static INLINE
Vector3 operator*(f32 a, const Vector3 &b){
	Vector3 result;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
	return result;
}

static INLINE
Vector3 operator*(const Vector3 &a, f32 b){
	Vector3 result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

static INLINE
void operator+=(Vector3 &a, const Vector3 &b){
	a = a + b;
}

static INLINE
void operator-=(Vector3 &a, const Vector3 &b){
	a = a - b;
}

static INLINE
void operator*=(Vector3 &a, const Vector3 &b){
	a = a * b;
}

static INLINE
void operator*=(Vector3 &a, f32 b){
	a = a * b;
}

static INLINE
bool operator>(const Vector3 &a, const Vector3 &b){
	return a.x > b.x && a.y > b.y && a.z > b.z;
}

static INLINE
bool operator<(const Vector3 &a, const Vector3 &b){
	return a.x < b.x && a.y < b.y && a.z < b.z;
}

static INLINE
bool operator==(const Vector3 &a, const Vector3 &b){
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

static INLINE
f32 v3_norm2(const Vector3 &v){
	f32 norm2 = v.x * v.x + v.y * v.y + v.z * v.z;
	return norm2;
}

static INLINE
f32 v3_norm(const Vector3 &v){
	f32 norm2 = v3_norm2(v);
	return sqrtf(norm2);
}

static INLINE
bool v3_cmp_zero(const Vector3 &v){
	return v3_norm2(v) < F32_EPSILON2;
}

static INLINE
Vector3 v3_normalize(const Vector3 &v){
	f32 norm = v3_norm(v);
	ASSERT(norm > 0.0f);
	Vector3 result = (1.0f / norm) * v;
	return result;
}

static INLINE
f32 v3_dot(const Vector3 &a, const Vector3 &b){
	f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
	return result;
}

static INLINE
Vector3 v3_cross(const Vector3 &a, const Vector3 &b){
	Vector3 result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

static INLINE
Vector3 v3_triple_cross(const Vector3 &a, const Vector3 &b, const Vector3 &c){
	// NOTE: Using Lagrange's formula:
	//		(a cross b) cross c = -c cross (a cross b)
	//					= -(c dot b) * a + (c dot a) * b
	//					= (c dot a) * b - (c dot b) * a
	//
	// (I'm not sure if this ends up being an optimization or not)
	//

	Vector3 result =  v3_dot(c, a) * b - v3_dot(c, b) * a;
	return result;
}

// ----------------------------------------------------------------
// Quaternion
// ----------------------------------------------------------------
struct Quaternion{
	f32 w, x, y, z;
};

static Quaternion make_quat(f32 w, f32 x, f32 y, f32 z){
	Quaternion result;
	result.w = w;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

static Quaternion operator*(const Quaternion &a, const Quaternion &b){
	Quaternion result;
	result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	result.y = a.w * b.y + a.y * b.w - a.x * b.z + a.z * b.x;
	result.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
	return result;
}

static Quaternion quat_normalize(const Quaternion &q){
	f32 norm2 = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
	f32 norm = 1.0f / sqrtf(norm2);
	Quaternion result;
	result.w = q.w * norm;
	result.x = q.x * norm;
	result.y = q.y * norm;
	result.z = q.z * norm;
	return result;
}

// ----------------------------------------------------------------
// Matrix4
// ----------------------------------------------------------------
union Matrix4{
	f32 m[16];

#if MAT4_COLUMN_MAJOR
	struct{
		f32 m11, m21, m31, m41;
		f32 m12, m22, m32, m42;
		f32 m13, m23, m33, m43;
		f32 m14, m24, m34, m44;
	};
#else
	struct{
		f32 m11, m12, m13, m14;
		f32 m21, m22, m23, m24;
		f32 m31, m32, m33, m34;
		f32 m41, m42, m43, m44;
	};
#endif
};

static Matrix4 mat4_identity = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

static Matrix4 operator*(const Matrix4 &a, const Matrix4 &b){
	Matrix4 result;

	result.m11 = a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31 + a.m14 * b.m41;
	result.m12 = a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32 + a.m14 * b.m42;
	result.m13 = a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33 + a.m14 * b.m43;
	result.m14 = a.m11 * b.m14 + a.m12 * b.m24 + a.m13 * b.m34 + a.m14 * b.m44;

	result.m21 = a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31 + a.m24 * b.m41;
	result.m22 = a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32 + a.m24 * b.m42;
	result.m23 = a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33 + a.m24 * b.m43;
	result.m24 = a.m21 * b.m14 + a.m22 * b.m24 + a.m23 * b.m34 + a.m24 * b.m44;

	result.m31 = a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31 + a.m34 * b.m41;
	result.m32 = a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32 + a.m34 * b.m42;
	result.m33 = a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33 + a.m34 * b.m43;
	result.m34 = a.m31 * b.m14 + a.m32 * b.m24 + a.m33 * b.m34 + a.m34 * b.m44;

	result.m41 = a.m41 * b.m11 + a.m42 * b.m21 + a.m43 * b.m31 + a.m44 * b.m41;
	result.m42 = a.m41 * b.m12 + a.m42 * b.m22 + a.m43 * b.m32 + a.m44 * b.m42;
	result.m43 = a.m41 * b.m13 + a.m42 * b.m23 + a.m43 * b.m33 + a.m44 * b.m43;
	result.m44 = a.m41 * b.m14 + a.m42 * b.m24 + a.m43 * b.m34 + a.m44 * b.m44;

	return result;
}

static Matrix4 mat4_scale(f32 s){
	Matrix4 result = {};
	result.m11 = s;
	result.m22 = s;
	result.m33 = s;
	result.m44 = 1.0f;
	return result;
}

static Matrix4 mat4_scale(const Vector3 &v){
	Matrix4 result = {};
	result.m11 = v.x;
	result.m22 = v.y;
	result.m33 = v.z;
	result.m44 = 1.0f;
	return result;
}

static Matrix4 mat4_translation(const Vector3 &v){
	Matrix4 result = mat4_identity;
	result.m14 = v.x;
	result.m24 = v.y;
	result.m34 = v.z;
	return result;
}

static Matrix4 mat4_perspective(f32 aspect_ratio, f32 yfov, f32 znear, f32 zfar){
	f32 aux = tanf(0.5f * yfov);
	Matrix4 result = {};
	result.m11 = 1.0f / (aspect_ratio * aux);
	result.m22 = 1.0f / aux;
	result.m33 = (zfar + znear)/(znear - zfar);
	result.m34 = (2.0f * zfar * znear)/(znear - zfar);
	result.m43 = -1.0f;
	return result;
}

static Matrix4 mat4_orthographic(f32 xmag, f32 ymag, f32 znear, f32 zfar){
	f32 zlen = zfar - znear;
	Matrix4 result = {};
	result.m11 = 1.0f / xmag;
	result.m22 = 1.0f / ymag;
	result.m33 = -2.0f / zlen;
	result.m34 = -(zfar + znear) / zlen;
	result.m44 = 1.0f;
	return result;
}

// ----------------------------------------------------------------
// Utility
// ----------------------------------------------------------------
#define CONST_PI 3.14159265358979323846
#define CONST_2PI (2.0 * CONST_PI)

static Quaternion quat_angle_axis(f32 angle, const Vector3 &axis){
	f32 half_angle = 0.5f * angle;
	f32 half_sin = sinf(half_angle);
	f32 half_cos = cosf(half_angle);

	Vector3 naxis = v3_normalize(axis);
	Quaternion result;
	result.w = half_cos;
	result.x = half_sin * naxis.x;
	result.y = half_sin * naxis.y;
	result.z = half_sin * naxis.z;
	return result;
}

static Quaternion quat_rotation_between(const Vector3 &a, const Vector3 &b){
	Vector3 cross = v3_cross(a, b);
	Quaternion result;
	result.w = sqrtf(v3_norm2(a) * v3_norm2(b)) + v3_dot(a, b);
	result.x = cross.x;
	result.y = cross.y;
	result.z = cross.z;
	return quat_normalize(result);
}

static Vector3 v3_rotate(const Vector3 &v, const Quaternion &q){
	Quaternion vq = make_quat(0.0f, v.x, v.y, v.z);
	Quaternion qconj = make_quat(q.w, -q.x, -q.y, -q.z);
	Quaternion tmp = q * vq * qconj;
	Vector3 result;
	result.x = tmp.x;
	result.y = tmp.y;
	result.z = tmp.z;
	return result;
}

static Matrix4 mat4_from_quat(const Quaternion &q){
	Matrix4 result;

	result.m11 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	result.m12 = 2.0f * (q.x * q.y - q.z * q.w);
	result.m13 = 2.0f * (q.x * q.z + q.y * q.w);
	result.m14 = 0.0f;

	result.m21 = 2.0f * (q.x * q.y + q.z * q.w);
	result.m22 = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
	result.m23 = 2.0f * (q.y * q.z - q.x * q.w);
	result.m24 = 0.0f;

	result.m31 = 2.0f * (q.x * q.z - q.y * q.w);
	result.m32 = 2.0f * (q.y * q.z + q.x * q.w);
	result.m33 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	result.m34 = 0.0f;

	result.m41 = 0.0f;
	result.m42 = 0.0f;
	result.m43 = 0.0f;
	result.m44 = 1.0f;

	return result;
}

static Vector3 operator*(const Matrix4 &m, const Vector3 &v){
	Vector3 result;
	result.x = m.m11 * v.x + m.m12 * v.y + m.m13 * v.z + m.m14;
	result.y = m.m21 * v.x + m.m22 * v.y + m.m23 * v.z + m.m24;
	result.z = m.m31 * v.x + m.m32 * v.y + m.m33 * v.z + m.m34;
	return result;
}

#endif //GJK_MATH_HH_
