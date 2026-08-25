#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include "renderer/Shader.h"

enum class LightType {
    Point = 0,
    Spot = 1
};

struct MainLight {
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float intensity{1.0f};
    bool castShadows{true};

    void ApplyToShader(const Shader& shader) const {
        shader.setVec3("mainLight.direction", direction);
        shader.setVec3("mainLight.color", color);
        shader.setFloat("mainLight.intensity", intensity);
        shader.setBool("mainLight.castShadows", castShadows);
    }
};

struct AdditionalLight {
    LightType type{LightType::Point};
    glm::vec3 position{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
    glm::vec3 color{1.0f};
    float constant{1.0f};
    float linear{0.09f};
    float quadratic{0.032f};
    float cutOff{glm::cos(glm::radians(12.5f))};
    float outerCutOff{glm::cos(glm::radians(15.0f))};

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
    glm::vec3 ambientColor{0.1f};

    void ApplyToShader(const Shader& shader, const std::vector<AdditionalLight>& additionalLights) const {
        mainLight.ApplyToShader(shader);
        shader.setVec3("u_AmbientColor", ambientColor);
        
        int lightCount = std::min(static_cast<int>(additionalLights.size()), 16);
        shader.setInt("u_AdditionalLightsCount", lightCount);
        
        for (int i = 0; i < lightCount; ++i) {
            additionalLights[i].ApplyToShader(shader, i);
        }
    }
};