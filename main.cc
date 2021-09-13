#define OPENGL_DECLARE_FUNCTION_POINTERS 1
#include "opengl.hh"

#include <SDL.h>
#include "common.hh"
#include "math.hh"
#include "gjk.hh"

#define WINDOW_W 800
#define WINDOW_H 450

static SDL_Window *window;
static SDL_GLContext gl_context;

static
void *malloc_nofail(usize size){
	void *mem = malloc(size);
	if(!mem) abort();
	return mem;
}

// ----------------------------------------------------------------
// GL Helpers
// ----------------------------------------------------------------

static
GLuint gl_compile_shader(const char *debug_name,
		GLenum shader_type, const char *shader_text){
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_text, NULL);
	glCompileShader(shader);
	GLint compile_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE){
		char infolog[256];
		glGetShaderInfoLog(shader, EV_NARRAY(infolog), NULL, infolog);
		LOG_ERROR("failed to compile shader (%s, type = %d): (%s)\n",
				debug_name, shader_type, infolog);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static
GLuint gl_create_program(const char *debug_name, const char *vshader_text, const char *fshader_text){
	GLuint vshader = gl_compile_shader(debug_name, GL_VERTEX_SHADER, vshader_text);
	GLuint fshader = gl_compile_shader(debug_name, GL_FRAGMENT_SHADER, fshader_text);
	DEBUG_ASSERT(vshader != 0 && fshader != 0);

	GLuint program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	GLint link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if(link_status == GL_FALSE){
		char infolog[256];
		glGetProgramInfoLog(program, EV_NARRAY(infolog), NULL, infolog);
		LOG_ERROR("failed to link program (%s): %s\n", debug_name, infolog);
		glDeleteProgram(program);
		glDeleteShader(vshader);
		glDeleteShader(fshader);
		return 0;
	}
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	return program;
}

static
bool gl_load(void){
	bool gl_load_ok = true;
	#define GL_PROC(_1, name, _2)									\
		gl##name = (PFN_gl##name)SDL_GL_GetProcAddress("gl"#name);	\
		if(!gl##name){												\
			LOG_ERROR("unable to load `%s`\n", "gl"#name);			\
			gl_load_ok = false;										\
		}
	#include "opengl.inl"

	if(!gl_load_ok){
		LOG_ERROR("failed to load OpenGL API\n");
		return false;
	}
	return true;
}

#if BUILD_DEBUG
static
void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar *message,
		const void *userParam){
	LOG("OpenGL Error: %s\n", message);
}
#endif

static
void gl_init_state(void){
#if BUILD_DEBUG
	DEBUG_LOG("GL_VENDOR = %s\n", glGetString(GL_VENDOR));
	DEBUG_LOG("GL_RENDERER = %s\n", glGetString(GL_RENDERER));
	DEBUG_LOG("GL_VERSION = %s\n", glGetString(GL_VERSION));
	DEBUG_LOG("GL_SHADING_LANGUAGE_VERSION = %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//DEBUG_LOG("GL_EXTENSIONS = %s\n", glGetString(GL_EXTENSIONS));
	glDebugMessageCallback(gl_debug_callback, NULL);
	glEnable(GL_DEBUG_OUTPUT);
#endif
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	GLuint default_vao;
	glGenVertexArrays(1, &default_vao);
	glBindVertexArray(default_vao);
}

// ----------------------------------------------------------------
// Line Renderer
// ----------------------------------------------------------------

struct LineVertex{
	Vector3 position;
	Vector3 color;
};

struct LineRenderer{
	GLuint program;

	GLuint vbuffer;
	usize vbuffer_size;

	GLuint ubuffer;
	usize ubuffer_size;

	i32 max_vertices;
	i32 num_vertices;
	LineVertex *vertices;
};

struct RenderParams{
	Matrix4 pv;
};

LineRenderer liner_init(i32 max_lines){
	DEBUG_ASSERT(max_lines > 0 && max_lines <= 0x00FFFFFF);
	static const char *vshader =
		"#version 420\n"
		"layout(location = 0) in vec3 in_position;\n"
		"layout(location = 1) in vec3 in_color;\n"
		"layout(location = 0) out vec3 out_color;\n"
		"layout(std140, row_major, binding = 0)\n"
		"uniform RenderParams { mat4 pv; };\n"
		"void main(){\n"
		"	gl_Position = pv * vec4(in_position, 1.0);\n"
		"	out_color = in_color;\n"
		"}\n";
	static const char *fshader =
		"#version 420\n"
		"layout(location = 0) in vec3 in_color;\n"
		"layout(location = 0) out vec3 out_color;\n"
		"void main(){ out_color = in_color; }\n";


	// create shader program
	GLuint program = gl_create_program("line_renderer", vshader, fshader);
	DEBUG_ASSERT(program != 0);

	// create vertex buffer
	GLuint vbuffer;
	i32 max_vertices = 2 * max_lines;
	usize vbuffer_size = sizeof(LineVertex) * max_vertices;
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, vbuffer_size, NULL, GL_STATIC_DRAW);

	// create uniform buffer for render params
	GLuint ubuffer;
	usize ubuffer_size = sizeof(RenderParams);
	glGenBuffers(1, &ubuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubuffer);
	glBufferData(GL_UNIFORM_BUFFER, ubuffer_size, NULL, GL_STATIC_DRAW);

	LineRenderer result;
	result.program = program;
	result.vbuffer = vbuffer;
	result.vbuffer_size = vbuffer_size;
	result.ubuffer = ubuffer;
	result.ubuffer_size = ubuffer_size;
	result.max_vertices = max_vertices;
	result.num_vertices = 0;
	result.vertices = (LineVertex*)malloc_nofail(vbuffer_size);
	return result;
}

void liner_draw(LineRenderer *L, RenderParams *render_params){
	u32 num_vertices = L->num_vertices;
	if(num_vertices == 0)
		return;

	//glBindBuffer(GL_UNIFORM_BUFFER, L->ubuffer);
	//glBufferData(GL_UNIFORM_BUFFER, L->ubuffer_size, NULL, GL_STATIC_DRAW);
	//glBufferData(GL_UNIFORM_BUFFER, L->ubuffer_size, render_params, GL_STATIC_DRAW);

	DEBUG_ASSERT(L->ubuffer_size == sizeof(RenderParams));

	glUseProgram(L->program);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, L->ubuffer);
	glBufferData(GL_UNIFORM_BUFFER, L->ubuffer_size, NULL, GL_STATIC_DRAW);
	glBufferData(GL_UNIFORM_BUFFER, L->ubuffer_size, render_params, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, L->vbuffer);
	glBufferData(GL_ARRAY_BUFFER, L->vbuffer_size, NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, L->vbuffer_size, L->vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)12);

	glDrawArrays(GL_LINES, 0, num_vertices);

	L->num_vertices = 0;
}

