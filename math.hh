// TODO: more testing and use SIMD in the future

// TODO: since we're declaring pretty much everything
// INLINE there is probably no difference between
// passing by value and by const reference (CHECK THIS)

// TODO: eventually reorder functions and operators so
// that those with similar functionality are closer
// (since i've been adding functions as we need them we
// currently have a weird order of functions)

#ifndef EVERLOT_MATH_HH_
#define EVERLOT_MATH_HH_ 1

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

// TODO: Eventually tune this value.
// NOTE: We should either use these values for f32 epsilon or use
// the functions f32_cmp_zero or v3_cmp_zero that use these values
// when comparing f32s with zero.
#define F32_EPSILON (1.0e-3f)
#define F32_EPSILON2 (F32_EPSILON * F32_EPSILON)

#if ARCH_X64
#include <intrin.h>

//
// These functions should compile to the assembly below (before
// inlining) but it seems to only happen when -O2 is used.
//
// f32_truncate:
//	cvttss2si eax, xmm0
//	ret
//
// f32_ceil:
//	roundss xmm0, xmm0, 2 ; or vroundss xmm0, xmm0, xmm0, 2
//	cvttss2si eax, xmm0
//	ret
//
// f32_min:
//	minss xmm0, xmm1
//	ret
//
// f32_max:
//	maxss xmm0, xmm1
//	ret
//

static INLINE
i32 f32_truncate(f32 value){
	__m128 xmm0 = _mm_load_ss(&value);
	return (i32)_mm_cvtt_ss2si(xmm0);
}

static INLINE
i32 f32_ceil(f32 value){
	f32 result;
	__m128 xmm0 = _mm_load_ss(&value);
	xmm0 = _mm_ceil_ss(xmm0, xmm0);
	_mm_store_ss(&result, xmm0);
	return (i32)result;
}

static INLINE
f32 f32_min(f32 a, f32 b){
	f32 result;
	__m128 xmm0 = _mm_load_ss(&a);
	__m128 xmm1 = _mm_load_ss(&b);
	xmm0 = _mm_min_ss(xmm0, xmm1);
	_mm_store_ss(&result, xmm0);
	return result;
}

static INLINE
f32 f32_max(f32 a, f32 b){
	f32 result;
	__m128 xmm0 = _mm_load_ss(&a);
	__m128 xmm1 = _mm_load_ss(&b);
	xmm0 = _mm_max_ss(xmm0, xmm1);
	_mm_store_ss(&result, xmm0);
	return result;
}

static INLINE
f32 f32_abs(f32 value){
	return f32_max(value, -value);
}

#if 0
static INLINE
f32 f32_absmin(f32 a, f32 b){
	// NOTE: Return either a or b depending on which has
	// the least absolute value (magnitude).
	__m128 zero = _mm_set1_ps(0.0f);
	__m128 values = _mm_set_ps(0.0f, 0.0f, a, b);
	__m128 neg_values = _mm_sub_ps(zero, values);
	__m128 abs_values = _mm_max_ps(values, neg_values);
	__m128 reversed_abs_values = _mm_shuffle_ps(abs_values, abs_values, 0xE1); // 0b11100001
	__m128 result_mask = _mm_cmplt_ps(abs_values, reversed_abs_values);

	__m128i tmp1 = _mm_and_si128(
		_mm_castps_si128(values),
		_mm_castps_si128(result_mask));
	__m128i tmp2 = _mm_srli_si128(tmp1, 4);
	__m128i result128 = _mm_or_si128(tmp1, tmp2);
	f32 result = _mm_cvtss_f32(_mm_castsi128_ps(result128));
	//or... f32 result; _mm_store_ss(&result, _mm_castsi128_ps(result128));
	return result;
}
#endif

static INLINE
f32 f32_sign(f32 value){
	__m128 one = _mm_set_ss(1.0f);		// 0x3F800000
	__m128 n_zero = _mm_set_ss(-0.0f);	// 0x80000000
	__m128 value128 = _mm_load_ss(&value);
	__m128 sign = _mm_and_ps(value128, n_zero);
	__m128 result128 = _mm_or_ps(one, sign);
	f32 result = _mm_cvtss_f32(result128);
	return result;
}

#else //ARCH_X64

