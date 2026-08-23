#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath)
    {
        std::string vertexCode = loadFile(vertexPath);
        std::string fragmentCode = loadFile(fragmentPath);

        unsigned int vertex = compile(GL_VERTEX_SHADER, vertexCode.c_str(), vertexPath);
        unsigned int fragment = compile(GL_FRAGMENT_SHADER, fragmentCode.c_str(), fragmentPath);

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);

        int success;
        char infoLog[512];
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, 512, nullptr, infoLog);
            std::cout << "Shader link error:\n" << infoLog << std::endl;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() const { glUseProgram(ID); }

    void setMat4(const std::string &name, const float* mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat);
    }

    void setMat3(const std::string &name, const float* mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat);
    }

    void setVec3(const std::string &name, const glm::vec3& value) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
    }

    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }  

    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);     
    }

    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }

private:
    std::string loadFile(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cout << "Failed to open shader file: " << path << std::endl;
            return "";
        }

        std::string line;
        std::stringstream ss;

        while (std::getline(file, line)) {
            if (line.find("#include") != std::string::npos) {
                size_t start = line.find_first_of("\"") + 1;
                size_t end = line.find_last_of("\"");
                std::string includeFileName = line.substr(start, end - start);
                
                std::string directory = path.substr(0, path.find_last_of("/\\") + 1);
                std::string fullIncludePath = directory + includeFileName;

                ss << loadFile(fullIncludePath) << '\n';
            } else {
                ss << line << '\n';
            }
        }
        return ss.str();
    }

    unsigned int compile(GLenum type, const char* src, const char* path)
    {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cout << "Shader compile error (" << path << "):\n" << infoLog << std::endl;
        }
        return shader;
    }
};