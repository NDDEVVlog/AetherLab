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
#include "core/EditorComponent.h"
#include "components/Transform.h"
#include "components/MeshRenderer.h"
#include "components/RotateAround.h"
#include "components/LightComponent.h"
#include "components/CameraFollow.h"

#include "core/ImGuiManager.h"

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

GLFWwindow* InitializeWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "URP Engine", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return nullptr;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);  
    
    return window;
}

void SetupInput(InputManager& inputManager, GLFWwindow* window, Camera& camera, bool& isUIActive, float& deltaTime) {
    inputManager.SubscribePress(GLFW_KEY_ESCAPE, [window]() { 
        glfwSetWindowShouldClose(window, true); 
    });
    
    inputManager.SubscribePress(GLFW_KEY_TAB, [window, &isUIActive]() {
        isUIActive = !isUIActive;
        glfwSetInputMode(window, GLFW_CURSOR, isUIActive ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    });

    inputManager.SubscribeHold(GLFW_KEY_W, [&camera, &isUIActive, &deltaTime]() { if(!isUIActive) camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_S, [&camera, &isUIActive, &deltaTime]() { if(!isUIActive) camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_A, [&camera, &isUIActive, &deltaTime]() { if(!isUIActive) camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_D, [&camera, &isUIActive, &deltaTime]() { if(!isUIActive) camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_Q, [&camera, &isUIActive, &deltaTime]() { if(!isUIActive) camera.ProcessKeyboard(CameraMovement::UP, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_E, [&camera, &isUIActive, &deltaTime]() { if(!isUIActive) camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime); });
    
    inputManager.SubscribeMouseMove([&camera, &isUIActive](float xoffset, float yoffset) { 
        if (!isUIActive) camera.ProcessMouseMovement(xoffset, yoffset); 
    });
}

void RenderScene(Shader& shader, Camera& camera, LightManager& lightManager, const std::vector<std::unique_ptr<Entity>>& scene, const std::vector<AdditionalLight>& activeLights) {
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
        if (!entity->IsActive()) continue;
        
        auto renderer = entity->GetComponent<MeshRenderer>();
        auto transform = entity->GetComponent<Transform>();
        
        if (renderer && renderer->model && transform) {
            glm::mat4 modelMatrix = transform->GetModelMatrix();
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
            
            shader.setMat4("model", glm::value_ptr(modelMatrix));
            shader.setMat3("normalMatrix", glm::value_ptr(normalMatrix));
            
            renderer->model->Draw(shader);
        }
    }
}

void RenderUI(ImGuiManager& uiManager, LightManager& lightManager, const std::vector<std::unique_ptr<Entity>>& scene, bool isUIActive) {
    uiManager.BeginFrame();
    
    if (isUIActive) {
        ImGui::Begin("Engine Control Panel");
        ImGui::Text("Press TAB to toggle camera/mouse mode");
        ImGui::Separator();
        
        if (ImGui::CollapsingHeader("Global Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Ambient Color", glm::value_ptr(lightManager.ambientColor));
            ImGui::ColorEdit3("Main Light Color", glm::value_ptr(lightManager.mainLight.color));
            ImGui::SliderFloat3("Main Light Dir", glm::value_ptr(lightManager.mainLight.direction), -1.0f, 1.0f);
        }
        
        if (ImGui::CollapsingHeader("Entities")) {
            for (const auto& entity : scene) {
                if (ImGui::TreeNode(entity->name.c_str())) {
                    std::vector<EditorComponent*> editorComps = entity->GetComponents<EditorComponent>();
                    for (auto* editorGUI : editorComps) {
                        editorGUI->UpdateEditor();
                    }

                    ImGui::TreePop();
                }
            }
        }
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    uiManager.EndFrame();
}

int main() {
    GLFWwindow* window = InitializeWindow();
    if (!window) return -1;

    Shader shader("shaders/cube.vert", "shaders/cube.frag");
    Model myModel("assets/Models/Kenku/Birdbrain.fbx");
    Texture myTexture;
    myTexture.load("assets/Models/Kenku/ColorNeat.png", "texture_diffuse", false);
    myModel.AddTexture(myTexture);

    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    InputManager inputManager(window);
    ImGuiManager uiManager(window);
    LightManager lightManager;
    
    lightManager.mainLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    lightManager.mainLight.color = glm::vec3(0.5f, 0.5f, 0.5f);
    lightManager.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);

    float deltaTime = 0.0f, lastFrame = 0.0f;
    bool isUIActive = false;

    SetupInput(inputManager, window, camera, isUIActive, deltaTime);

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
    rotatingLightEntity->AddComponent<RotateAround>(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 5.0f, 0.0f), 90.0f);
    rotatingLightEntity->AddComponent<LightComponent>(LightType::Point, glm::vec3(0.2f, 0.5f, 1.0f));
    scene.push_back(std::move(rotatingLightEntity));

    auto flashLightEntity = std::make_unique<Entity>("FlashLight");
    flashLightEntity->AddComponent<Transform>();
    flashLightEntity->AddComponent<CameraFollow>(&camera);
    flashLightEntity->AddComponent<LightComponent>(LightType::Spot, glm::vec3(1.0f, 1.0f, 1.0f));
    scene.push_back(std::move(flashLightEntity));

    std::vector<AdditionalLight> activeLights;
    activeLights.reserve(32);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager.Update();
        activeLights.clear();

        for (const auto& entity : scene) {
            entity->Update(deltaTime);
            if (auto lightComp = entity->GetComponent<LightComponent>()) {
                activeLights.push_back(lightComp->GenerateLightData());
            }
        }

        RenderScene(shader, camera, lightManager, scene, activeLights);
        RenderUI(uiManager, lightManager, scene, isUIActive);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}