void liner_push_line(LineRenderer *L, Vector3 p1, Vector3 p2, Vector3 color){
	DEBUG_ASSERT((L->num_vertices + 2) <= L->max_vertices);
	LineVertex *v = &L->vertices[L->num_vertices];
	L->num_vertices += 2;
	v[0].position = p1;
	v[0].color = color;
	v[1].position = p2;
	v[1].color = color;
}

void liner_push_aabb(LineRenderer *L, Rect3 aabb, Vector3 color){
	// bottom vertices
	Vector3 b1 = make_v3(aabb.min.x, aabb.min.y, aabb.min.z);
	Vector3 b2 = make_v3(aabb.max.x, aabb.min.y, aabb.min.z);
	Vector3 b3 = make_v3(aabb.max.x, aabb.max.y, aabb.min.z);
	Vector3 b4 = make_v3(aabb.min.x, aabb.max.y, aabb.min.z);
	// top vertices
	Vector3 t1 = make_v3(aabb.min.x, aabb.min.y, aabb.max.z);
	Vector3 t2 = make_v3(aabb.max.x, aabb.min.y, aabb.max.z);
	Vector3 t3 = make_v3(aabb.max.x, aabb.max.y, aabb.max.z);
	Vector3 t4 = make_v3(aabb.min.x, aabb.max.y, aabb.max.z);

	// bottom edges
	liner_push_line(L, b1, b2, color);
	liner_push_line(L, b2, b3, color);
	liner_push_line(L, b3, b4, color);
	liner_push_line(L, b4, b1, color);
	// middle edges
	liner_push_line(L, b1, t1, color);
	liner_push_line(L, b2, t2, color);
	liner_push_line(L, b3, t3, color);
	liner_push_line(L, b4, t4, color);
	// top edges
	liner_push_line(L, t1, t2, color);
	liner_push_line(L, t2, t3, color);
	liner_push_line(L, t3, t4, color);
	liner_push_line(L, t4, t1, color);
}

