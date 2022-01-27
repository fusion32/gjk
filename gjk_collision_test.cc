// NOTE(IMPORTANT): This is a simplified version from "gjk.cc".
// It simply check for a collision between two shapes but won't
// give extra detail about distance or closest features.

#include "gjk.hh"

static
void gjk_collistion_test_simplex2(Vector3 *points, i32 *num_points, Vector3 *next_dir){
	// A = points[1]
	// B = points[0]
	ASSERT(*num_points == 2);
	Vector3 AO = -points[1];
	Vector3 AB = points[0] - points[1];

	// NOTE: This assert should be true or we would
	// have exited the gjk main loop.
	ASSERT(v3_dot(AB, AO) > 0);

	// points = [B, A], dir = AB x AO x AB
	*next_dir = v3_triple_cross(AB, AO, AB);
}

static
void gjk_collistion_test_simplex3(Vector3 *points, i32 *num_points, Vector3 *next_dir){
	// A = points[2]
	// B = points[1]
	// C = points[0]
	ASSERT(*num_points == 3);
	Vector3 AO = -points[2];
	Vector3 AB = points[1] - points[2];
	Vector3 AC = points[0] - points[2];
	Vector3 ABC = v3_cross(AB, AC);
	Vector3 aux;

	aux = v3_cross(ABC, AC);
	if(v3_dot(aux, AO) > 0){
		ASSERT(v3_dot(AC, AO) > 0);
		// points = [C, A], dir = AC x AO x AC
		points[1] = points[2];
		*num_points = 2;
		*next_dir = v3_triple_cross(AC, AO, AC);
		return;
	}

	aux = v3_cross(AB, ABC);
	if(v3_dot(aux, AO) > 0){
		ASSERT(v3_dot(AB, AO) > 0);
		// points = [B, A], dir = AB x AO x AB
		points[0] = points[1];
		points[1] = points[2];
		*num_points = 2;
		*next_dir = v3_triple_cross(AB, AO, AB);
		return;
	}

	if(v3_dot(ABC, AO) > 0){
		// points = [C, B, A], dir = ABC
		*next_dir = ABC;
	}else{
		// points = [B, C, A], dir = -ABC
		aux = points[1];
		points[1] = points[0];
		points[0] = aux;
		*next_dir = -ABC;
	}
}

static
bool gjk_collistion_test_simplex4(Vector3 *points, i32 *num_points, Vector3 *next_dir){
	// A = points[3]
	// B = points[2]
	// C = points[1]
	// D = points[0]
	ASSERT(*num_points == 4);
	Vector3 AO = -points[3];
	Vector3 AB = points[2] - points[3];
	Vector3 AC = points[1] - points[3];
	Vector3 AD = points[0] - points[3];
	Vector3 ACB = v3_cross(AB, AC);
	Vector3 ABD = v3_cross(AD, AB);
	Vector3 ADC = v3_cross(AC, AD);

	if(v3_dot(ACB, AO) > 0){
		ASSERT(v3_dot(AB, AO) > 0 || v3_dot(AC, AO) > 0);
		// points = [C, B, A], dir = ACB
		points[0] = points[1];
		points[1] = points[2];
		points[2] = points[3];
		*num_points = 3;
		*next_dir = ACB;
		return false;
	}

	if(v3_dot(ABD, AO) > 0){
		ASSERT(v3_dot(AB, AO) > 0 || v3_dot(AD, AO) > 0);
		// points = [B, D, A], dir = ABD
		Vector3 tmp = points[0];
		points[0] = points[2];
		points[1] = tmp;
		points[2] = points[3];
		*num_points = 3;
		*next_dir = ABD;
		return false;
	}

	if(v3_dot(ADC, AO) > 0){
		ASSERT(v3_dot(AC, AO) > 0 || v3_dot(AD, AO) > 0);
		// points = [D, C, A]
		// points[0] = points[0];
		// points[1] = points[1];
		points[2] = points[3];
		*num_points = 3;
		*next_dir = ADC;
		return false;
	}

	return true;
}

static
Vector3 gjk_polygon_support(GJK_Polygon *p1, GJK_Polygon *p2, Vector3 dir){
	ASSERT(p1->num_points > 0 && p2->num_points > 0);

	i32 index1 = 0;
	f32 max1 = v3_dot(p1->points[0], dir);

	i32 index2 = 0;
	f32 max2 = v3_dot(p2->points[0], -dir);

	i32 imax = i32_max(p1->num_points, p2->num_points);
	for(i32 i = 1; i < imax; i += 1){
		if(i < p1->num_points){
			f32 dot = v3_dot(p1->points[i], dir);
			if(dot > max1){
				max1 = dot;
				index1 = i;
			}
		}

		if(i < p2->num_points){
			f32 dot = v3_dot(p2->points[i], -dir);
			if(dot > max2){
				max2 = dot;
				index2 = i;
			}
		}
	}

	Vector3 result = p1->points[index1] - p2->points[index2];
	return result;
}

bool gjk_collision_test(GJK_Polygon *p1, GJK_Polygon *p2){
	Vector3 initial_point = gjk_polygon_support(
			p1, p2, make_v3(0.0f, 0.0f, -1.0f));
	i32 num_points = 1;
	Vector3 points[4] = { initial_point };
	Vector3 dir = -initial_point;
	while(1){
		Vector3 new_point = gjk_polygon_support(p1, p2, dir);
		if(v3_dot(dir, new_point) < 0)
			return false;

		points[num_points] = new_point;
		num_points += 1;

		ASSERT(num_points >= 2 && num_points <= 4);
		switch(num_points){
			case 2:
				gjk_collistion_test_simplex2(points, &num_points, &dir);
				break;
			case 3:
				gjk_collistion_test_simplex3(points, &num_points, &dir);
				break;
			case 4:
				if(gjk_collistion_test_simplex4(points, &num_points, &dir))
					return true;
				break;
		}
	}
}
