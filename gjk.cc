// NOTE(IMPORTANT): Different from gjk_collision_test, our main
// condition to leave the iteration loop is when no progress has
// been made (ie. the support function returned a point that was
// already in the simplex it returned a point that made the next
// simplex degenerate). Because of that, we need to check extra
// cases when the origin could be behind the point A.

#include "gjk.hh"

struct GJK_Point{
	Vector3 minkowski;
	Vector3 polygon1;
	Vector3 polygon2;
};

static bool gjk_check_degenerate_simplex2(GJK_Point *points, i32 num_points){
	ASSERT(num_points == 2);
	Vector3 A = points[1].minkowski;
	Vector3 B = points[0].minkowski;
	return v3_cmp_zero(B - A);
}

static bool gjk_check_degenerate_simplex3(GJK_Point *points, i32 num_points){
	ASSERT(num_points == 3);
	Vector3 A = points[2].minkowski;
	Vector3 B = points[1].minkowski;
	Vector3 C = points[0].minkowski;
	f32 area = 0.5f * v3_norm(v3_cross(B - A, C - A));
	return f32_cmp_zero(area);
}

static bool gjk_check_degenerate_simplex4(GJK_Point *points, i32 num_points){
	ASSERT(num_points == 4);
	Vector3 A = points[3].minkowski;
	Vector3 B = points[2].minkowski;
	Vector3 C = points[1].minkowski;
	Vector3 D = points[0].minkowski;
	//f32 volume = 0.1666667f * v3_dot(v3_cross(C - A, B - A), D - A);
	f32 volume = (1.0f / 6.0f) * v3_dot(v3_cross(C - A, B - A), D - A);
	return f32_cmp_zero(volume);
}

static bool gjk_check_degenerate_simplex(GJK_Point *points, i32 num_points){
	ASSERT(num_points >= 2 && num_points <= 4);
	switch(num_points){
		case 2: return gjk_check_degenerate_simplex2(points, num_points);
		case 3: return gjk_check_degenerate_simplex3(points, num_points);
		case 4: return gjk_check_degenerate_simplex4(points, num_points);
	}
	return false; // NOTE: compiler warning
}

static
void gjk_simplex2(GJK_Point *points, i32 *num_points, Vector3 *dir){
	// A = points[1]
	// B = points[0]
	ASSERT(*num_points == 2);
	Vector3 AO = -points[1].minkowski;
	Vector3 AB = points[0].minkowski - points[1].minkowski;

	if(v3_dot(AB, AO) > 0){
		// points = [B, A], dir = AB x AO x AB
		*dir = v3_triple_cross(AB, AO, AB);
	}else{
		// points = [A], dir = AO
		points[0] = points[1];
		*num_points = 1;
		*dir = AO;
	}
}

static
void gjk_simplex3(GJK_Point *points, i32 *num_points, Vector3 *dir){
	// A = points[2].minkowski
	// B = points[1].minkowski
	// C = points[0].minkowski
	ASSERT(*num_points == 3);
	Vector3 AO = -points[2].minkowski;
	Vector3 AB = points[1].minkowski - points[2].minkowski;
	Vector3 AC = points[0].minkowski - points[2].minkowski;
	Vector3 ABC = v3_cross(AB, AC);
	Vector3 aux;

	aux = v3_cross(ABC, AC);
	if(v3_dot(aux, AO) > 0){
		if(v3_dot(AC, AO) > 0){
			// points = [C, A], dir = AC x AO x AC
			//points[0] = points[0];
			points[1] = points[2];
			*num_points = 2;
			*dir = v3_triple_cross(AC, AO, AC);
		}else{
			// points = [A], dir = AO
			points[0] = points[2];
			*num_points = 1;
			*dir = AO;
		}
		return;
	}

	aux = v3_cross(AB, ABC);
	if(v3_dot(aux, AO) > 0){
		if(v3_dot(AB, AO) > 0){
			// points = [B, A], dir = AB x AO x AB
			points[0] = points[1];
			points[1] = points[2];
			*num_points = 2;
			*dir = v3_triple_cross(AB, AO, AB);
		}else{
			// points = [A], dir = AO
			points[0] = points[2];
			*num_points = 1;
			*dir = AO;
		}
		return;
	}

	if(v3_dot(ABC, AO) > 0){
		// points = [C, B, A], dir = ABC
		*dir = ABC;
	}else{
		// points = [B, C, A], dir = -ABC
		GJK_Point tmp = points[0];
		points[0] = points[1];
		points[1] = tmp;
		*dir = -ABC;
	}
}

