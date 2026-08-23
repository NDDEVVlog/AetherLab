#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "renderer/Shader.h"

enum class LightType {
    Point = 0,
    Spot = 1
};

struct MainLight {
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f);

    void ApplyToShader(const Shader& shader) const {
        shader.setVec3("mainLight.direction", direction);
        shader.setVec3("mainLight.color", color);
    }
};

struct AdditionalLight {
    LightType type;
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;

    void ApplyToShader(const Shader& shader, int index) const {
        std::string base = "u_AdditionalLights[" + std::to_string(index) + "]";
        shader.setInt(base + ".type", static_cast<int>(type));
        shader.setVec3(base + ".position", position);
        shader.setVec3(base + ".direction", direction);
        shader.setVec3(base + ".color", color);
        shader.setFloat(base + ".constant", constant);
        shader.setFloat(base + ".linear", linear);
        shader.setFloat(base + ".quadratic", quadratic);
        shader.setFloat(base + ".cutOff", cutOff);
        shader.setFloat(base + ".outerCutOff", outerCutOff);
    }
};

class LightManager {
public:
    MainLight mainLight;
    std::vector<AdditionalLight> additionalLights;
    glm::vec3 ambientColor = glm::vec3(0.1f);

    void ApplyToShader(const Shader& shader) const {
        mainLight.ApplyToShader(shader);
        shader.setVec3("u_AmbientColor", ambientColor);
        
        int lightCount = std::min(static_cast<int>(additionalLights.size()), 16);
        shader.setInt("u_AdditionalLightsCount", lightCount);
        
        for (int i = 0; i < lightCount; ++i) {
            additionalLights[i].ApplyToShader(shader, i);
        }
    }
};