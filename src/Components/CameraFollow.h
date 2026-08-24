#pragma once
#include "../core/Component.h"
#include "../core/Entity.h"
#include "../renderer/Camera.h"
#include "Transform.h"

class CameraFollow : public Component {
private:
    const Camera* m_camera;

public:
    explicit CameraFollow(const Camera* targetCamera) : m_camera(targetCamera) {}

    void Update(float deltaTime) override {
        if (!m_camera) return;

        if (Transform* transform = owner->GetComponent<Transform>()) {
            transform->position = m_camera->Position;
            
            glm::vec3 dir = m_camera->Front;
            float pitch = glm::degrees(asin(dir.y));
            float yaw = glm::degrees(atan2(-dir.x, -dir.z));
            transform->rotation = glm::vec3(pitch, yaw, 0.0f);
        }
    }
};