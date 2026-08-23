
#pragma once
#include <string>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"
#include"Shader.h"

class Mesh
{
public:
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures;
	// Store VAO in public so it can be used in the Draw function
	VAO VAO;

	// Initializes the mesh
    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures){
        
        Mesh::vertices = vertices;
        Mesh::indices = indices;
        Mesh::textures = textures;

        VAO.Bind();
        // Generates Vertex Buffer Object and links it to vertices
        VBO VBO(vertices);
        // Generates Element Buffer Object and links it to indices
        EBO EBO(indices);
        // Links VBO attributes such as coordinates and colors to VAO
        VAO.LinkAttribute(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
        VAO.LinkAttribute(VBO, 1, 2, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
        VAO.LinkAttribute(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(5 * sizeof(float)));
        VAO.LinkAttribute(VBO, 3, 3, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float)));
        // Unbind all to prevent accidentally modifying them
        VAO.Unbind();
        VBO.Unbind();
        EBO.Unbind();
        }

        // Draws the mesh
        void Draw(Shader& shader, Camera& camera){
        // Bind shader to be able to access uniforms
        shader.use();
        VAO.Bind();

        // Keep track of how many of each type of textures we have
        unsigned int numDiffuse = 0;
        unsigned int numSpecular = 0;

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            std::string num;
            std::string type = textures[i].type;
            if (type == "diffuse")
            {
                num = std::to_string(numDiffuse++);
            }
            else if (type == "specular")
            {
                num = std::to_string(numSpecular++);
            }
            shader.setInt(type + num, static_cast<int>(i));
            textures[i].bind(i);
        }
            // Take care of the camera Matrix
            shader.setVec3("u_CameraPositionWS", camera.Position);
            glm::mat4 view = camera.GetViewMatrix();
            shader.setMat4("view", glm::value_ptr(view));

            // Draw the actual mesh
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    }
};