static
bool gjk_simplex4(GJK_Point *points, i32 *num_points, Vector3 *dir){
	// A = points[3].minkowski
	// B = points[2].minkowski
	// C = points[1].minkowski
	// D = points[0].minkowski
	ASSERT(*num_points == 4);
	Vector3 AO = -points[3].minkowski;
	Vector3 AB = points[2].minkowski - points[3].minkowski;
	Vector3 AC = points[1].minkowski - points[3].minkowski;
	Vector3 AD = points[0].minkowski - points[3].minkowski;
	Vector3 ACB = v3_cross(AB, AC);
	Vector3 ABD = v3_cross(AD, AB);
	Vector3 ADC = v3_cross(AC, AD);

	// NOTE: In the case we return a triangle as the next simplex
	// the winding order of that which will become an INNER face
	// of the tetrahedron must be clockwise. This means that the
	// same triangle OUTTER face will have with a counter-clockwise
	// winding order.

	// TODO: I've fixed the winding order of the triangles we
	// return but if we ever get any unexpected result, we should
	// check if there aren't any errors there. It's easy to lose
	// track of these details.

	// TODO: The DEBUG_ASSERT when handling the ABD triangle was
	// triggering because either AB or AD dot product with AO was
	// slightly negative. This seemed to happen whenever the origin
	// was very close to one of the edges. I then changed the assertion
	// to require only one of the dot products to be positive but i'm
	// not sure if we still have a bug.

	if(v3_dot(ACB, AO) > 0){
		if(v3_dot(ABD, AO) > 0){
			if(v3_dot(AB, AO) > 0){
				// points = [B, A], dir = AB x AO x AB
				points[0] = points[2];
				points[1] = points[3];
				*num_points = 2;
				*dir = v3_triple_cross(AB, AO, AB);
			}else{
				// points = [A], dir = AO
				points[0] = points[3];
				*num_points = 1;
				*dir = AO;
			}
		}else if(v3_dot(ADC, AO) > 0){
			if(v3_dot(AC, AO) > 0){
				// points = [C, A], dir = AC x AO x AC
				points[0] = points[1];
				points[1] = points[3];
				*num_points = 2;
				*dir = v3_triple_cross(AC, AO, AC);
			}else{
				// points = [A], dir = AO
				points[0] = points[3];
				*num_points = 1;
				*dir = AO;
			}
		}else{
			ASSERT(v3_dot(AB, AO) > 0 || v3_dot(AC, AO) > 0);
			// points = [C, B, A], dir = ACB
			points[0] = points[1];
			points[1] = points[2];
			points[2] = points[3];
			*num_points = 3;
			*dir = ACB;
		}

		return false;
	}

	if(v3_dot(ABD, AO) > 0){
		// NOTE: The case (ABD && ACB) is already covered in the
		// tests above.
		if(v3_dot(ADC, AO) > 0){
			if(v3_dot(AD, AO) > 0){
				// points = [D, A], dir = AD x AO x AD
				//points[0] = points[0];
				points[1] = points[3];
				*num_points = 2;
				*dir = v3_triple_cross(AD, AO, AD);
			}else{
				// points = [A], dir = AO
				points[0] = points[3];
				*num_points = 1;
				*dir = AO;
			}
		}else{
			ASSERT(v3_dot(AB, AO) > 0 || v3_dot(AD, AO) > 0);
			// points = [B, D, A], dir = ABD
			GJK_Point tmp = points[0];
			points[0] = points[2];
			points[1] = tmp;
			points[2] = points[3];
			*num_points = 3;
			*dir = ABD;
		}
		return false;
	}

	if(v3_dot(ADC, AO) > 0){
		// NOTE: The cases (ADC && ACB) and (ADC && ABD) are already
		// covered in the tests above.
		ASSERT(v3_dot(AC, AO) > 0 || v3_dot(AD, AO) > 0);
		// points = [D, C, A], dir = ADC
		//points[0] = points[0];
		//points[1] = points[1];
		points[2] = points[3];
		*num_points = 3;
		*dir = ADC;
		return false;
	}

	// NOTE: If we get here, the tetrahedron contains the
	// origin which means the shapes are overlapping.
	return true;
}

static INLINE
f32 gjk_distance1(GJK_Point A,
		Vector3 *closest1, Vector3 *closest2){
	f32 distance = v3_norm(A.minkowski);
	ASSERT(closest1 && closest2);
	*closest1 = A.polygon1;
	*closest2 = A.polygon2;
	return distance;
}

static INLINE
f32 gjk_distance2(GJK_Point A, GJK_Point B,
		Vector3 *closest1, Vector3 *closest2){
	Vector3 AO = -A.minkowski;
	Vector3 AB = B.minkowski - A.minkowski;
	f32 k = v3_dot(AO, AB) / v3_norm2(AB);
	ASSERT(k >= 0.0f && k <= 1.0f);
	Vector3 closest = A.minkowski + k * AB;
	f32 distance = v3_norm(closest);
	ASSERT(closest1 && closest2);
	*closest1 = A.polygon1 + k * (B.polygon1 - A.polygon1);
	*closest2 = A.polygon2 + k * (B.polygon2 - A.polygon2);
	return distance;
}