void liner_push_point(LineRenderer *L, Vector3 point, Vector3 color){
	Rect3 point_rect = make_rect3(
		make_v3(-0.02f, -0.02f, -0.02f),
		make_v3(+0.02f, +0.02f, +0.02f));
	liner_push_aabb(L, point_rect + point, color);
}

// ----------------------------------------------------------------
// Camera
// ----------------------------------------------------------------
struct Camera{
	Vector3 position;
	Vector3 direction;
	Vector3 up;
};

static
Camera cam_init(Vector3 pos, Vector3 dir, Vector3 up){
	DEBUG_ASSERT(f32_cmp_zero(v3_dot(dir, up)));
	Camera result;
	result.position = pos;
	result.direction = dir;
	result.up = up;
	return result;
}

static
void cam_move(Camera *cam, Vector3 movement){
	cam->position += movement;
}

static
void cam_turn_x(Camera *cam, f32 radians){
	Vector3 right = v3_cross(cam->direction, cam->up);
	Quaternion rot = quat_angle_axis(radians, right);
	cam->direction = v3_rotate(cam->direction, rot);
	cam->up = v3_rotate(cam->up, rot);
}

static
void cam_turn_y(Camera *cam, f32 radians){
	Quaternion rot = quat_angle_axis(radians, cam->up);
	cam->direction = v3_rotate(cam->direction, rot);
	// cam->up is unchanged
}

static
void cam_turn_z(Camera *cam, f32 radians){
	Quaternion rot = quat_angle_axis(radians, cam->direction);
	// cam->direction unchanged
	cam->up = v3_rotate(cam->up, rot);
}

static
Matrix4 cam_mat4(Camera *cam){
	Vector3 position = cam->position;
	Vector3 zaxis = -cam->direction;
	Vector3 xaxis = v3_cross(cam->up, zaxis);
	Vector3 yaxis = v3_cross(zaxis, xaxis);

	Matrix4 result;
	result.m11 = xaxis.x;
	result.m12 = xaxis.y;
	result.m13 = xaxis.z;
	result.m14 = -v3_dot(xaxis, position);

	result.m21 = yaxis.x;
	result.m22 = yaxis.y;
	result.m23 = yaxis.z;
	result.m24 = -v3_dot(yaxis, position);

	result.m31 = zaxis.x;
	result.m32 = zaxis.y;
	result.m33 = zaxis.z;
	result.m34 = -v3_dot(zaxis, position);

	result.m41 = 0.0f;
	result.m42 = 0.0f;
	result.m43 = 0.0f;
	result.m44 = 1.0f;
	return result;
}

// ----------------------------------------------------------------
// GJK
// ----------------------------------------------------------------

static
void gjk_draw_polygon_points(LineRenderer *L,
		GJK_Polygon *p, Vector3 color){
	for(i32 i = 0; i < p->num_points; i += 1)
		liner_push_point(L, p->points[i], color);
}

static
void gjk_draw_minkowski_points(LineRenderer *L,
		GJK_Polygon *p1, GJK_Polygon *p2, Vector3 color){
	for(i32 i = 0; i < p1->num_points; i += 1){
		for(i32 j = 0; j < p2->num_points; j += 1){
			liner_push_point(L,
				p1->points[i] - p2->points[j],
				color);
		}
	}
}