static INLINE
i32 f32_truncate(f32 value){
	return (i32)truncf(value);
}

static INLINE
i32 f32_ceil(f32 value){
	return (i32)ceilf(value);
}

static INLINE
f32 f32_min(f32 a, f32 b){
	return a < b ? a : b;
}

static INLINE
f32 f32_max(f32 a, f32 b){
	return a > b ? a : b;
}

static INLINE
f32 f32_abs(f32 value){
	return value > 0.0f ? value : -value;
}

#if 0
static INLINE
f32 f32_absmin(f32 a, f32 b){
	// NOTE: Return either a or b depending on which has
	// the least absolute value (magnitude).
	return f32_abs(a) < f32_abs(b) ? a : b;
}
#endif

static INLINE
f32 f32_sign(f32 value){
	return value > 0.0f ? +1.0f : -1.0f;
}

#endif //ARCH_X64

static INLINE
f32 f32_lerp(f32 a, f32 b, f32 t){
	f32 result = a + t * (b - a);
	return result;
}

static INLINE
bool f32_cmp_zero(f32 value){
	return f32_abs(value) < F32_EPSILON;
}

// ----------------------------------------------------------------
// Vector2
// ----------------------------------------------------------------
union Vector2{
	f32 v[2];
	struct{
		f32 x, y;
	};
};

static Vector2 v2_zero = {};

static INLINE
Vector2 make_v2(f32 x, f32 y){
	Vector2 result;
	result.x = x;
	result.y = y;
	return result;
}

static INLINE
Vector2 operator-(const Vector2 &v){
	Vector2 result;
	result.x = -v.x;
	result.y = -v.y;
	return result;
}

