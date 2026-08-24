#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>

#include "renderer/Shader.h"
#include "renderer/Model.h"
#include "core/InputManager.hpp"
#include "renderer/Camera.h"
#include "Struct/Material.h"
#include "Struct/Light.h"

#include "core/Entity.h"
#include "components/MeshRenderer.h"
#include "components/RotateAround.h"
#include "components/LightComponent.h"
#include "components/CameraFollow.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
int framebufferWidth = SCR_WIDTH;
int framebufferHeight = SCR_HEIGHT;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    framebufferWidth = width;
    framebufferHeight = height;
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "URP Engine", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader shader("shaders/cube.vert", "shaders/cube.frag");
    Model myModel("assets/Corset.fbx");
    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    InputManager inputManager(window);
    LightManager lightManager;
    
    lightManager.mainLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    lightManager.mainLight.color = glm::vec3(0.5f, 0.5f, 0.5f);
    lightManager.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);

    float deltaTime = 0.0f, lastFrame = 0.0f;

    inputManager.SubscribePress(GLFW_KEY_ESCAPE, [&]() { glfwSetWindowShouldClose(window, true); });
    inputManager.SubscribeHold(GLFW_KEY_W, [&]() { camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_S, [&]() { camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_A, [&]() { camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_D, [&]() { camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_Q, [&]() { camera.ProcessKeyboard(CameraMovement::UP, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_E, [&]() { camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime); });
    inputManager.SubscribeMouseMove([&](float xoffset, float yoffset) { camera.ProcessMouseMovement(xoffset, yoffset); });

    std::vector<std::unique_ptr<Entity>> scene;

    auto corsetEntity = std::make_unique<Entity>("Corset");
    corsetEntity->AddComponent<Transform>()->position = glm::vec3(0.0f, 0.0f, 0.0f);
    corsetEntity->AddComponent<MeshRenderer>(&myModel);
    scene.push_back(std::move(corsetEntity));

    auto staticLightEntity = std::make_unique<Entity>("StaticLight");
    staticLightEntity->AddComponent<Transform>()->position = glm::vec3(0.0f, 2.0f, 2.0f);
    staticLightEntity->AddComponent<LightComponent>(LightType::Point, glm::vec3(1.0f, 0.5f, 0.2f));
    scene.push_back(std::move(staticLightEntity));

    auto rotatingLightEntity = std::make_unique<Entity>("RotatingLight");
    rotatingLightEntity->AddComponent<Transform>()->position = glm::vec3(4.0f, 1.0f, 0.0f); 
    rotatingLightEntity->AddComponent<RotateAround>(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);
    rotatingLightEntity->AddComponent<LightComponent>(LightType::Point, glm::vec3(0.2f, 0.5f, 1.0f));
    scene.push_back(std::move(rotatingLightEntity));

    auto flashLightEntity = std::make_unique<Entity>("FlashLight");
    flashLightEntity->AddComponent<Transform>();
    flashLightEntity->AddComponent<CameraFollow>(&camera);
    flashLightEntity->AddComponent<LightComponent>(LightType::Spot, glm::vec3(1.0f, 1.0f, 1.0f));
    scene.push_back(std::move(flashLightEntity));

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager.Update();

        std::vector<AdditionalLight> activeLights;
        for (auto& entity : scene) {
            entity->Update(deltaTime);
            if (auto lightComp = entity->GetComponent<LightComponent>()) {
                activeLights.push_back(lightComp->GenerateLightData());
            }
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("u_CameraPositionWS", camera.Position);
        shader.setFloat("material.shininess", 32.0f); 
        shader.setVec3("material.baseAlbedo", glm::vec3(1.0f));
        shader.setVec3("material.baseSpecular", glm::vec3(0.5f));

        lightManager.ApplyToShader(shader, activeLights);

        float aspectRatio = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
            
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("view", glm::value_ptr(view));
        
        for (const auto& entity : scene) {
            if (auto renderer = entity->GetComponent<MeshRenderer>()) {
                if (renderer->model) {
                    glm::mat4 modelMatrix = entity->GetComponent<Transform>()->GetModelMatrix();
                    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
                    
                    shader.setMat4("model", glm::value_ptr(modelMatrix));
                    shader.setMat3("normalMatrix", glm::value_ptr(normalMatrix));
                    
                    renderer->model->Draw(shader);
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}