static
void gjk_draw_closest_feature(LineRenderer *L,
		Vector3 *points, i32 num_points, Vector3 color){
	DEBUG_ASSERT(num_points >= 1 && num_points <= 3);
	switch(num_points){
		case 1:
			liner_push_point(L, points[0], color);
			break;
		case 2:
			liner_push_line(L, points[0], points[1], color);
			break;
		case 3:
			liner_push_line(L, points[0], points[1], color);
			liner_push_line(L, points[1], points[2], color);
			liner_push_line(L, points[2], points[0], color);
			break;
	}
}

void gjk_test1(LineRenderer *L,
		bool swap_polygon_order,
		bool draw_minkowski_points,
		Vector3 position1, f32 angle2){

	Vector3 points1[] = {
		make_v3(-1.0f, +1.0f, -1.0f) + position1,
		make_v3(+1.0f, +1.0f, -1.0f) + position1,
		make_v3(+0.0f, -1.0f, -1.0f) + position1,
		make_v3(+0.0f, +0.0f, +1.0f) + position1,
	};

#if 1
	Vector3 points2[] = {
		make_v3(-1.0f, +1.0f, -1.0f),
		make_v3(+1.0f, +1.0f, -1.0f),
		make_v3(+0.0f, -1.0f, -1.0f),
		make_v3(+0.0f, +0.0f, +1.0f),
	};
#else
	Vector3 points2[] = {
		make_v3(-1.0f, +1.0f, -1.0f),
		make_v3(+1.0f, +1.0f, -1.0f),
		make_v3(+1.0f, -1.0f, -1.0f),
		make_v3(-1.0f, -1.0f, -1.0f),
		make_v3(-1.0f, +1.0f, +1.0f),
		make_v3(+1.0f, +1.0f, +1.0f),
		make_v3(+1.0f, -1.0f, +1.0f),
		make_v3(-1.0f, -1.0f, +1.0f),
	};
#endif

	// rotate points2
	DEBUG_ASSERT(angle2 >= 0.0f && angle2 <= EV_2PI);
	Quaternion rotation = quat_angle_axis(angle2, make_v3(0.0f, 0.0f, 1.0f));
	for(i32 i = 0; i < EV_NARRAY(points2); i += 1)
		points2[i] = v3_rotate(points2[i], rotation);

	GJK_Polygon p1 = make_gjk_polygon(points1, EV_NARRAY(points1));
	GJK_Polygon p2 = make_gjk_polygon(points2, EV_NARRAY(points2));

	if(swap_polygon_order){
		GJK_Polygon tmp = p1;
		p1 = p2;
		p2 = tmp;
	}

	GJK_Result result = gjk(&p1, &p2);
	//LOG("gjk result: overlap = %d, distance = %f,"
	//	" num_points1 = %d, num_points2 = %d\n",
	//	result.overlap, result.distance,
	//	result.num_points1, result.num_points2);
	Vector3 polygon_color = result.overlap
		? make_v3(0.90f, 0.00f, 0.10f)
		: make_v3(0.75f, 0.15f, 0.65f);
	gjk_draw_polygon_points(L, &p1, polygon_color);
	gjk_draw_polygon_points(L, &p2, polygon_color);

	if(draw_minkowski_points){
		gjk_draw_minkowski_points(L, &p1, &p2,
			make_v3(1.0f, 1.0f, 1.0f));
	}

	if(!result.overlap){
		Vector3 color1 = make_v3(0.5f, 0.65f, 0.15f);
		gjk_draw_closest_feature(L, result.points1,
			result.num_points1, color1);

		Vector3 color2 = make_v3(0.15f, 0.65f, 0.5f);
		gjk_draw_closest_feature(L, result.points2,
			result.num_points2, color2);

		liner_push_line(L,
			result.closest1,
			result.closest2,
			make_v3(1.0f, 0.0f, 0.0f));
	}
}