static INLINE
Vector2 operator+(const Vector2 &a, const Vector2 &b){
	Vector2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

static INLINE
Vector2 operator-(const Vector2 &a, const Vector2 &b){
	Vector2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

static INLINE
Vector2 operator*(const Vector2 &a, const Vector2 &b){
	Vector2 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return result;
}

static INLINE
Vector2 operator*(f32 a, const Vector2 &b){
	Vector2 result;
	result.x = a * b.x;
	result.y = a * b.y;
	return result;
}

static INLINE
Vector2 operator*(const Vector2 &a, f32 b){
	Vector2 result = b * a;
	return result;
}

static INLINE
void operator+=(Vector2 &a, const Vector2 &b){
	a = a + b;
}

static INLINE
void operator-=(Vector2 &a, const Vector2 &b){
	a = a - b;
}

static INLINE
void operator*=(Vector2 &a, const Vector2 &b){
	a = a * b;
}

static INLINE
bool operator>(const Vector2 &a, const Vector2 &b){
	return a.x > b.x && a.y > b.y;
}

static INLINE
bool operator<(const Vector2 &a, const Vector2 &b){
	return a.x < b.x && a.y < b.y;
}

static INLINE
f32 v2_norm2(const Vector2 &v){
	f32 norm2 = v.x * v.x + v.y * v.y;
	return norm2;
}

static INLINE
f32 v2_norm(const Vector2 &v){
	f32 norm2 = v2_norm2(v);
	return sqrtf(norm2);
}

static INLINE
Vector2 v2_normalize(const Vector2 &v){
	f32 norm = v2_norm(v);
	DEBUG_ASSERT(norm > 0.0f);
	Vector2 result = (1.0f / norm) * v;
	return result;
}

static INLINE
f32 v2_dist2(const Vector2 &a, const Vector2 &b){
	return v2_norm2(a - b);
}

static INLINE
Vector2 v2_min(const Vector2 &a, const Vector2 &b){
	Vector2 result;
	result.x = f32_min(a.x, b.x);
	result.y = f32_min(a.y, b.y);
	return result;
}

static INLINE
Vector2 v2_max(const Vector2 &a, const Vector2 &b){
	Vector2 result;
	result.x = f32_max(a.x, b.x);
	result.y = f32_max(a.y, b.y);
	return result;
}

static INLINE
Vector2 v2_orthogonal(const Vector2 &v){
	Vector2 result;
	result.x = -v.y;
	result.y = v.x;
	return result;
}

// ----------------------------------------------------------------
// Vector3
// ----------------------------------------------------------------
union Vector3{
	f32 v[3];
	struct{
		f32 x, y, z;
	};
	Vector2 xy;
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
Vector3 make_v3(const Vector2 &xy, f32 z){
	Vector3 result;
	result.x = xy.x;
	result.y = xy.y;
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
	DEBUG_ASSERT(norm > 0.0f);
	Vector3 result = (1.0f / norm) * v;
	return result;
}

static INLINE
f32 v3_dist2(const Vector2 &a, const Vector2 &b){
	return v2_norm2(a - b);
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

static INLINE
Vector3 v3_lerp(const Vector3 &a, const Vector3 &b, f32 t){
	Vector3 result = a + t * (b - a);
	return result;
}

static INLINE
Vector3 v3_min(const Vector3 &a, const Vector3 &b){
	Vector3 result;
	result.x = f32_min(a.x, b.x);
	result.y = f32_min(a.y, b.y);
	result.z = f32_min(a.z, b.z);
	return result;
}

#if 0
static INLINE
Vector3 v3_absmin(const Vector3 &a, const Vector3 &b){
	Vector3 result;
	result.x = f32_absmin(a.x, b.x);
	result.y = f32_absmin(a.y, b.y);
	result.z = f32_absmin(a.z, b.z);
	return result;
}
#endif

static INLINE
Vector3 v3_max(const Vector3 &a, const Vector3 &b){
	Vector3 result;
	result.x = f32_max(a.x, b.x);
	result.y = f32_max(a.y, b.y);
	result.z = f32_max(a.z, b.z);
	return result;
}

static INLINE
Vector3 v3_clamp(const Vector3 &v, const Vector3 &min, const Vector3 &max){
	Vector3 result = v3_max(min, v3_min(max, v));
	return result;
}

static INLINE
Vector3 v3_abs(const Vector3 &v){
	Vector3 result;
	result.x = f32_abs(v.x);
	result.y = f32_abs(v.y);
	result.z = f32_abs(v.z);
	return result;
}

static INLINE
Vector3 v3_sign(const Vector3 &v){
	Vector3 result;
	result.x = f32_sign(v.x);
	result.y = f32_sign(v.y);
	result.z = f32_sign(v.z);
	return result;
}

static INLINE
Vector3 v3_orthogonal(const Vector3 &v){
	Vector3 result;
	f32 aux = v.x * v.x + v.y * v.y;
	DEBUG_ASSERT(aux >= 0.0f); // PARANOID: this should be always true
	DEBUG_ASSERT((aux + v.z * v.z) >= 0.9f);
	if(aux > 0.1f){
		result = v3_cross(v, make_v3(-v.y, v.x, 0.0f));
	}else{
		result = v3_cross(v, make_v3(1.0f, 1.0f, 0.0f));
	}
	result = v3_normalize(result);
	return result;
}

static INLINE
Vector3 v3_reflect(const Vector3 &v, const Vector3 &normal){
	DEBUG_ASSERT(v3_norm2(normal) > 0.98f && v3_norm2(normal) < 1.02f);
	return 2.0f * v3_dot(-v, normal) * normal + v;
}

// ----------------------------------------------------------------
// Rect2
// ----------------------------------------------------------------
struct Rect2{
	Vector2 min;
	Vector2 max;
};

static INLINE
Rect2 make_rect2(const Vector2 &min, const Vector2 &max){
	Rect2 result;
	result.min = min;
	result.max = max;
	return result;
}

static INLINE
Rect2 rect2_combine(const Rect2 &a, const Rect2 &b){
	Rect2 result;
	result.min = v2_min(a.min, b.min);
	result.max = v2_max(a.max, b.max);
	return result;
}

static INLINE
Vector2 rect2_dim(const Rect2 &r){
	return r.max - r.min;
}

static INLINE
f32 rect2_perimeter(const Rect2 &r){
	Vector2 dim = rect2_dim(r);
	return 2.0f * (dim.x + dim.y);
}

static INLINE
f32 rect2_area(const Rect2 &r){
	Vector2 dim = rect2_dim(r);
	return dim.x * dim.y;
}

static INLINE
bool rect2_check_overlap(const Rect2 &a, const Rect2 &b){
	return b.min < a.max && b.max > a.min;
}

static INLINE
bool rect2_contains_point(const Rect2 &r, const Vector2 &p){
	return p > r.min && p < r.max;
}

static INLINE
bool rect2_contains(const Rect2 &r, const Rect2 &other){
	return rect2_contains_point(r, other.min) &&
		rect2_contains_point(r, other.max);
}

// ----------------------------------------------------------------
// Rect3
// ----------------------------------------------------------------
struct Rect3{
	Vector3 min;
	Vector3 max;
};

static INLINE
Rect3 make_rect3(const Vector3 &min, const Vector3 &max){
	Rect3 result;
	result.min = min;
	result.max = max;
	return result;
}

static INLINE
Rect3 rect3_combine(const Rect3 &a, const Rect3 &b){
	Rect3 result;
	result.min = v3_min(a.min, b.min);
	result.max = v3_max(a.max, b.max);
	return result;
}

static INLINE
Vector3 rect3_dim(const Rect3 &r){
	return r.max - r.min;
}

static INLINE
Vector3 rect3_center(const Rect3 &r){
	return 0.5f * (r.max + r.min);
}

static INLINE
f32 rect3_area(const Rect3 &r){
	Vector3 dim = rect3_dim(r);
	return 2.0f * (dim.x * dim.y + dim.x * dim.z + dim.y * dim.z);
}

static INLINE
f32 rect3_volume(const Rect3 &r){
	Vector3 dim = rect3_dim(r);
	return dim.x * dim.y * dim.z;
}

static INLINE
bool rect3_check_overlap(const Rect3 &a, const Rect3 &b){
	return b.min < a.max && b.max > a.min;
}

static INLINE
bool rect3_contains_point(const Rect3 &r, const Vector3 &p){
	return p > r.min && p < r.max;
}

static INLINE
bool rect3_contains(const Rect3 &r, const Rect3 &other){
	return rect3_contains_point(r, other.min) &&
		rect3_contains_point(r, other.max);
}

static INLINE
Rect3 operator+(const Rect3 &r, const Vector3 &v){
	Rect3 result;
	result.min = r.min + v;
	result.max = r.max + v;
	return result;
}

static INLINE
Rect3 operator+(const Vector3 &v, const Rect3 &r){
	return r + v;
}

static INLINE
bool operator==(const Rect3 &a, const Rect3 &b){
	return a.min == b.min && a.max == b.max;
}

static INLINE
Rect3 rect3_intersection(const Rect3 &a, const Rect3 &b){
	DEBUG_ASSERT(rect3_check_overlap(a, b));
	Rect3 result;
	result.min = v3_max(a.min, b.min);
	result.max = v3_min(a.max, b.max);
	return result;
}

static INLINE
Vector3 rect3_overlap_depth(const Rect3 &a, const Rect3 &b){
#if 0
	// NOTE: This will give the incorrect result whenever
	// one of the bounding boxes contains the other entirely.
	Rect3 intersection = rect3_intersection(a, b);
	Vector3 result = rect3_dim(intersection);
	return result;
#else
	DEBUG_ASSERT(rect3_check_overlap(a, b));
	Vector3 result1 = a.max - b.min;
	Vector3 result2 = b.max - a.min;
	Vector3 result = v3_min(result1, result2);
	return result;
#endif
}

static INLINE
bool rect3_check_sphere_overlap(const Rect3 &r, const Vector3 &center, f32 radius){
	Vector3 closest = v3_clamp(center, r.min, r.max);
	f32 dist2 = v3_norm2(closest);
	f32 radius2 = radius * radius;
	return dist2 < radius2;
}

static INLINE
Rect3 rect3_uniform_scale(const Rect3 &r, f32 scale){
	// NOTE: It would be a bug if we could turn the rect inside out.
	DEBUG_ASSERT(scale > 0.0f);
	Vector3 d = 0.5f * (scale - 1.0f) * rect3_dim(r);
	Rect3 result;
	result.min = r.min - d;
	result.max = r.max + d;
	return result;
}

static INLINE
Rect3 rect3_scale(const Rect3 &r, const Vector3 &scale){
	// NOTE: It would be a bug if we could turn the rect inside out.
	DEBUG_ASSERT(scale > v3_zero);
	Vector3 one = make_v3(1.0f, 1.0f, 1.0f);
	Vector3 d = 0.5f * (scale - one) * rect3_dim(r);
	Rect3 result;
	result.min = r.min - d;
	result.max = r.max + d;
	return result;
}

#if 0
// TODO: I started using rect3_move instead of the operator+ for
// displaced AABBs but maybe the operator+ is clear enough so I
// reverted back to using the operator+. See if we get into trouble
// for this.

static INLINE
Rect3 rect3_move(const Rect3 &r, const Vector3 &displacement){
	Rect3 result;
	result.min = r.min + displacement;
	result.max = r.max + displacement;
	return result;
}

static INLINE
Rect3 rect3_scale_and_move(const Rect3 &r,
		const Vector3 &scale,
		const Vector3 &displacement){
	// NOTE: The order of operations is commutative.
	Rect3 result = rect3_scale(r, scale);
	result = rect3_move(result, displacement);
	return result;
}
#endif

// ----------------------------------------------------------------
// Ray3
// ----------------------------------------------------------------

struct Ray3{
	Vector3 origin;
	Vector3 inv_dir;
	f32 max_fraction;
};

static INLINE
Ray3 make_ray3(const Vector3 &origin, const Vector3 &dest){
	// NOTE: See note in ray3_intersects_rect3 for a
	// divide by zero explanation.

	Vector3 dir = dest - origin;
	Ray3 result;
	result.origin = origin;
	result.inv_dir = make_v3(
		1 / dir.x,
		1 / dir.y,
		1 / dir.z);
	result.max_fraction = 1.0f;
	return result;
}

static INLINE
bool ray3_intersects_rect3(const Ray3 &ray,
		const Rect3 &rect, f32 *hit_fraction){

	// NOTE:
	// Since ray_direction may contain zero components,
	// we may end up with division by zero when computing
	// inv_ray_dir. In C++ this may be undefined behaviour
	// but it is well defined on IEEE754 floating point
	// hardware which is the case on most hardware.
	//
	// I'm not gonna try to explain in detail here but
	// there's a GDC18 talk covering this and using
	// 4-lanes SIMD:
	//	https://www.youtube.com/watch?v=6BIfqfC1i7U
	//
	// Also don't try to swap the order when computing
	// tmin_x, tmax_x, tmin_y, tmax_y, tmin_z, tmax_z,
	// tmin, and tmax, since it uses assumptions better
	// explained in the video above.
	//

	// TODO: The names here are a bit weird. First t0 and
	// t1 are intersections of rect.min and rect.max with
	// the ray respectively but naming them tmin and tmax
	// could be confusing since tmin may be greater than
	// tmax if the ray does not intersect the rect. Maybe
	// rename them t_min_intersection and t_max_intersection?
	// Maybe it would make it even more confusing.

	f32 t0_x = (rect.min.x - ray.origin.x) * ray.inv_dir.x;
	f32 t1_x = (rect.max.x - ray.origin.x) * ray.inv_dir.x;
	f32 t0_y = (rect.min.y - ray.origin.y) * ray.inv_dir.y;
	f32 t1_y = (rect.max.y - ray.origin.y) * ray.inv_dir.y;
	f32 t0_z = (rect.min.z - ray.origin.z) * ray.inv_dir.z;
	f32 t1_z = (rect.max.z - ray.origin.z) * ray.inv_dir.z;
	f32 tmin_x = f32_min(t0_x, t1_x);
	f32 tmax_x = f32_max(t1_x, t0_x);
	f32 tmin_y = f32_min(t0_y, t1_y);
	f32 tmax_y = f32_max(t1_y, t0_y);
	f32 tmin_z = f32_min(t0_z, t1_z);
	f32 tmax_z = f32_max(t1_z, t0_z);
	f32 tmin = f32_max(tmin_z, f32_max(tmin_y, f32_max(tmin_x, 0.0f)));
	f32 tmax = f32_min(tmax_z, f32_min(tmax_y, f32_min(tmax_x, ray.max_fraction)));

	*hit_fraction = tmin;
	return tmin < tmax;
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
#define EV_PI 3.14159265358979323846
#define EV_2PI (2.0 * EV_PI)
#define EV_DEG2RAD ((f32)(EV_PI / 180.0))
#define EV_RAD2DEG ((f32)(EV_PI * 180.0))

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

#endif //EVERLOT_MATH_HH_
