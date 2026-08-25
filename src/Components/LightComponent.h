#pragma once
#include "../core/Component.h"
#include "../core/EditorComponent.h"
#include "../core/Entity.h"
#include "../Struct/Light.h"
#include <glm/gtc/type_ptr.hpp>
#include "../imgui/imgui.h"
#include "Transform.h"

class LightComponent : public EditorComponent {
public:
    LightType type;
    glm::vec3 color;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;

    explicit LightComponent(LightType lightType = LightType::Point, glm::vec3 lightColor = glm::vec3(1.0f))
        : type(lightType), color(lightColor), constant(1.0f), linear(0.09f), quadratic(0.032f),
          cutOff(glm::cos(glm::radians(12.5f))), outerCutOff(glm::cos(glm::radians(15.0f))) {}

    [[nodiscard]] AdditionalLight GenerateLightData() const {
        AdditionalLight data;
        data.type = type;
        data.color = color;
        data.constant = constant;
        data.linear = linear;
        data.quadratic = quadratic;
        data.cutOff = cutOff;
        data.outerCutOff = outerCutOff;

        if (Transform* transform = owner->GetComponent<Transform>()) {
            data.position = transform->position;
            data.direction = transform->GetForward();
        }

        return data;
    }

    void UpdateEditor() override {
        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            int lightType = static_cast<int>(type);
            const char* lightTypes[] = {"Point", "Spot"};
            if (ImGui::Combo("Type", &lightType, lightTypes, IM_ARRAYSIZE(lightTypes))) {
                type = static_cast<LightType>(lightType);
            }

            ImGui::ColorEdit3("Color", glm::value_ptr(color));
            ImGui::DragFloat("Constant", &constant, 0.01f, 0.0f);
            ImGui::DragFloat("Linear", &linear, 0.01f, 0.0f);
            ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.0f);

            if (type == LightType::Spot) {
                float innerAngle = glm::degrees(glm::acos(glm::clamp(cutOff, -1.0f, 1.0f)));
                float outerAngle = glm::degrees(glm::acos(glm::clamp(outerCutOff, -1.0f, 1.0f)));

                ImGui::SliderFloat("Inner Angle", &innerAngle, 0.0f, 90.0f, "%.1f deg");
                ImGui::SliderFloat("Outer Angle", &outerAngle, 0.0f, 90.0f, "%.1f deg");
                outerAngle = glm::max(outerAngle, innerAngle);

                cutOff = glm::cos(glm::radians(innerAngle));
                outerCutOff = glm::cos(glm::radians(outerAngle));
            }
        }
    }
};