#pragma once
#include <string>
#include <vector>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Texture.h"
#include "Shader.h"

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    VAO vao;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }

    void Draw(const Shader& shader) const {
        shader.setBool("material.hasAlbedoMap", false);
        shader.setBool("material.hasSpecularMap", false);
        shader.setBool("material.hasNormalMap", false);

        for (unsigned int i = 0; i < textures.size(); i++) {
            textures[i].bind(i);
            if (textures[i].type == "texture_diffuse") {
                shader.setInt("material.albedoMap", i);
                shader.setBool("material.hasAlbedoMap", true);
            } else if (textures[i].type == "texture_specular") {
                shader.setInt("material.specularMap", i);
                shader.setBool("material.hasSpecularMap", true);
            } else if (textures[i].type == "texture_normal") {
                shader.setInt("material.normalMap", i);
                shader.setBool("material.hasNormalMap", true);
            }
        }

        vao.Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        vao.Unbind();
    }

private:
    void setupMesh() {
        vao.Bind();
        VBO vbo(vertices);
        EBO ebo(indices);

        const GLsizei stride = sizeof(Vertex);
        vao.LinkAttribute(vbo, 0, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, position));
        vao.LinkAttribute(vbo, 1, 2, GL_FLOAT, stride, (void*)offsetof(Vertex, texUV));
        vao.LinkAttribute(vbo, 2, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, normal));
        vao.LinkAttribute(vbo, 3, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, color));

        vao.Unbind();
        vbo.Unbind();
        ebo.Unbind();
    }
};