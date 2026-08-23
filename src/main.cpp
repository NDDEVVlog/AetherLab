#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstddef>
#include <vector>

#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "renderer/VAO.h"
#include "renderer/VBO.h"
#include "core/InputManager.hpp"
#include "renderer/Camera.h"
#include "Struct/Material.h"
#include "Struct/Light.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
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

    Shader shader("shaders/cube.vert", "shaders/cube.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");
    
    Texture texture;
    if (!texture.load("assets/textures/ThaoVy2.jpg", "material.albedoMap")) {
        glfwTerminate();
        return -1;
    }

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f), glm::vec3(-1.7f,  3.0f, -7.5f)
    };

    VAO vao;
    vao.Bind();
    std::vector<Vertex> cubeVertices;
    cubeVertices.reserve(36);
    for (std::size_t i = 0; i < 36; ++i) {
        const float* vertex = &vertices[i * 8];
        cubeVertices.push_back({
            glm::vec3(vertex[0], vertex[1], vertex[2]),
            glm::vec2(vertex[3], vertex[4]),
            glm::vec3(vertex[5], vertex[6], vertex[7]),
            glm::vec3(1.0f)
        });
    }

    VBO vbo(cubeVertices);
    const GLsizei stride = sizeof(Vertex);
    vao.LinkAttribute(vbo, 0, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, position));
    vao.LinkAttribute(vbo, 1, 2, GL_FLOAT, stride, (void*)offsetof(Vertex, texUV));
    vao.LinkAttribute(vbo, 2, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, normal));
    vao.Unbind();
    vbo.Unbind();

    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    InputManager inputManager(window);
    
    float deltaTime = 0.0f, lastFrame = 0.0f;

    inputManager.SubscribePress(GLFW_KEY_ESCAPE, [&]() { glfwSetWindowShouldClose(window, true); });
    inputManager.SubscribeHold(GLFW_KEY_W, [&]() { camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_S, [&]() { camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_A, [&]() { camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime); });
    inputManager.SubscribeHold(GLFW_KEY_D, [&]() { camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime); });
    inputManager.SubscribeMouseMove([&](float xoffset, float yoffset) { camera.ProcessMouseMovement(xoffset, yoffset); });

    Material material;
    material.albedoMap = &texture;
    material.baseSpecular = glm::vec3(0.5f);
    material.shininess = 32.0f;


    //DirectionalLight
    LightManager lightManager;
    lightManager.mainLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    lightManager.mainLight.color = glm::vec3(0.4f, 0.4f, 0.4f);
    lightManager.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
    
    AdditionalLight pointLight;
    pointLight.type = LightType::Point;
    pointLight.position = glm::vec3(2.0f, 2.0f, 2.0f);
    pointLight.color = glm::vec3(1.0f, 0.5f, 0.2f);
    pointLight.constant = 1.0f; pointLight.linear = 0.09f; pointLight.quadratic = 0.032f;
    lightManager.additionalLights.push_back(pointLight);

    AdditionalLight spotLight;
    spotLight.type = LightType::Spot;
    spotLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLight.constant = 1.0f; spotLight.linear = 0.09f; spotLight.quadratic = 0.032f;
    spotLight.cutOff = glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(15.0f));
    lightManager.additionalLights.push_back(spotLight);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager.Update();

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("u_CameraPositionWS", camera.Position);
        
        material.ApplyToShader(shader);

        lightManager.additionalLights[1].position = camera.Position;
        lightManager.additionalLights[1].direction = camera.Front;
        lightManager.ApplyToShader(shader);

        float aspectRatio = framebufferHeight > 0
            ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
            : static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("view", glm::value_ptr(view));
        
        vao.Bind();
        for (unsigned int i = 0; i < 6; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            
            shader.setMat4("model", glm::value_ptr(model));
            shader.setMat3("normalMatrix", glm::value_ptr(normalMatrix));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    vao.Delete();
    vbo.Delete();
    texture.Delete();
    glfwTerminate();
    return 0;
}