void gjk_test2(LineRenderer *L,
		bool swap_polygon_order,
		bool draw_minkowski_points,
		Vector3 position1, f32 angle2){
	Vector3 points1[] = {
		make_v3(-1.0f, +1.0f, -1.0f) + position1,
		make_v3(+1.0f, +1.0f, -1.0f) + position1,
		make_v3(+0.0f, -1.0f, -1.0f) + position1,
		make_v3(+0.0f, +0.0f, +1.0f) + position1,
	};
	Vector3 points2[] = {
		make_v3(-1.0f, +1.0f, -1.0f),
		make_v3(+1.0f, +1.0f, -1.0f),
		make_v3(+0.0f, -1.0f, -1.0f),
		make_v3(+0.0f, +0.0f, +1.0f),
	};

	// rotate points2
	Quaternion rotation = quat_angle_axis(angle2, make_v3(0.0f, 0.0f, 1.0f));
	for(i32 i = 0; i < EV_NARRAY(points2); i += 1)
		points2[i] = v3_rotate(points2[i], rotation);

	GJK_Polygon p1 = make_gjk_polygon(points1, EV_NARRAY(points1));
	GJK_Polygon p2 = make_gjk_polygon(points2, EV_NARRAY(points2));

	if(swap_polygon_order){
		GJK_Polygon tmp = p1;
		p1 = p2;
		p2 = tmp;
	}

	bool result = gjk_collision_test(&p1, &p2);
	//LOG("gjk_collistion_test: overlap = %d\n", result);
	Vector3 polygon_color = result
		? make_v3(0.90f, 0.00f, 0.10f)
		: make_v3(0.75f, 0.15f, 0.65f);
	gjk_draw_polygon_points(L, &p1, polygon_color);
	gjk_draw_polygon_points(L, &p2, polygon_color);

	if(draw_minkowski_points){
		gjk_draw_minkowski_points(L, &p1, &p2,
			make_v3(1.0f, 1.0f, 1.0f));
	}
}

// ----------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------

