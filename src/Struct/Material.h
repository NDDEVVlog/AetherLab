#pragma once
#include <glm/glm.hpp>
#include "../renderer/Texture.h" // We need Texture.h to call bind()

struct Material {
    // Pointers to the textures. If they are nullptr, the material has no map.
    Texture* albedoMap = nullptr;
    Texture* specularMap = nullptr;
    Texture* normalMap = nullptr;


    glm::vec3 baseAlbedo = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 baseSpecular = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 baseAmbient = glm::vec3(0.0f,0.0f,0.0f);
    float shininess = 32.0f;

    void ApplyToShader(const Shader& shader) const {

        if (albedoMap != nullptr) {
            albedoMap->bind(0); 
            shader.setInt("material.albedoMap", 0);
            shader.setBool("material.hasAlbedoMap", true);
        } else {
            shader.setBool("material.hasAlbedoMap", false);
        }


        if (specularMap != nullptr) {
            specularMap->bind(1); 
            shader.setInt("material.specularMap", 1);
            shader.setBool("material.hasSpecularMap", true);
        } else {
            shader.setBool("material.hasSpecularMap", false);
        }

        if (normalMap != nullptr) {
            normalMap->bind(2); 
            shader.setInt("material.normalMap", 2);
            shader.setBool("material.hasNormalMap", true);
        } else {
            shader.setBool("material.hasNormalMap", false);
        }
        shader.setVec3("material.baseAlbedo", baseAlbedo);
        shader.setVec3("material.baseSpecular", baseSpecular);
        shader.setFloat("material.shininess", shininess);
    }
};