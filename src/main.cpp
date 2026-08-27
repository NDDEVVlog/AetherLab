#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>

#include "renderer/Shader.h"
#include "renderer/Model.h"
#include "renderer/Skybox.h"
#include "renderer/FBO.h"
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
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

int framebufferWidth = SCR_WIDTH;
int framebufferHeight = SCR_HEIGHT;

struct OutlineSettings {
    bool enable = false;
    int outputMode = 0; 
    glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    float thickness = 1.5f;
    float normalThreshold = 0.5f;
    float depthThreshold = 0.01f;
};

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

glm::mat4 CalculateLightSpaceMatrix(const MainLight& light) {
    glm::vec3 lightPos = -glm::normalize(light.direction) * 20.0f;
    glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 1.0f, 50.0f);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return lightProjection * lightView;
}

void RenderNormalPass(const Shader& normalShader, const FBO& normalFBO, Camera& camera, const std::vector<std::unique_ptr<Entity>>& scene) {
    normalFBO.Bind();
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    normalShader.use();
    
    float aspectRatio = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    normalShader.setMat4("projection", glm::value_ptr(projection));
    normalShader.setMat4("view", glm::value_ptr(view));

    for (const auto& entity : scene) {
        if (!entity->IsActive()) continue;
        auto renderer = entity->GetComponent<MeshRenderer>();
        auto transform = entity->GetComponent<Transform>();
        
        if (renderer && renderer->model && transform) {
            glm::mat4 modelMatrix = transform->GetModelMatrix();
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
            
            normalShader.setMat4("model", glm::value_ptr(modelMatrix));
            normalShader.setMat3("normalMatrix", glm::value_ptr(normalMatrix));
            renderer->model->Draw(normalShader);
        }
    }
    normalFBO.Unbind();
}

void RenderShadowPass(const Shader& shadowShader, const FBO& shadowFBO, const std::vector<std::unique_ptr<Entity>>& scene, const glm::mat4& lightSpaceMatrix) {
    shadowFBO.Bind();
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    shadowShader.use();
    shadowShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));

    for (const auto& entity : scene) {
        if (!entity->IsActive()) continue;
        auto renderer = entity->GetComponent<MeshRenderer>();
        auto transform = entity->GetComponent<Transform>();
        
        if (renderer && renderer->model && transform) {
            shadowShader.setMat4("model", glm::value_ptr(transform->GetModelMatrix()));
            renderer->model->Draw(shadowShader);
        }
    }

    glCullFace(GL_BACK);
    shadowFBO.Unbind();
}

void RenderScene(Shader& shader, Camera& camera, LightManager& lightManager, const std::vector<std::unique_ptr<Entity>>& scene, const std::vector<AdditionalLight>& activeLights, const glm::mat4& lightSpaceMatrix, unsigned int shadowMapID) {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    shader.setVec3("u_CameraPositionWS", camera.Position);
    shader.setFloat("material.shininess", 32.0f); 
    shader.setVec3("material.baseAlbedo", glm::vec3(1.0f));
    shader.setVec3("material.baseSpecular", glm::vec3(0.5f));
    shader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, shadowMapID);
    shader.setInt("shadowMap", 10);

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

