#ifndef TEXTURE_MATERIAL_H
#define TEXTURE_MATERIAL_H

#include "material.h"
#include "texture.h"

#include "../../libs/glm/vec4.hpp"
#include "../../libs/glm/gtc/type_ptr.hpp"

#include "camera.h"

class TextureMaterial : public Material {
public:
    int get_material_id() {
        return 3;
    }

    void bind_uniforms() {
        glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, glm::value_ptr(Camera::current_camera->projection_matrix));
        glUniformMatrix4fv(model_view_matrix_location, 1, GL_FALSE, glm::value_ptr(Camera::current_camera->model_view_matrix));

        if (texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->texture);
            glUniform1i(texture_location, 0);
        }
    }

    void setup_uniforms() {
        projection_matrix_location = get_uniform("u_proj_matrix");
        model_view_matrix_location = get_uniform("u_model_view_matrix");

        texture_location = get_uniform("u_texture");
    }

    void unbind() {
        glDisable(GL_TEXTURE_2D);
    }

    void setup_state() {
        glEnable(GL_TEXTURE_2D);
    }

    const GLchar **get_vertex_shader_source() {
        static const GLchar *vertex_shader_source[] = {
            "uniform mat4 u_proj_matrix;\n"
            "uniform mat4 u_model_view_matrix;\n"
            "\n"
            "attribute vec4 a_position;\n"
            "attribute vec2 a_uv;\n"
            "\n"
            "varying vec2 v_uv;\n"
            "\n"
            "void main() {\n"
            "  v_uv = a_uv;\n"
            "  gl_Position = u_proj_matrix * u_model_view_matrix * a_position;\n"
            "}"
        };

        return vertex_shader_source;
    }

    const GLchar **get_fragment_shader_source() {
        static const GLchar *fragment_shader_source[] = {
            "precision mediump float;\n"
            "\n"
            "uniform sampler2D u_texture;\n"
            "\n"
            "varying vec2 v_uv;\n"
            "\n"
            "void main() {\n"
            "  gl_FragColor = texture2D(u_texture, v_uv);\n"
            "}"
        };

        return fragment_shader_source;
    }

    TextureMaterial() : Material() {
        texture_location = 0;
        texture = NULL;
    }

    GLint projection_matrix_location;
    GLint model_view_matrix_location;

    GLint texture_location;

    Texture *texture;
};



#endif // COLORED_MATERIAL_H
