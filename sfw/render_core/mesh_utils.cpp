/*************************************************************************/
/*  Most of these are originally from primitive_meshes.cpp               */
/*  From https://github.com/Relintai/pandemonium_engine (MIT)            */
/*************************************************************************/

//--STRIP
#include "render_core/mesh_utils.h"
//--STRIP

void MeshUtils::create_cone(Ref<Mesh> mesh) {
	if (!mesh.is_valid()) {
		return;
	}

	uint32_t vc = mesh->vertices.size();

	//eleje
	mesh->add_color(1, 0, 0);
	mesh->add_vertex3(0, 0.5, 0);

	mesh->add_color(1, 0, 0);
	mesh->add_vertex3(-0.5, -0.5, 0.5);

	mesh->add_color(1, 0, 0);
	mesh->add_vertex3(0.5, -0.5, 0.5);

	mesh->add_triangle(0 + vc, 1 + vc, 2 + vc);

	//bal
	mesh->add_color(0, 1, 0);
	mesh->add_vertex3(0, 0.5, 0);

	mesh->add_color(0, 1, 0);
	mesh->add_vertex3(-0.5, -0.5, -0.5);

	mesh->add_color(0, 1, 0);
	mesh->add_vertex3(-0.5, -0.5, 0.5);

	mesh->add_triangle(3 + vc, 4 + vc, 5 + vc);

	//jobb
	mesh->add_color(0, 0, 1);
	mesh->add_vertex3(0, 0.5, 0);

	mesh->add_color(0, 0, 1);
	mesh->add_vertex3(0.5, -0.5, 0.5);

	mesh->add_color(0, 0, 1);
	mesh->add_vertex3(0.5, -0.5, -0.5);

	mesh->add_triangle(6 + vc, 7 + vc, 8 + vc);

	//hátulja
	mesh->add_color(1, 1, 0);
	mesh->add_vertex3(0, 0.5, 0);

	mesh->add_color(1, 1, 0);
	mesh->add_vertex3(0.5, -0.5, -0.5);

	mesh->add_color(1, 1, 0);
	mesh->add_vertex3(-0.5, -0.5, -0.5);

	mesh->add_triangle(9 + vc, 10 + vc, 11 + vc);

	//alja

	mesh->add_color(1, 0, 1);
	mesh->add_vertex3(-0.5, -0.5, -0.5);

	mesh->add_color(1, 0, 1);
	mesh->add_vertex3(0.5, -0.5, 0.5);

	mesh->add_color(1, 0, 1);
	mesh->add_vertex3(-0.5, -0.5, 0.5);

	mesh->add_color(1, 0, 1);
	mesh->add_vertex3(0.5, -0.5, -0.5);

	mesh->add_triangle(12 + vc, 13 + vc, 14 + vc);
	mesh->add_triangle(13 + vc, 12 + vc, 15 + vc);
}