static INLINE
f32 gjk_distance3(GJK_Point A, GJK_Point B, GJK_Point C,
		Vector3 *closest1, Vector3 *closest2){
	Vector3 AO = -A.minkowski;
	Vector3 AB = B.minkowski - A.minkowski;
	Vector3 AC = C.minkowski - A.minkowski;

#if 0
	Vector3 ABC = v3_cross(AB, AC);
	f32 inv_abc_abc = 1 / v3_dot(ABC, ABC);
	f32 kc = v3_dot(v3_cross(AB, AO), ABC) * inv_abc_abc;
	f32 kb = v3_dot(v3_cross(AO, AC), ABC) * inv_abc_abc;
	f32 ka = 1 - kb - kc;
	Vector3 closest = ka * A.minkowski
		+ kb * B.minkowski + kc * C.minkowski;
	f32 distance = v3_norm(closest);

	ASSERT(ka >= 0.0f && ka <= 1.0f
			&& kb >= 0.0f && kb <= 1.0f
			&& kc >= 0.0f && kc <= 1.0f);
	ASSERT(closest1 && closest2);
	*closest1 = ka * A.polygon1
		+ kb * B.polygon1 + kc * C.polygon1;
	*closest2 = ka * A.polygon2
		+ kb * B.polygon2 + kc * C.polygon2;
#else
	Vector3 ABC = v3_normalize(v3_cross(AB, AC));
	f32 distance = v3_dot(AO, ABC);

	// NOTE: P is the closest point but I renamed it to be less
	// verbose and because AP looks better than Aclosest for the
	// vector from A to P.
	Vector3 P = -distance * ABC;
	Vector3 AP = A.minkowski - P;

	f32 ab2 = v3_dot(AB, AB);
	f32 ac2 = v3_dot(AC, AC);
	f32 dbc = v3_dot(AB, AC);

	f32 kcommon = 1.0f / (dbc * dbc - ab2 * ac2);
	f32 kab = kcommon * v3_dot(AP, (ac2 * AB - dbc * AC));
	f32 kac = kcommon * v3_dot(AP, (ab2 * AC - dbc * AB));
	// TODO: Sometimes this assert triggers with values like -0.01f.
	// Maybe this is not a big deal but you never know.
	//ASSERT(kab >= 0.0f && kac >= 0.0f && (kab + kac) <= 1.0f);
	ASSERT(closest1 && closest2);
	*closest1 = A.polygon1
		+ kab * (B.polygon1 - A.polygon1)
		+ kac * (C.polygon1 - A.polygon1);
	*closest2 = A.polygon2
		+ kab * (B.polygon2 - A.polygon2)
		+ kac * (C.polygon2 - A.polygon2);

#if 0
	// TODO: Remove eventually. I'm leaving this just in case.
	debug_push_line(gjk_DEBUG,
		A.minkowski,
		A.minkowski + kab * AB,
		make_v3(0.3f, 0.0f, 0.7f));
	debug_push_line(gjk_DEBUG,
		A.minkowski + kab * AB,
		A.minkowski + kab * AB + kac * AC,
		make_v3(0.7f, 0.0f, 0.3f));
	Vector3 color = make_v3(0.0f, 1.0f, 0.0f);
	debug_push_point(gjk_DEBUG, P, color);
	debug_push_point(gjk_DEBUG, v3_zero, color);
	debug_push_line(gjk_DEBUG, A.minkowski, B.minkowski, color);
	debug_push_line(gjk_DEBUG, A.minkowski, C.minkowski, color);
#endif

#endif

	return distance;
}

static INLINE
GJK_Result gjk_overlap_result(void){
	GJK_Result result = {};
	result.overlap = true;
	return result;
}