void RenderScreenPass(Shader& screenShader, unsigned int quadVAO, unsigned int colorTexture) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void RenderOutlinePass(Shader& outlineShader, unsigned int quadVAO, const FBO& myFBO, const FBO& normalFBO, const OutlineSettings& settings) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    outlineShader.use();
    outlineShader.setInt("mainTexture", 0);
    outlineShader.setInt("normalTexture", 1);
    outlineShader.setInt("depthTexture", 2);
    
    outlineShader.setInt("outputMode", settings.outputMode);
    outlineShader.setVec2("texelSize", glm::vec2(1.0f / static_cast<float>(framebufferWidth), 1.0f / static_cast<float>(framebufferHeight)));
    
    outlineShader.setFloat("outlineThickness", settings.thickness);
    outlineShader.setFloat("normalThreshold", settings.normalThreshold);
    outlineShader.setFloat("depthThreshold", settings.depthThreshold);
    outlineShader.setVec4("outlineColor", settings.color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, myFBO.GetColorTexture());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalFBO.GetColorTexture());

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, myFBO.GetDepthTexture());

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void RenderUI(ImGuiManager& uiManager, LightManager& lightManager, const std::vector<std::unique_ptr<Entity>>& scene, bool isUIActive, OutlineSettings& outlineSettings) {
    uiManager.BeginFrame();
    
    if (isUIActive) {
        ImGui::Begin("Engine Control Panel");
        ImGui::Text("Press TAB to toggle camera/mouse mode");
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Post Processing", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable Outline Effect", &outlineSettings.enable);
            
            if (outlineSettings.enable) {
                const char* modes[] = { "Final (Color + Outline)", "Outline Only", "View Normal", "View Depth" };
                ImGui::Combo("Output Mode", &outlineSettings.outputMode, modes, IM_ARRAYSIZE(modes));
                
                if (outlineSettings.outputMode <= 1) {
                    ImGui::ColorEdit4("Outline Color", glm::value_ptr(outlineSettings.color));
                    ImGui::SliderFloat("Thickness", &outlineSettings.thickness, 0.0f, 5.0f);
                    ImGui::SliderFloat("Normal Threshold", &outlineSettings.normalThreshold, 0.0f, 2.0f);
                    ImGui::SliderFloat("Depth Threshold", &outlineSettings.depthThreshold, 0.0001f, 0.1f, "%.4f");
                }
            }
        }
        
        if (ImGui::CollapsingHeader("Global Lighting")) {
            ImGui::ColorEdit3("Ambient Color", glm::value_ptr(lightManager.ambientColor));
            ImGui::ColorEdit3("Main Light Color", glm::value_ptr(lightManager.mainLight.color));
            ImGui::SliderFloat3("Main Light Dir", glm::value_ptr(lightManager.mainLight.direction), -1.0f, 1.0f);
            ImGui::SliderFloat("Main Light Intensity", &lightManager.mainLight.intensity, 0.0f, 5.0f);
            ImGui::Checkbox("Cast Shadows", &lightManager.mainLight.castShadows);
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
    Shader skyBoxShader("shaders/skybox.vert","shaders/skybox.frag");
    Shader shadowShader("shaders/shadow.vert", "shaders/shadow.frag");
    Shader screenShader("shaders/screen.vert", "shaders/screen.frag");
    Shader normalShader("shaders/normal_pass.vert","shaders/normal_pass.frag");
    Shader outlineShader("shaders/screen.vert", "shaders/outline.frag");

    std::string skyBoxPath = "assets/SkyBox/";
    Skybox skyBox(std::vector<std::string>{
        skyBoxPath + "right.jpg", skyBoxPath + "left.jpg",
        skyBoxPath + "top.jpg", skyBoxPath + "bottom.jpg",
        skyBoxPath + "front.jpg", skyBoxPath + "back.jpg"
    });


    Model myModel("assets/Corset.fbx");
    Texture myTexture;
    myTexture.load("assets/Models/Kenku/ColorNeat.png", "texture_diffuse", false);
    myModel.AddTexture(myTexture);

 
    Model planeModel("assets/Models/Plane.fbx");
    Texture planeColor;
    planeColor.load("assets/textures/Tile138/Tiles138_1K-JPG_Color.jpg", "texture_diffuse", false);
    planeModel.AddTexture(planeColor);
    Texture planeNormal;
    planeNormal.load("assets/textures/Tile138/Tiles138_1K-JPG_NormalGL.jpg", "texture_normal", false);
    planeModel.AddTexture(planeNormal);


    Model Monster("assets/Models/Monster/Monster.dae"); 
    Texture monsterTexture;
    monsterTexture.load("assets/Models/Monster/Monster.jpg","texture_diffuse",false); 
    Monster.AddTexture(monsterTexture);

    Model duckModel("assets/Models/Duck/Duck.dae");
    Texture duckTexture;
    duckTexture.load("assets/Models/Duck/DuckCM.png","texture_diffuse",false);
    duckModel.AddTexture(duckTexture);

    FBO shadowFBO(SHADOW_WIDTH, SHADOW_HEIGHT);
    shadowFBO.AttachDepthTexture();
    shadowFBO.CheckStatus();
    
    FBO myFBO(SCR_WIDTH, SCR_HEIGHT);
    myFBO.AttachColorTexture();
    myFBO.AttachDepthTexture();
    myFBO.CheckStatus();

    FBO normalFBO(SCR_WIDTH, SCR_HEIGHT);
    normalFBO.AttachColorTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
    normalFBO.AttachDepthRenderBuffer();
    normalFBO.CheckStatus();


    //Use for post processing
    float quadVertices[] = { 
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    VAO screenVAO;
    screenVAO.Bind();
    VBO screenVBO(quadVertices, sizeof(quadVertices));

    screenVAO.LinkAttribute(screenVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    screenVAO.LinkAttribute(screenVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    screenVAO.Unbind();
    screenVBO.Unbind();

    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    InputManager inputManager(window);
    ImGuiManager uiManager(window);
    LightManager lightManager;
    OutlineSettings outlineSettings;
    
    lightManager.mainLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    lightManager.mainLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    lightManager.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);

    float deltaTime = 0.0f, lastFrame = 0.0f;
    bool isUIActive = false;

    SetupInput(inputManager, window, camera, isUIActive, deltaTime);

    std::vector<std::unique_ptr<Entity>> scene;

    // --- SETUP ENTITIES ---
    auto corsetEntity = std::make_unique<Entity>("Corset");
    corsetEntity->AddComponent<Transform>()->position = glm::vec3(0.0f, 0.0f, 0.0f);
    corsetEntity->AddComponent<MeshRenderer>(&myModel);
    scene.push_back(std::move(corsetEntity));

    auto planeEntity = std::make_unique<Entity>("Plane");
    auto planeTransform = planeEntity->AddComponent<Transform>();
    planeTransform->position = glm::vec3(0.0f, -1.0f, 0.0f);
    planeTransform->rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
    planeTransform->scale = glm::vec3(10.0f);
    planeEntity->AddComponent<MeshRenderer>(&planeModel);
    scene.push_back(std::move(planeEntity));

    auto monsterEntity = std::make_unique<Entity>("Monster");
    auto lanternTransform = monsterEntity->AddComponent<Transform>();
    lanternTransform->position = glm::vec3(-4.0f, -0.8f, 3.5f); 
    lanternTransform->rotation = glm::vec3(-90.0f,118.0f,0.0f);
    lanternTransform->scale = glm::vec3(0.1f); 
    monsterEntity->AddComponent<MeshRenderer>(&Monster);
    scene.push_back(std::move(monsterEntity));

  
    auto duckEntity = std::make_unique<Entity>("Duck");
    auto duckTransform = duckEntity->AddComponent<Transform>();
    duckTransform->position = glm::vec3(3.0f, -0.5f, 1.0f); 
    duckTransform->scale = glm::vec3(0.01f); 
    duckEntity->AddComponent<MeshRenderer>(&duckModel);
    scene.push_back(std::move(duckEntity));



    auto staticLightEntity = std::make_unique<Entity>("StaticLight");
    staticLightEntity->AddComponent<Transform>()->position = glm::vec3(0.0f, 2.0f, 2.0f);
    staticLightEntity->AddComponent<LightComponent>(LightType::Point, glm::vec3(1.0f, 0.5f, 0.2f));
    scene.push_back(std::move(staticLightEntity));


    auto rotatingLightEntity = std::make_unique<Entity>("RotatingLight");
    rotatingLightEntity->AddComponent<Transform>()->position = glm::vec3(4.0f, 1.0f, 0.0f); 
    rotatingLightEntity->AddComponent<RotateAround>(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 5.0f, 0.0f), 90.0f);
    rotatingLightEntity->AddComponent<LightComponent>(LightType::Point, glm::vec3(0.2f, 0.5f, 1.0f));
    scene.push_back(std::move(rotatingLightEntity));


    auto lanternLight = std::make_unique<Entity>("LanternLight");
    lanternLight->AddComponent<Transform>()->position = glm::vec3(-4.0f, 2.5f, -1.5f); // Đặt hơi cao lên cho giống bóng đèn
    lanternLight->AddComponent<LightComponent>(LightType::Point, glm::vec3(1.0f, 0.6f, 0.1f));
    scene.push_back(std::move(lanternLight));

 
    auto avocadoLight = std::make_unique<Entity>("AvocadoLight");
    avocadoLight->AddComponent<Transform>()->position = glm::vec3(3.5f, 1.0f, 1.5f);
    avocadoLight->AddComponent<LightComponent>(LightType::Point, glm::vec3(0.3f, 1.0f, 0.3f));
    scene.push_back(std::move(avocadoLight));


    auto extraOrbitLight = std::make_unique<Entity>("ExtraOrbitLight");
    extraOrbitLight->AddComponent<Transform>()->position = glm::vec3(0.0f, 1.5f, 0.0f);

    extraOrbitLight->AddComponent<RotateAround>(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 7.0f, 0.0f), -45.0f); 
    extraOrbitLight->AddComponent<LightComponent>(LightType::Point, glm::vec3(0.8f, 0.2f, 1.0f));
    scene.push_back(std::move(extraOrbitLight));


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

        glm::mat4 lightSpaceMatrix = CalculateLightSpaceMatrix(lightManager.mainLight);
        RenderShadowPass(shadowShader, shadowFBO, scene, lightSpaceMatrix);

        if (outlineSettings.enable) {
            RenderNormalPass(normalShader, normalFBO, camera, scene);
        }

        myFBO.Bind();
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        RenderScene(shader, camera, lightManager, scene, activeLights, lightSpaceMatrix, shadowFBO.GetDepthTexture());


        //SKyybox
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glm::mat4 skyboxProjection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
        skyBoxShader.use();
        skyBoxShader.setMat4("view", glm::value_ptr(skyboxView));
        skyBoxShader.setMat4("projection", glm::value_ptr(skyboxProjection));
        skyBox.Draw(skyBoxShader);
        
        myFBO.Unbind();

        if (outlineSettings.enable) {
            RenderOutlinePass(outlineShader, screenVAO.ID, myFBO, normalFBO, outlineSettings);
        } else {
            RenderScreenPass(screenShader, screenVAO.ID, myFBO.GetColorTexture());
        }

        RenderUI(uiManager, lightManager, scene, isUIActive, outlineSettings);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}