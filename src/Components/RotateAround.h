#pragma once
#include "../core/Component.h"
#include "../core/Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Transform.h"

class RotateAround : public Component {
public:
    glm::vec3 pivot;
    glm::vec3 axis;
    float speed; 

    explicit RotateAround(glm::vec3 targetPivot, glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f), float degreesPerSecond = 45.0f)
        : pivot(targetPivot), axis(glm::normalize(rotationAxis)), speed(degreesPerSecond) {}

    void Update(float deltaTime) override {

        Transform* transform = owner->GetComponent<Transform>();
        glm::vec3 offset = transform->position - pivot;
        glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(speed * deltaTime), axis);
        
        offset = glm::vec3(rotMatrix * glm::vec4(offset, 1.0f));
        transform->position = pivot + offset;
        transform->rotation += axis * (speed * deltaTime);
    }
};