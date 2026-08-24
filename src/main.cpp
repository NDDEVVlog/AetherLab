#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "renderer/Shader.h"
#include "renderer/Model.h"
#include "core/InputManager.hpp"
#include "renderer/Camera.h"
#include "Struct/Material.h"
#include "Struct/Light.h"

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
    
    // Đảm bảo đường dẫn tới file model của bạn là chính xác
    Model myModel("assets/Corset.fbx");

    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    InputManager inputManager(window);
    
    float deltaTime = 0.0f, lastFrame = 0.0f;

    inputManager.SubscribePress(GLFW_KEY_ESCAPE, [&]() { glfwSetWindowShouldClose(window, true); });
    inputManager.SubscribeHold(GLFW_KEY_W, [&]() { camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_S, [&]() { camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_A, [&]() { camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_D, [&]() { camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_Q, [&]() { camera.ProcessKeyboard(CameraMovement::UP, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_E, [&]() { camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime); });
    inputManager.SubscribeMouseMove([&](float xoffset, float yoffset) { camera.ProcessMouseMovement(xoffset, yoffset); });

    LightManager lightManager;
    lightManager.mainLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    lightManager.mainLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    lightManager.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
    
    AdditionalLight pointLight;
    pointLight.type = LightType::Point;
    pointLight.position = glm::vec3(0.0f, 2.0f, 2.0f);
    pointLight.color = glm::vec3(1.0f, 0.5f, 0.2f);
    pointLight.constant = 1.0f; pointLight.linear = 0.09f; pointLight.quadratic = 0.032f;
    lightManager.additionalLights.push_back(pointLight);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager.Update();

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        
        shader.setVec3("u_CameraPositionWS", camera.Position);
        shader.setFloat("material.shininess", 32.0f); 
        shader.setVec3("material.baseAlbedo", glm::vec3(1.0f));
        shader.setVec3("material.baseSpecular", glm::vec3(0.5f));

        lightManager.ApplyToShader(shader);

        float aspectRatio = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
            
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("view", glm::value_ptr(view));
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        
        shader.setMat4("model", glm::value_ptr(model));
        shader.setMat3("normalMatrix", glm::value_ptr(normalMatrix));

        myModel.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}