/*
void MeshUtils::create_capsule(Array &p_arr, const float radius, const float mid_height, const int radial_segments, const int rings) {
	//radial_segments = p_segments > 4 ? p_segments : 4;
	//rings = p_rings > 1 ? p_rings : 1;

	int i, j, prevrow, thisrow, point;
	float x, y, z, u, v, w;
	float onethird = 1.0 / 3.0;
	float twothirds = 2.0 / 3.0;

	// note, this has been aligned with our collision shape but I've left the descriptions as top/middle/bottom

	PoolVector<Vector3> points;
	PoolVector<Vector3> normals;
	PoolVector<float> tangents;
	PoolVector<Vector2> uvs;
	PoolVector<int> indices;
	point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

	// top hemisphere
	thisrow = 0;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		v = j;

		v /= (rings + 1);
		w = sin(0.5 * Math_PI * v);
		z = radius * cos(0.5 * Math_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = sin(u * (Math_PI * 2.0));
			y = -cos(u * (Math_PI * 2.0));

			Vector3 p = Vector3(x * radius * w, y * radius * w, z);
			points.push_back(p + Vector3(0.0, 0.0, 0.5 * mid_height));
			normals.push_back(p.normalized());
			ADD_TANGENT(-y, x, 0.0, 1.0)
			uvs.push_back(Vector2(u, v * onethird));
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};
		};

		prevrow = thisrow;
		thisrow = point;
	};

	// cylinder
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		v = j;
		v /= (rings + 1);

		z = mid_height * v;
		z = (mid_height * 0.5) - z;

		for (i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = sin(u * (Math_PI * 2.0));
			y = -cos(u * (Math_PI * 2.0));

			Vector3 p = Vector3(x * radius, y * radius, z);
			points.push_back(p);
			normals.push_back(Vector3(x, y, 0.0));
			ADD_TANGENT(-y, x, 0.0, 1.0)
			uvs.push_back(Vector2(u, onethird + (v * onethird)));
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};
		};

		prevrow = thisrow;
		thisrow = point;
	};

	// bottom hemisphere
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		v = j;

		v /= (rings + 1);
		v += 1.0;
		w = sin(0.5 * Math_PI * v);
		z = radius * cos(0.5 * Math_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			float u2 = i;
			u2 /= radial_segments;

			x = sin(u2 * (Math_PI * 2.0));
			y = -cos(u2 * (Math_PI * 2.0));

			Vector3 p = Vector3(x * radius * w, y * radius * w, z);
			points.push_back(p + Vector3(0.0, 0.0, -0.5 * mid_height));
			normals.push_back(p.normalized());
			ADD_TANGENT(-y, x, 0.0, 1.0)
			uvs.push_back(Vector2(u2, twothirds + ((v - 1.0) * onethird)));
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};
		};

		prevrow = thisrow;
		thisrow = point;
	};

	p_arr[RS::ARRAY_VERTEX] = points;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
	p_arr[RS::ARRAY_INDEX] = indices;
}

void MeshUtils::create_cube(Array &p_arr, const Vector3 size, const int subdivide_w, const int subdivide_h, const int subdivide_d) {
	//subdivide_w = p_divisions > 0 ? p_divisions : 0;
	//subdivide_d = p_divisions > 0 ? p_divisions : 0;

	int i, j, prevrow, thisrow, point;
	float x, y, z;
	float onethird = 1.0 / 3.0;
	float twothirds = 2.0 / 3.0;

	Vector3 start_pos = size * -0.5;

	// set our bounding box

	PoolVector<Vector3> points;
	PoolVector<Vector3> normals;
	PoolVector<float> tangents;
	PoolVector<Vector2> uvs;
	PoolVector<int> indices;
	point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

	// front + back
	y = start_pos.y;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= subdivide_h + 1; j++) {
		x = start_pos.x;
		for (i = 0; i <= subdivide_w + 1; i++) {
			float u = i;
			float v = j;
			u /= (3.0 * (subdivide_w + 1.0));
			v /= (2.0 * (subdivide_h + 1.0));

			// front
			points.push_back(Vector3(x, -y, -start_pos.z)); // double negative on the Z!
			normals.push_back(Vector3(0.0, 0.0, 1.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(u, v));
			point++;

			// back
			points.push_back(Vector3(-x, -y, start_pos.z));
			normals.push_back(Vector3(0.0, 0.0, -1.0));
			ADD_TANGENT(-1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(twothirds + u, v));
			point++;

			if (i > 0 && j > 0) {
				int i2 = i * 2;

				// front
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// back
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			};

			x += size.x / (subdivide_w + 1.0);
		};

		y += size.y / (subdivide_h + 1.0);
		prevrow = thisrow;
		thisrow = point;
	};

	// left + right
	y = start_pos.y;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_h + 1); j++) {
		z = start_pos.z;
		for (i = 0; i <= (subdivide_d + 1); i++) {
			float u = i;
			float v = j;
			u /= (3.0 * (subdivide_d + 1.0));
			v /= (2.0 * (subdivide_h + 1.0));

			// right
			points.push_back(Vector3(-start_pos.x, -y, -z));
			normals.push_back(Vector3(1.0, 0.0, 0.0));
			ADD_TANGENT(0.0, 0.0, -1.0, 1.0);
			uvs.push_back(Vector2(onethird + u, v));
			point++;

			// left
			points.push_back(Vector3(start_pos.x, -y, z));
			normals.push_back(Vector3(-1.0, 0.0, 0.0));
			ADD_TANGENT(0.0, 0.0, 1.0, 1.0);
			uvs.push_back(Vector2(u, 0.5 + v));
			point++;

			if (i > 0 && j > 0) {
				int i2 = i * 2;

				// right
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// left
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			};

			z += size.z / (subdivide_d + 1.0);
		};

		y += size.y / (subdivide_h + 1.0);
		prevrow = thisrow;
		thisrow = point;
	};

	// top + bottom
	z = start_pos.z;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_d + 1); j++) {
		x = start_pos.x;
		for (i = 0; i <= (subdivide_w + 1); i++) {
			float u = i;
			float v = j;
			u /= (3.0 * (subdivide_w + 1.0));
			v /= (2.0 * (subdivide_d + 1.0));

			// top
			points.push_back(Vector3(-x, -start_pos.y, -z));
			normals.push_back(Vector3(0.0, 1.0, 0.0));
			ADD_TANGENT(-1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(onethird + u, 0.5 + v));
			point++;

			// bottom
			points.push_back(Vector3(x, start_pos.y, -z));
			normals.push_back(Vector3(0.0, -1.0, 0.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(twothirds + u, 0.5 + v));
			point++;

			if (i > 0 && j > 0) {
				int i2 = i * 2;

				// top
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// bottom
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			};

			x += size.x / (subdivide_w + 1.0);
		};

		z += size.z / (subdivide_d + 1.0);
		prevrow = thisrow;
		thisrow = point;
	};

	p_arr[RS::ARRAY_VERTEX] = points;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
	p_arr[RS::ARRAY_INDEX] = indices;
}

void MeshUtils::create_cylinder(Array &p_arr, float top_radius, float bottom_radius, float height, int radial_segments, int rings) {
	//radial_segments = p_segments > 4 ? p_segments : 4;
	//rings = p_rings > 0 ? p_rings : 0;

	int i, j, prevrow, thisrow, point;
	float x, y, z, u, v, radius;

	PoolVector<Vector3> points;
	PoolVector<Vector3> normals;
	PoolVector<float> tangents;
	PoolVector<Vector2> uvs;
	PoolVector<int> indices;
	point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

	thisrow = 0;
	prevrow = 0;
	const real_t side_normal_y = (bottom_radius - top_radius) / height;
	for (j = 0; j <= (rings + 1); j++) {
		v = j;
		v /= (rings + 1);

		radius = top_radius + ((bottom_radius - top_radius) * v);

		y = height * v;
		y = (height * 0.5) - y;

		for (i = 0; i <= radial_segments; i++) {
			u = i;
			u /= radial_segments;

			x = sin(u * (Math_PI * 2.0));
			z = cos(u * (Math_PI * 2.0));

			Vector3 p = Vector3(x * radius, y, z * radius);
			points.push_back(p);
			normals.push_back(Vector3(x, side_normal_y, z).normalized());
			ADD_TANGENT(z, 0.0, -x, 1.0)
			uvs.push_back(Vector2(u, v * 0.5));
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};
		};

		prevrow = thisrow;
		thisrow = point;
	};

	// add top
	if (top_radius > 0.0) {
		y = height * 0.5;

		thisrow = point;
		points.push_back(Vector3(0.0, y, 0.0));
		normals.push_back(Vector3(0.0, 1.0, 0.0));
		ADD_TANGENT(1.0, 0.0, 0.0, 1.0)
		uvs.push_back(Vector2(0.25, 0.75));
		point++;

		for (i = 0; i <= radial_segments; i++) {
			float r = i;
			r /= radial_segments;

			x = sin(r * (Math_PI * 2.0));
			z = cos(r * (Math_PI * 2.0));

			u = ((x + 1.0) * 0.25);
			v = 0.5 + ((z + 1.0) * 0.25);

			Vector3 p = Vector3(x * top_radius, y, z * top_radius);
			points.push_back(p);
			normals.push_back(Vector3(0.0, 1.0, 0.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0)
			uvs.push_back(Vector2(u, v));
			point++;

			if (i > 0) {
				indices.push_back(thisrow);
				indices.push_back(point - 1);
				indices.push_back(point - 2);
			};
		};
	};

	// add bottom
	if (bottom_radius > 0.0) {
		y = height * -0.5;

		thisrow = point;
		points.push_back(Vector3(0.0, y, 0.0));
		normals.push_back(Vector3(0.0, -1.0, 0.0));
		ADD_TANGENT(1.0, 0.0, 0.0, 1.0)
		uvs.push_back(Vector2(0.75, 0.75));
		point++;

		for (i = 0; i <= radial_segments; i++) {
			float r = i;
			r /= radial_segments;

			x = sin(r * (Math_PI * 2.0));
			z = cos(r * (Math_PI * 2.0));

			u = 0.5 + ((x + 1.0) * 0.25);
			v = 1.0 - ((z + 1.0) * 0.25);

			Vector3 p = Vector3(x * bottom_radius, y, z * bottom_radius);
			points.push_back(p);
			normals.push_back(Vector3(0.0, -1.0, 0.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0)
			uvs.push_back(Vector2(u, v));
			point++;

			if (i > 0) {
				indices.push_back(thisrow);
				indices.push_back(point - 2);
				indices.push_back(point - 1);
			};
		};
	};

	p_arr[RS::ARRAY_VERTEX] = points;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
	p_arr[RS::ARRAY_INDEX] = indices;
}

void MeshUtils::create_plane(Array &p_arr, Size2 size, int subdivide_w, int subdivide_d, Vector3 center_offset) {
	//subdivide_w = p_divisions > 0 ? p_divisions : 0;
	//subdivide_d = p_divisions > 0 ? p_divisions : 0;

	int i, j, prevrow, thisrow, point;
	float x, z;

	Size2 start_pos = size * -0.5;

	PoolVector<Vector3> points;
	PoolVector<Vector3> normals;
	PoolVector<float> tangents;
	PoolVector<Vector2> uvs;
	PoolVector<int> indices;
	point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

	// top + bottom
	z = start_pos.y;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_d + 1); j++) {
		x = start_pos.x;
		for (i = 0; i <= (subdivide_w + 1); i++) {
			float u = i;
			float v = j;
			u /= (subdivide_w + 1.0);
			v /= (subdivide_d + 1.0);

			points.push_back(Vector3(-x, 0.0, -z) + center_offset);
			normals.push_back(Vector3(0.0, 1.0, 0.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(1.0 - u, 1.0 - v)); // 1.0 - uv to match orientation with Quad
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};

			x += size.x / (subdivide_w + 1.0);
		};

		z += size.y / (subdivide_d + 1.0);
		prevrow = thisrow;
		thisrow = point;
	};

	p_arr[RS::ARRAY_VERTEX] = points;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
	p_arr[RS::ARRAY_INDEX] = indices;
}

void MeshUtils::create_prism(Array &p_arr, float left_to_right, Vector3 size, int subdivide_w, int subdivide_h, int subdivide_d) {
	//subdivide_w = p_divisions > 0 ? p_divisions : 0;
	//subdivide_h = p_divisions > 0 ? p_divisions : 0;
	//subdivide_d = p_divisions > 0 ? p_divisions : 0;

	int i, j, prevrow, thisrow, point;
	float x, y, z;
	float onethird = 1.0 / 3.0;
	float twothirds = 2.0 / 3.0;

	Vector3 start_pos = size * -0.5;

	// set our bounding box

	PoolVector<Vector3> points;
	PoolVector<Vector3> normals;
	PoolVector<float> tangents;
	PoolVector<Vector2> uvs;
	PoolVector<int> indices;
	point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

	// front + back 
	y = start_pos.y;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_h + 1); j++) {
		float scale = (y - start_pos.y) / size.y;
		float scaled_size_x = size.x * scale;
		float start_x = start_pos.x + (1.0 - scale) * size.x * left_to_right;
		float offset_front = (1.0 - scale) * onethird * left_to_right;
		float offset_back = (1.0 - scale) * onethird * (1.0 - left_to_right);

		x = 0.0;
		for (i = 0; i <= (subdivide_w + 1); i++) {
			float u = i;
			float v = j;
			u /= (3.0 * (subdivide_w + 1.0));
			v /= (2.0 * (subdivide_h + 1.0));

			u *= scale;

			// front
			points.push_back(Vector3(start_x + x, -y, -start_pos.z)); // double negative on the Z!
			normals.push_back(Vector3(0.0, 0.0, 1.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(offset_front + u, v));
			point++;

			// back
			points.push_back(Vector3(start_x + scaled_size_x - x, -y, start_pos.z));
			normals.push_back(Vector3(0.0, 0.0, -1.0));
			ADD_TANGENT(-1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(twothirds + offset_back + u, v));
			point++;

			if (i > 0 && j == 1) {
				int i2 = i * 2;

				// front
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// back
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			} else if (i > 0 && j > 0) {
				int i2 = i * 2;

				// front
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// back
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			};

			x += scale * size.x / (subdivide_w + 1.0);
		};

		y += size.y / (subdivide_h + 1.0);
		prevrow = thisrow;
		thisrow = point;
	};

	// left + right
	Vector3 normal_left, normal_right;

	normal_left = Vector3(-size.y, size.x * left_to_right, 0.0);
	normal_right = Vector3(size.y, size.x * (1.0 - left_to_right), 0.0);
	normal_left.normalize();
	normal_right.normalize();

	y = start_pos.y;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_h + 1); j++) {
		float left, right;
		float scale = (y - start_pos.y) / size.y;

		left = start_pos.x + (size.x * (1.0 - scale) * left_to_right);
		right = left + (size.x * scale);

		z = start_pos.z;
		for (i = 0; i <= (subdivide_d + 1); i++) {
			float u = i;
			float v = j;
			u /= (3.0 * (subdivide_d + 1.0));
			v /= (2.0 * (subdivide_h + 1.0));

			// right
			points.push_back(Vector3(right, -y, -z));
			normals.push_back(normal_right);
			ADD_TANGENT(0.0, 0.0, -1.0, 1.0);
			uvs.push_back(Vector2(onethird + u, v));
			point++;

			// left
			points.push_back(Vector3(left, -y, z));
			normals.push_back(normal_left);
			ADD_TANGENT(0.0, 0.0, 1.0, 1.0);
			uvs.push_back(Vector2(u, 0.5 + v));
			point++;

			if (i > 0 && j > 0) {
				int i2 = i * 2;

				// right
				indices.push_back(prevrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2 - 2);
				indices.push_back(prevrow + i2);
				indices.push_back(thisrow + i2);
				indices.push_back(thisrow + i2 - 2);

				// left
				indices.push_back(prevrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
				indices.push_back(prevrow + i2 + 1);
				indices.push_back(thisrow + i2 + 1);
				indices.push_back(thisrow + i2 - 1);
			};

			z += size.z / (subdivide_d + 1.0);
		};

		y += size.y / (subdivide_h + 1.0);
		prevrow = thisrow;
		thisrow = point;
	};

	// bottom
	z = start_pos.z;
	thisrow = point;
	prevrow = 0;
	for (j = 0; j <= (subdivide_d + 1); j++) {
		x = start_pos.x;
		for (i = 0; i <= (subdivide_w + 1); i++) {
			float u = i;
			float v = j;
			u /= (3.0 * (subdivide_w + 1.0));
			v /= (2.0 * (subdivide_d + 1.0));

			// bottom
			points.push_back(Vector3(x, start_pos.y, -z));
			normals.push_back(Vector3(0.0, -1.0, 0.0));
			ADD_TANGENT(1.0, 0.0, 0.0, 1.0);
			uvs.push_back(Vector2(twothirds + u, 0.5 + v));
			point++;

			if (i > 0 && j > 0) {
				// bottom
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};

			x += size.x / (subdivide_w + 1.0);
		};

		z += size.z / (subdivide_d + 1.0);
		prevrow = thisrow;
		thisrow = point;
	};

	p_arr[RS::ARRAY_VERTEX] = points;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
	p_arr[RS::ARRAY_INDEX] = indices;
}

void MeshUtils::create_quad(Array &p_arr, Size2 size, Vector3 center_offset) {
	PoolVector<Vector3> faces;
	PoolVector<Vector3> normals;
	PoolVector<float> tangents;
	PoolVector<Vector2> uvs;

	faces.resize(6);
	normals.resize(6);
	tangents.resize(6 * 4);
	uvs.resize(6);

	Vector2 _size = Vector2(size.x / 2.0f, size.y / 2.0f);

	Vector3 quad_faces[4] = {
		Vector3(-_size.x, -_size.y, 0) + center_offset,
		Vector3(-_size.x, _size.y, 0) + center_offset,
		Vector3(_size.x, _size.y, 0) + center_offset,
		Vector3(_size.x, -_size.y, 0) + center_offset,
	};

	static const int indices[6] = {
		0, 1, 2,
		0, 2, 3
	};

	for (int i = 0; i < 6; i++) {
		int j = indices[i];
		faces.set(i, quad_faces[j]);
		normals.set(i, Vector3(0, 0, 1));
		tangents.set(i * 4 + 0, 1.0);
		tangents.set(i * 4 + 1, 0.0);
		tangents.set(i * 4 + 2, 0.0);
		tangents.set(i * 4 + 3, 1.0);

		static const Vector2 quad_uv[4] = {
			Vector2(0, 1),
			Vector2(0, 0),
			Vector2(1, 0),
			Vector2(1, 1),
		};

		uvs.set(i, quad_uv[j]);
	}

	p_arr[RS::ARRAY_VERTEX] = faces;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
}

void SphereMesh::create_mesh_array(Array &p_arr, float radius, float height, int radial_segments, int rings, bool is_hemisphere) {
	//radial_segments = p_radial_segments > 4 ? p_radial_segments : 4;
	//rings = p_rings > 1 ? p_rings : 1;

	int i, j, prevrow, thisrow, point;
	float x, y, z;

	float scale = height * (is_hemisphere ? 1.0 : 0.5);

	// set our bounding box

	PoolVector<Vector3> points;
	PoolVector<Vector3> normals;
	PoolVector<float> tangents;
	PoolVector<Vector2> uvs;
	PoolVector<int> indices;
	point = 0;

#define ADD_TANGENT(m_x, m_y, m_z, m_d) \
	tangents.push_back(m_x);            \
	tangents.push_back(m_y);            \
	tangents.push_back(m_z);            \
	tangents.push_back(m_d);

	thisrow = 0;
	prevrow = 0;
	for (j = 0; j <= (rings + 1); j++) {
		float v = j;
		float w;

		v /= (rings + 1);
		w = sin(Math_PI * v);
		y = scale * cos(Math_PI * v);

		for (i = 0; i <= radial_segments; i++) {
			float u = i;
			u /= radial_segments;

			x = sin(u * (Math_PI * 2.0));
			z = cos(u * (Math_PI * 2.0));

			if (is_hemisphere && y < 0.0) {
				points.push_back(Vector3(x * radius * w, 0.0, z * radius * w));
				normals.push_back(Vector3(0.0, -1.0, 0.0));
			} else {
				Vector3 p = Vector3(x * radius * w, y, z * radius * w);
				points.push_back(p);
				Vector3 normal = Vector3(x * w * scale, radius * (y / scale), z * w * scale);
				normals.push_back(normal.normalized());
			};
			ADD_TANGENT(z, 0.0, -x, 1.0)
			uvs.push_back(Vector2(u, v));
			point++;

			if (i > 0 && j > 0) {
				indices.push_back(prevrow + i - 1);
				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i - 1);

				indices.push_back(prevrow + i);
				indices.push_back(thisrow + i);
				indices.push_back(thisrow + i - 1);
			};
		};

		prevrow = thisrow;
		thisrow = point;
	};

	p_arr[RS::ARRAY_VERTEX] = points;
	p_arr[RS::ARRAY_NORMAL] = normals;
	p_arr[RS::ARRAY_TANGENT] = tangents;
	p_arr[RS::ARRAY_TEX_UV] = uvs;
	p_arr[RS::ARRAY_INDEX] = indices;
}

void MeshUtils::create_point(Array &p_arr) {
	PoolVector<Vector3> faces;
	faces.resize(1);
	faces.set(0, Vector3(0.0, 0.0, 0.0));

	p_arr[RS::ARRAY_VERTEX] = faces;
}

*/