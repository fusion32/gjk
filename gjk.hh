#ifndef GJK_EVERLOT_HH_
#define GJK_EVERLOT_HH_

#include "common.hh"
#include "math.hh"

struct GJK_Polygon{
	i32 num_points;
	Vector3 *points;
};

static INLINE
GJK_Polygon make_gjk_polygon(Vector3 *points, i32 num_points){
	GJK_Polygon result;
	result.num_points = num_points;
	result.points = points;
	return result;
}

struct GJK_Result{
	bool overlap;
	f32 distance;
	Vector3 closest1;
	Vector3 closest2;

	i32 num_points1;
	Vector3 points1[3];

	i32 num_points2;
	Vector3 points2[3];
};

GJK_Result gjk(GJK_Polygon *p1, GJK_Polygon *p2);
bool gjk_collision_test(GJK_Polygon *p1, GJK_Polygon *p2);

#endif //GJK_EVERLOT_HH_
