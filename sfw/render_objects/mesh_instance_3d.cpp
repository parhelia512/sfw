//--STRIP
#include "render_objects/mesh_instance_3d.h"

#include "render_objects/camera_3d.h"
//--STRIP

void MeshInstance3D::render() {
	if (!mesh) {
		return;
	}

	Transform mat_orig = Camera3D::current_camera->get_model_view_matrix();

	Camera3D::current_camera->set_model_view_matrix(mat_orig * transform);

	if (material) {
		material->bind();
	}

	mesh->render();

	for (int i = 0; i < children.size(); ++i) {
		MeshInstance3D *c = children[i];

		if (c) {
			c->render();
		}
	}

	Camera3D::current_camera->set_model_view_matrix(mat_orig);
}

MeshInstance3D::MeshInstance3D() {
	material = NULL;
	mesh = NULL;
}
MeshInstance3D::~MeshInstance3D() {
	children.clear();
}
