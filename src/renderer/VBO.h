#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 textUV;
};

class VBO{
public:
    unsigned int ID;

    VBO(const GLfloat* vertices,const GLsizeiptr size)
    {
        glGenBuffers(1, &ID);
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
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