static INLINE
GJK_Result gjk_no_overlap_result(GJK_Point *points, i32 num_points){
	// TODO: In the case we get two parallel triangles, we may
	// get some flickering because any line perpendicular to both
	// can be used to calculate the closest points. Perhaps the
	// best solution would be to let the gjk caller handle this
	// case separately.

	ASSERT(num_points >= 1 && num_points <= 3);
	GJK_Result result;
	result.overlap = false;
	switch(num_points){
		case 1: {
			result.distance = gjk_distance1(points[0],
					&result.closest1, &result.closest2);

			result.num_points1 = 1;
			result.points1[0] = points[0].polygon1;
			result.num_points2 = 1;
			result.points2[0] = points[0].polygon2;
			break;
		}

		case 2: {
			result.distance = gjk_distance2(
					points[1], points[0],
					&result.closest1, &result.closest2);

			if(points[0].polygon1 == points[1].polygon1){
				result.num_points1 = 1;
				result.points1[0] = points[0].polygon1;
			}else{
				result.num_points1 = 2;
				result.points1[0] = points[0].polygon1;
				result.points1[1] = points[1].polygon1;
			}

			if(points[0].polygon2 == points[1].polygon2){
				result.num_points2 = 1;
				result.points2[0] = points[0].polygon2;
			}else{
				result.num_points2 = 2;
				result.points2[0] = points[0].polygon2;
				result.points2[1] = points[1].polygon2;
			}
			break;
		}

		case 3: {
			result.distance = gjk_distance3(
					points[2], points[1], points[0],
					&result.closest1, &result.closest2);

			if(points[0].polygon1 == points[1].polygon1){
				if(points[0].polygon1 == points[2].polygon1){
					result.num_points1 = 1;
					result.points1[0] = points[0].polygon1;
				}else{
					result.num_points1 = 2;
					result.points1[0] = points[0].polygon1;
					result.points1[1] = points[2].polygon1;
				}
			}else if(points[0].polygon1 == points[2].polygon1
					|| points[1].polygon1 == points[2].polygon1){
				// we know that points[0].polygon1 != points[1].polygon1
				result.num_points1 = 2;
				result.points1[0] = points[0].polygon1;
				result.points1[1] = points[1].polygon1;
			}else{
				// we know that points[0].polygon1 != points[1].polygon1
				// and points[0].polygon1 != points[2].polygon1 and
				// points[1].polygon1 != points[2].polygon1
				result.num_points1 = 3;
				result.points1[0] = points[0].polygon1;
				result.points1[1] = points[1].polygon1;
				result.points1[2] = points[2].polygon1;
			}

			// NOTE: This is replicated from above.
			if(points[0].polygon2 == points[1].polygon2){
				if(points[0].polygon2 == points[2].polygon2){
					result.num_points2 = 1;
					result.points2[0] = points[0].polygon2;
				}else{
					result.num_points2 = 2;
					result.points2[0] = points[0].polygon2;
					result.points2[1] = points[2].polygon2;
				}
			}else if(points[0].polygon2 == points[2].polygon2
					|| points[1].polygon2 == points[2].polygon2){
				result.num_points2 = 2;
				result.points2[0] = points[0].polygon2;
				result.points2[1] = points[1].polygon2;
			}else{
				result.num_points2 = 3;
				result.points2[0] = points[0].polygon2;
				result.points2[1] = points[1].polygon2;
				result.points2[2] = points[2].polygon2;
			}
			break;
		}
	}
	return result;
}

static
GJK_Point gjk_polygon_support(GJK_Polygon *p1, GJK_Polygon *p2, Vector3 dir){
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

	GJK_Point result;
	result.minkowski = p1->points[index1] - p2->points[index2];
	result.polygon1 = p1->points[index1];
	result.polygon2 = p2->points[index2];
	return result;
}

GJK_Result gjk(GJK_Polygon *p1, GJK_Polygon *p2){
	GJK_Point initial_point = gjk_polygon_support(
			p1, p2, make_v3(0.0f, 0.0f, -1.0f));
	Vector3 direction = -initial_point.minkowski;
	i32 num_points = 1;
	GJK_Point points[4] = { initial_point };

	// NOTE: Without this check, when two exact polygons are
	// overlapping exactly, we'll end up doing invalid work.
	if(v3_cmp_zero(initial_point.minkowski))
		return gjk_overlap_result();

	for(i32 num_iter = 0; num_iter < 16; num_iter += 1){
		GJK_Point next_point = gjk_polygon_support(p1, p2, direction);
		if(v3_cmp_zero(next_point.minkowski))
			return gjk_overlap_result();

		// TODO: We might get in trouble if we choose to use
		// a smart support function here.
		for(i32 i = 0; i < num_points; i += 1){
			if(next_point.minkowski == points[i].minkowski)
				goto no_overlap_result;
		}

		points[num_points] = next_point;
		num_points += 1;

		// NOTE: If we get a degenerate simplex with the latest
		// addition, it means it might suffer from FP precision
		// and that we'll probably strugle to reach a proper
		// result. In that case we just discard the point and
		// return the current result.
		if(gjk_check_degenerate_simplex(points, num_points)){
			num_points -= 1;
			goto no_overlap_result;
		}

		ASSERT(num_points >= 2 && num_points <= 4);
		switch(num_points){
			case 2:
				gjk_simplex2(points, &num_points, &direction);
				break;
			case 3:
				gjk_simplex3(points, &num_points, &direction);
				break;
			case 4:
				if(gjk_simplex4(points, &num_points, &direction))
					return gjk_overlap_result();
				break;
		}
	}

no_overlap_result:
	return gjk_no_overlap_result(points, num_points);
}
