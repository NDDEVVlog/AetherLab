#pragma once
#include "../core/Component.h"
#include "../core/EditorComponent.h"
#include "../core/Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../imgui/imgui.h"
#include "Transform.h"

class RotateAround : public EditorComponent {
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

    void UpdateEditor() override {
        if (ImGui::CollapsingHeader("Rotate Around", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Target / Pivot", glm::value_ptr(pivot), 0.1f);

            if (ImGui::DragFloat3("Axis", glm::value_ptr(axis), 0.05f, -1.0f, 1.0f)) {
                const float axisLength = glm::length(axis);
                if (axisLength > 0.0001f) {
                    axis = glm::normalize(axis);
                }
            }

            ImGui::SliderFloat("Speed", &speed, -360.0f, 360.0f, "%.1f deg/s");
        }
    }
};