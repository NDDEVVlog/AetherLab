#pragma once

#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <stdexcept>

class FBO {
private:
    unsigned int fboID = 0;
    std::vector<unsigned int> colorTextures;
    unsigned int rbo = 0;
    unsigned int depthTexture = 0;
    int width = 0;
    int height = 0;

public:
    FBO(int screenWidth, int screenHeight) : width(screenWidth), height(screenHeight) {
        glGenFramebuffers(1, &fboID);
    }

    ~FBO() {
        Delete();
    }

    void Bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    }

    void Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void AttachColorTexture(GLint internalFormat = GL_RGB, GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE) {
        Bind();
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        int attachmentIndex = GL_COLOR_ATTACHMENT0 + static_cast<int>(colorTextures.size());
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentIndex, GL_TEXTURE_2D, textureID, 0);
        
        colorTextures.push_back(textureID);
        
        std::vector<unsigned int> attachments;
        for (size_t i = 0; i < colorTextures.size(); ++i) {
            attachments.push_back(GL_COLOR_ATTACHMENT0 + static_cast<unsigned int>(i));
        }
        glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
        
        Unbind();
    }

    void AttachDepthRenderBuffer() {
        Bind();
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        Unbind();
    }

    void AttachDepthTexture() {
        Bind();
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
        
        if (colorTextures.empty()) {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        Unbind();
    }

    bool CheckStatus() const {
        Bind();
        bool isComplete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        if (!isComplete) throw std::runtime_error("Framebuffer Incomplete!");
        Unbind();
        return isComplete;
    }

    void Delete() {
        if (fboID != 0) {
            glDeleteFramebuffers(1, &fboID);
            fboID = 0;
        }
        if (!colorTextures.empty()) {
            glDeleteTextures(static_cast<GLsizei>(colorTextures.size()), colorTextures.data());
            colorTextures.clear();
        }
        if (rbo != 0) {
            glDeleteRenderbuffers(1, &rbo);
            rbo = 0;
        }
        if (depthTexture != 0) {
            glDeleteTextures(1, &depthTexture);
            depthTexture = 0;
        }
    }

    [[nodiscard]] unsigned int GetColorTexture(size_t index = 0) const { return colorTextures.at(index); }
    [[nodiscard]] unsigned int GetDepthTexture() const { return depthTexture; }
};