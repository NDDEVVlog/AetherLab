#pragma once

#include<glad/glad.h>
#include"VBO.h"

class VAO
{
public:
	// ID reference for the Vertex Array Object
	unsigned int ID;
	// Constructor that generates a VAO ID
	VAO()
    {
        glGenVertexArrays(1, &ID);
    }


	void LinkAttribute(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
    {
        
        VBO.Bind();
        glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
        glEnableVertexAttribArray(layout);
        VBO.Unbind();

    }

	void Bind() const
    {
        glBindVertexArray(ID);
    }

	void Unbind() const
    {
        glBindVertexArray(0);
    }
    void Delete() const
    {
        glDeleteVertexArrays(1, &ID);
    }
};