int SDL_main(int argc, char **argv){
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
		return -1;

	// window and graphics initialization
	// ----------------------------------------------------------------

	// set GL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#if BUILD_DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// create SDL window
	window = SDL_CreateWindow("Everlot", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, WINDOW_W, WINDOW_H,
			SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if(!window){
		LOG_ERROR("unable to create SDL window: %s\n", SDL_GetError());
		return -1;
	}

	// create GL context
	gl_context = SDL_GL_CreateContext(window);
	if(!gl_context){
		LOG_ERROR("unable to create GL context: %s\n", SDL_GetError());
		return -1;
	}

	// load and init GL
	if(!gl_load()){
		LOG_ERROR("failed to load OpenGL\n");
		return -1;
	}
	gl_init_state();

	// activate v-sync
	SDL_GL_SetSwapInterval(1);

	// rendering state
	// ----------------------------------------------------------------
	LineRenderer L = liner_init(UINT16_MAX);
	Camera camera = cam_init(
		make_v3(0.0f, 0.0f, 16.0f),
		make_v3(0.0f, 0.0f, -1.0f),
		make_v3(0.0f, 1.0f, 0.0f));
	Matrix4 projection = mat4_perspective(16.0f / 9.0f, (f32)(EV_PI / 4), 0.01f, 100.0f);

	// input state
	// ----------------------------------------------------------------
	i32 last_mouse_x, last_mouse_y;
	SDL_GetMouseState(&last_mouse_x, &last_mouse_y);

	i32 gjk_test = 1;
	bool draw_minkowski_points = false;
	bool swap_polygon_order = false;

	bool camera_move_forward = false;
	bool camera_move_left = false;
	bool camera_move_back = false;
	bool camera_move_right = false;
	bool camera_roll_left = false;
	bool camera_roll_right = false;

	bool polygon1_move_n = false;
	bool polygon1_move_w = false;
	bool polygon1_move_s = false;
	bool polygon1_move_e = false;
	bool polygon1_move_up = false;
	bool polygon1_move_down = false;

	const f32 camera_move_speed = 4.0f;					// m/s
	const f32 camera_turn_speed = (f32)(EV_PI / 10);	// rad/s
	const f32 polygon1_move_speed = 1.0f;				// m/s
	const f32 polygon2_turn_speed = (f32)(EV_PI / 4);	// rad/s

	Vector3 polygon1_position = {};
	f32 polygon2_angle = 0.0f;

	// print controls
	// ----------------------------------------------------------------
	LOG("TESTS:\n");
	LOG(" [1]: using gjk()\n");
	LOG(" [2]: using gjk_collision_test()\n");
	LOG("COMMANDS:\n");
	LOG(" [M]: toggle minkowski sum points\n");
	LOG(" [X]: swap order of polygons in the minkowski sum\n");
	LOG("CAMERA:\n");
	LOG(" [W]: move forward\n");
	LOG(" [A]: move left\n");
	LOG(" [S]: move back\n");
	LOG(" [D]: move right\n");
	LOG(" [MOUSE1]: hold and move the mouse to"
		" move the direction of the camera\n");
	LOG("POLYGON 1:\n");
	LOG(" [UP]: move north (positive y-axis)\n");
	LOG(" [LEFT]: move west (negative x-axis)\n");
	LOG(" [DOWN]: move south (negative y-axis)\n");
	LOG(" [RIGHT]: move east (positive x-axis)\n");
	LOG(" [PGUP]: move up (positive z-axis)\n");
	LOG(" [PGDN]: move down (negative z-axis)\n");


	// main loop
	// ----------------------------------------------------------------

	// NOTE: These SDL_DROP* events require us to free
	// ev.drop.file memory so we can just ignore them
	// since we're not interested.
	SDL_EventState(SDL_DROPBEGIN, SDL_IGNORE);
	SDL_EventState(SDL_DROPFILE, SDL_IGNORE);
	SDL_EventState(SDL_DROPTEXT, SDL_IGNORE);
	SDL_EventState(SDL_DROPCOMPLETE, SDL_IGNORE);

	// show window
	SDL_ShowWindow(window);

	f64 inv_counter_frequency = 1 / (f64)SDL_GetPerformanceFrequency();
	u64 prev_counter = SDL_GetPerformanceCounter();
	u64 cur_counter;
	f64 frame_dt = 0.1;
	while(true){
		f32 dt = (f32)frame_dt;
		SDL_Event ev;
		while(SDL_PollEvent(&ev)){
			switch(ev.type){
				case SDL_QUIT:
					return 0;
				case SDL_KEYDOWN:
				case SDL_KEYUP:{
					bool keydown = (ev.key.state == SDL_PRESSED);
					switch(ev.key.keysym.sym){
						// tests
						case '1':
							if(keydown){
								gjk_test = 1;
								LOG("TEST = 1\n");
							}
							break;
						case '2':
							if(keydown){	
								gjk_test = 2;
								LOG("TEST = 2\n");
							}
							break;
						// variables
						case 'm':
							if(keydown){
								draw_minkowski_points = !draw_minkowski_points;
							}
							break;
						case 'x':
							if(keydown){
								swap_polygon_order = !swap_polygon_order;
							}
							break;

						// camera control
						case 'w':
							camera_move_forward = keydown;
							break;
						case 'a':
							camera_move_left = keydown;
							break;
						case 's':
							camera_move_back = keydown;
							break;
						case 'd':
							camera_move_right = keydown;
							break;
						case 'q':
							camera_roll_left = keydown;
							break;
						case 'e':
							camera_roll_right = keydown;
							break;

						// polygon control
						case SDLK_UP:
							polygon1_move_n = keydown;
							break;
						case SDLK_LEFT:
							polygon1_move_w = keydown;
							break;
						case SDLK_DOWN:
							polygon1_move_s = keydown;
							break;
						case SDLK_RIGHT:
							polygon1_move_e = keydown;
							break;
						case SDLK_PAGEUP:
							polygon1_move_up = keydown;
							break;
						case SDLK_PAGEDOWN:
							polygon1_move_down = keydown;
							break;
					}
					break;
				}
			}
		}

		{
			i32 walk = 0;
			i32 strafe = 0;
			if(camera_move_forward)
				walk += 1;
			if(camera_move_left)
				strafe -= 1;
			if(camera_move_back)
				walk -= 1;
			if(camera_move_right)
				strafe += 1;

			if(walk || strafe){
				Vector3 walk_dir = camera.direction;
				Vector3 strafe_dir = v3_cross(camera.direction, camera.up);
				Vector3 movement = (f32)walk * walk_dir + (f32)strafe * strafe_dir;
				Vector3 movement_dir = v3_normalize(movement);

				f32 move_amount = camera_move_speed * dt;
				cam_move(&camera, move_amount * movement_dir);
			}
		}

		{
			i32 mouse_x, mouse_y;
			u32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
			if(mouse_buttons & SDL_BUTTON_LMASK){
				i32 mouse_dx = last_mouse_x - mouse_x;
				i32 mouse_dy = last_mouse_y - mouse_y;
				if(mouse_dx){
					f32 turn_amount = mouse_dx * camera_turn_speed * dt;
					cam_turn_y(&camera, turn_amount);
				}
				if(mouse_dy){
					f32 turn_amount = mouse_dy * camera_turn_speed * dt;
					cam_turn_x(&camera, turn_amount);
				}
			}
			last_mouse_x = mouse_x;
			last_mouse_y = mouse_y;
		}

		{
			i32 roll = 0;
			if(camera_roll_left)
				roll -= 1;
			if(camera_roll_right)
				roll += 1;

			if(roll){
				f32 turn_amount = roll * camera_turn_speed * dt;
				cam_turn_z(&camera, turn_amount);
			}
		}

		{
			i32 move_x = 0;
			i32 move_y = 0;
			i32 move_z = 0;
			if(polygon1_move_n)
				move_y += 1;
			if(polygon1_move_w)
				move_x -= 1;
			if(polygon1_move_s)
				move_y -= 1;
			if(polygon1_move_e)
				move_x += 1;
			if(polygon1_move_up)
				move_z += 1;
			if(polygon1_move_down)
				move_z -= 1;

			if(move_x || move_y || move_z){
				f32 move_amount = polygon1_move_speed * dt;
				Vector3 dir = v3_normalize(make_v3((f32)move_x, (f32)move_y, (f32)move_z));
				polygon1_position += move_amount * dir;
			}
		}

		{
			f32 turn_amount = polygon2_turn_speed * dt;
			polygon2_angle += turn_amount;
			if(polygon2_angle > EV_2PI)
				polygon2_angle = 0.0f;
		}

		// update and render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		switch(gjk_test){
			default:
			case 1:
				gjk_test1(&L,
					swap_polygon_order,
					draw_minkowski_points,
					polygon1_position, polygon2_angle);
				break;
			case 2:
				gjk_test2(&L,
					draw_minkowski_points,
					swap_polygon_order,
					polygon1_position, polygon2_angle);
				break;
		}

		RenderParams render_params;
		render_params.pv = projection * cam_mat4(&camera);
		liner_draw(&L, &render_params);
		SDL_GL_SwapWindow(window);

		// calculate frame time
		cur_counter = SDL_GetPerformanceCounter();
		frame_dt = (cur_counter - prev_counter) * inv_counter_frequency;
		prev_counter = cur_counter;

		// TEMPORARY
		{
			static f64 frame_dt_cache[120];
			static u32 frame_count = 0;

			frame_dt_cache[frame_count++] = frame_dt;
			if(frame_count >= 120){
				f64 avg, min, max;
				avg = frame_dt_cache[0];
				min = max = frame_dt_cache[0];
				for(u32 i = 1; i < frame_count; i += 1){
					avg += frame_dt_cache[i];
					if(frame_dt_cache[i] < min)
						min = frame_dt_cache[i];
					else if(frame_dt_cache[i] > max)
						max = frame_dt_cache[i];
				}
				avg /= frame_count;

				char fps_text[64];
				// min frame_dt = max fps
				// max frame_dt = min fps
				snprintf(fps_text, 64,
					"FPS: avg = %g, min = %g, max = %g\n",
					1.0 / avg, 1.0 / max, 1.0 / min);
				SDL_SetWindowTitle(window, fps_text);
				frame_count = 0;
			}
		}
	}
}
