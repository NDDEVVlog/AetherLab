#include "InputManager.hpp"

InputManager::InputManager(GLFWwindow* window) : m_window(window) {
    glfwSetWindowUserPointer(m_window, this);
    glfwSetCursorPosCallback(m_window, MouseCallbackThunk);
    glfwSetScrollCallback(m_window, ScrollCallbackThunk);
}

void InputManager::Update() {
    for (auto& [keyCode, data] : m_keyRegistry) {
        ProcessKeyState(keyCode, data);
    }
}

void InputManager::ProcessKeyState(int keyCode, KeyData& data) {
    int currentGlfwState = glfwGetKey(m_window, keyCode);

    if (currentGlfwState == GLFW_PRESS) {
        if (data.state == KeyState::None || data.state == KeyState::Released) {
            data.state = KeyState::Pressed;
            data.onPress.Invoke();
        } else {
            data.state = KeyState::Held;
            data.onHold.Invoke();
        }
    } 
    else if (currentGlfwState == GLFW_RELEASE) {
        if (data.state == KeyState::Pressed || data.state == KeyState::Held) {
            data.state = KeyState::Released;
            data.onRelease.Invoke();
        } else {
            data.state = KeyState::None;
        }
    }
}

InputManager::KeyEvent::ListenerID InputManager::SubscribePress(int keyCode, KeyEvent::Action action) {
    return m_keyRegistry[keyCode].onPress.Subscribe(std::move(action));
}

InputManager::KeyEvent::ListenerID InputManager::SubscribeHold(int keyCode, KeyEvent::Action action) {
    return m_keyRegistry[keyCode].onHold.Subscribe(std::move(action));
}

InputManager::KeyEvent::ListenerID InputManager::SubscribeRelease(int keyCode, KeyEvent::Action action) {
    return m_keyRegistry[keyCode].onRelease.Subscribe(std::move(action));
}

InputManager::MouseEvent::ListenerID InputManager::SubscribeMouseMove(MouseEvent::Action action) {
    return m_onMouseMove.Subscribe(std::move(action));
}

InputManager::ScrollEvent::ListenerID InputManager::SubscribeScroll(ScrollEvent::Action action) {
    return m_onScroll.Subscribe(std::move(action));
}

void InputManager::MouseCallbackThunk(GLFWwindow* window, double xpos, double ypos) {
    if (auto* instance = static_cast<InputManager*>(glfwGetWindowUserPointer(window))) {
        instance->HandleMouseMove(xpos, ypos);
    }
}

void InputManager::ScrollCallbackThunk(GLFWwindow* window, double xoffset, double yoffset) {
    (void)xoffset;
    if (auto* instance = static_cast<InputManager*>(glfwGetWindowUserPointer(window))) {
        instance->HandleScroll(yoffset);
    }
}

void InputManager::HandleMouseMove(double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    float xoffset = xpos - m_lastX;
    float yoffset = m_lastY - ypos; 
    
    m_lastX = xpos;
    m_lastY = ypos;

    m_onMouseMove.Invoke(xoffset, yoffset);
}

void InputManager::HandleScroll(double yoffset) {
    m_onScroll.Invoke(static_cast<float>(yoffset));
}