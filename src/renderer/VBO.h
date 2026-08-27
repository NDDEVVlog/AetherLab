#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texUV;
    glm::vec3 normal;
    glm::vec3 color;
};

class VBO{
public:
    unsigned int ID;

    VBO(const float* vertices, GLsizeiptr size)
    {
        glGenBuffers(1, &ID);
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    explicit VBO(const std::vector<Vertex>& vertices)
    {
        glGenBuffers(1, &ID);
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    }

    void Bind(){
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    }
    void Unbind(){
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    void Delete(){
        glDeleteBuffers(1, &ID);
    }
};