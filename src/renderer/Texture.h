#pragma once
#include <glad/glad.h>
#include <stb/stb_image.h>
#include <string>
#include <iostream>

class Texture {
public:
    unsigned int ID = 0;
    std::string type;
    std::string path;

    bool load(const std::string& filePath, const std::string& typeName, bool flipVertically = true) {
        path = filePath;
        type = typeName;
        stbi_set_flip_vertically_on_load(flipVertically);

        int width, height, channels;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
        
        if (!data) {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
            return false;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;

        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return true;
    }

    void bind(unsigned int unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit); //Store at GL_Texture0 1 2 3 ...
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    void Delete() const {
        glDeleteTextures(1, &ID);
    }
};