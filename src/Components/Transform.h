#pragma once
#include "../core/Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform : public Component {
public:
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    [[nodiscard]] glm::mat4 GetModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        

        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        
        model = glm::scale(model, scale);
        return model;
    }

    [[nodiscard]] glm::vec3 GetForward() const {
        glm::mat4 rot = glm::mat4(1.0f);
        
   
        rot = glm::rotate(rot, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rot = glm::rotate(rot, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rot = glm::rotate(rot, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        
        return glm::normalize(glm::vec3(rot * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    }
};