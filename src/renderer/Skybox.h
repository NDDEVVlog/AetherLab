#pragma once
#include <iostream>
#include <vector>
#include <stb/stb_image.h>
#include <string>
#include <memory>
#include <glad/glad.h>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Shader.h"

class Skybox
{
private:
    unsigned int cubemapTexture;
    std::vector<Vertex> skyboxVertices{
        //   Coordinates
        {{-1.0f, -1.0f,  1.0f}},//        7--------6
        {{ 1.0f, -1.0f,  1.0f}},//       /|       /|
        {{ 1.0f, -1.0f, -1.0f}},//      4--------5 |
        {{-1.0f, -1.0f, -1.0f}},//      | |      | |
        {{-1.0f,  1.0f,  1.0f}},//      | 3------|-2
        {{ 1.0f,  1.0f,  1.0f}},//      |/       |/
        {{ 1.0f,  1.0f, -1.0f}},//      0--------1
        {{-1.0f,  1.0f, -1.0f}}
    };

    std::vector<GLuint> skyboxIndices{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
    };
    VAO skyboxVAO;
    std::unique_ptr<VBO> skyboxVBO;
    std::unique_ptr<EBO> skyboxEBO;
public:
    explicit Skybox(const std::vector<std::string>& facesPath){
        cubemapTexture = loadCubemap(facesPath);
        skyboxVAO.Bind();
        skyboxVBO = std::make_unique<VBO>(skyboxVertices);
        skyboxEBO = std::make_unique<EBO>(skyboxIndices);
        skyboxVAO.LinkAttribute(*skyboxVBO, 0, 3, GL_FLOAT, sizeof(Vertex), nullptr);
        skyboxVAO.Unbind();
        skyboxVBO->Unbind();
    }

    ~Skybox() {
        skyboxEBO->Delete();
        skyboxVBO->Delete();
        skyboxVAO.Delete();
        glDeleteTextures(1, &cubemapTexture);
    }

    void Draw(const Shader& shader) const {
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        shader.setInt("skybox", 0);
        skyboxVAO.Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(skyboxIndices.size()), GL_UNSIGNED_INT, nullptr);
        skyboxVAO.Unbind();
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
    }

    unsigned int loadCubemap(std::vector<std::string> faces)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                            0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }  
};
