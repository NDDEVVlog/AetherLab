#pragma once
#include "../core/Component.h"
#include "../core/Entity.h"
#include "../Struct/Light.h"
#include "Transform.h"

class LightComponent : public Component {
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
};